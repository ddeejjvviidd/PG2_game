//
// WARNING:
// In general, you can NOT freely reorder includes!
//

// C++
// include anywhere, in any order
#include <iostream>
#include <fstream>
#include <chrono>
#include <stack>
#include <random>
#include <string>

// OpenCV (does not depend on GL)
#include <opencv2/opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h>
// WGLEW = Windows GL Extension Wrangler (change for different platform)
// platform specific functions (in this case Windows)
#ifdef _WIN32
#include <GL/wglew.h>
#endif

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nlohmann/json.hpp> // Include JSON library

#include "app.hpp"
#include "gl_err_callback.h"
#include "assets.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"
#include "Model.hpp"

using json = nlohmann::json; // Alias for convenience

App::App()
{
	// default constructor
	// nothing to do here (so far...)
	std::cout << "Constructed...\n";
}

bool App::init()
{
	std::cout << "Starting init...\n";
	try
	{
		// initialization code
		//...
		// init glfw
		// https://www.glfw.org/documentation.html
		std::cout << "Initializing GLFW...\n";
		glfwInit();

		glfwSetErrorCallback(error_callback);

		// Load settings from app_settings.json
		std::ifstream settingsFile("app_settings.json");
		if (settingsFile.is_open())
		{
			try
			{
				json settings = json::parse(settingsFile);

				// Set appname (with fallback)
				if (settings.contains("appname") && settings["appname"].is_string())
				{
					appname = settings["appname"].get<std::string>();
				}

				// Set default_resolution (with fallback)
				if (settings.contains("default_resolution") && settings["default_resolution"].is_object())
				{
					if (settings["default_resolution"].contains("x") && settings["default_resolution"]["x"].is_number_integer())
					{
						resX = settings["default_resolution"]["x"].get<int>();
					}
					if (settings["default_resolution"].contains("y") && settings["default_resolution"]["y"].is_number_integer())
					{
						resY = settings["default_resolution"]["y"].get<int>();
					}
				}
			}
			catch (const json::exception &e)
			{
				std::cerr << "JSON parsing error: " << e.what() << '\n';
				// Continue with defaults
			}
			settingsFile.close();
		}
		else
		{
			std::cerr << "Could not open app_settings.json, using defaults\n";
		}

		// Explicitly request OpenGL 4.6 Compatibility Profile (default-like)
		std::cout << "Creating window...\n";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // Try CORE_PROFILE if needed

		// open window (GL canvas) with no special properties
		// https://www.glfw.org/docs/latest/quick.html#quick_create_window
		window = glfwCreateWindow(resX, resY, appname.c_str(), NULL, NULL);
		if (!window)
		{
			std::cerr << "Failed to create GLFW window\n";
			throw std::runtime_error("GLFW window creation failed");
		}
		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, this);
		glfwSetScrollCallback(window, [](GLFWwindow *w, double x, double y)
							  { static_cast<App *>(glfwGetWindowUserPointer(w))->scroll_callback(w, x, y); });
		glfwSetKeyCallback(window, [](GLFWwindow *w, int k, int s, int a, int m)
						   { static_cast<App *>(glfwGetWindowUserPointer(w))->key_callback(w, k, s, a, m); });
		// Add mouse button callback with lambda
		glfwSetMouseButtonCallback(window, [](GLFWwindow *w, int button, int action, int mods)
								   { static_cast<App *>(glfwGetWindowUserPointer(w))->mouse_button_callback(w, button, action, mods); });

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow *w, int width, int height)
									   { static_cast<App *>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(w, width, height); });
		glfwSetCursorPosCallback(window, [](GLFWwindow *w, double x, double y)
								 { static_cast<App *>(glfwGetWindowUserPointer(w))->cursor_position_callback(w, x, y); });

		// init glew
		// http://glew.sourceforge.net/basic.html
		std::cout << "Initializing GLEW...\n";
		glewInit();
#ifdef _WIN32
		wglewInit();
#endif
		// if (not_success)
		//  throw std::runtime_error("something went bad");

		// HOWTO get string safely ( glGetString() may return nullptr )
		// Get OpenGL version
		const char *openGLVersion = (const char *)glGetString(GL_VERSION);
		if (openGLVersion == nullptr)
			std::cout << "<Unknown>\n";
		else
			std::cout << "Value is:" << openGLVersion << '\n';

		// HOWTO get integer
		// Get profile info with debugging
		GLint myint;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &myint);

		if (myint & GL_CONTEXT_CORE_PROFILE_BIT)
		{
			std::cout << "We are using CORE profile\n";
		}
		else
		{
			if (myint & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
			{
				std::cout << "We are using COMPATIBILITY profile\n";
			}
			else
			{
				throw std::runtime_error("What??");
			}
		}

		if (!GLEW_ARB_direct_state_access)
			throw std::runtime_error("No DSA :-(");

		if (GLEW_ARB_debug_output)
		{
			glDebugMessageCallback(MessageCallback, 0);
			glEnable(GL_DEBUG_OUTPUT);

			// default is asynchronous debug output, use this to simulate glGetError() functionality
			// glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

			std::cout << "GL_DEBUG enabled." << std::endl;
		}
		else
			std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;

		// Set initial VSync state
		glfwSwapInterval(vsyncEnabled ? 1 : 0);

		std::cout << "Calling init_assets...\n";
		init_assets();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Init failed : " << e.what() << std::endl;
		throw;
	}
	std::cout << "Initialized...\n";

	return true;
}

void App::init_assets(void)
{

	// shader: load, compile, link, initialize params
	// (may be moved to global variables - if all models use same shader)
	ShaderProgram my_shader = ShaderProgram("resources/basic.vert", "resources/basic.frag");
	// ShaderProgram my_transparent_shader = ShaderProgram("resources/tex.vert", "resources/tex.frag");
	shader_prog_ID = my_shader.getID();

	// Define the 5x5 labyrinth layout
	const int gridSize = 10;
	int labyrinth[gridSize][gridSize] = {
		{1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
		{1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
		{1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
		{1, 0, 1, 1, 1, 1, 0, 1, 0, 1},
		{1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
		{1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
		{1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
		{1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 0, 1, 1}};

	// Place cubes for each '1' in the labyrinth
	float cubeSize = 1.0f; // Size of each cube (adjust as needed)
	for (int z = 0; z < gridSize; ++z)
	{
		for (int x = 0; x < gridSize; ++x)
		{
			if (labyrinth[z][x] == 1)
			{
				// Create a cube model at position (x, 0, z)
				models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
				models.back().origin = glm::vec3(
					x * cubeSize - 5.0f, // Center the labyrinth around (0, 0, 0)
					0.0f,				 // Y = 0 (ground level)
					z * cubeSize - 5.0f	 // Center the labyrinth
				);
			}
		}
	}

	// Flat floor for labyrinth (20x20 units to match 10x10 grid of 2-unit cubes)
	models.emplace_back(30.0f, 30.0f, my_shader, "resources/textures/StoneFloorTexture.png");
	models.back().origin = glm::vec3(0.0f, -0.55f, 0.0f); // Slightly below cubes

	// Heightmap terrain (separate, offset to the right)
	models.emplace_back("resources/textures/heights.png", my_shader, "resources/textures/StoneFloorTexture.png", 50, 50, 5.0f);
	models.back().origin = glm::vec3(0.0f, -0.55f, -20.0f); // Offset 30 units along

	camera = Camera(glm::vec3(0.0f, 0.0f, -7.0f)); // A`djusted to see larger labyrinth

	// // Create four triangles at different positions
	// models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/grass.png"); // Front (0, 0, 0)
	// models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/grass.png"); // Back (0, 0, 2)
	// models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/grass.png"); // Left (-2, 0, 0)
	// models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/grass.png"); // Right (2, 0, 0)
	// models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/box_rgb888.png");	  // Above (0, 2, 0)

	// // Set positions
	// models[0].origin = glm::vec3(0.0f, 0.0f, 0.0f);	 // Front
	// models[1].origin = glm::vec3(0.0f, 0.0f, 2.0f);	 // Back
	// models[2].origin = glm::vec3(-1.0f, 0.0f, 1.0f); // Left
	// models[3].origin = glm::vec3(1.0f, 0.0f, 1.0f);	 // Right
	// models[4].origin = glm::vec3(0.0f, 2.0f, 0.0f);	 // Cube above

	// Transparent test
	models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
	models.back().origin = glm::vec3(0.0f, 0.0f, 0.0f);
	models.back().transparent = true;

	models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
	models.back().origin = glm::vec3(0.0f, 0.0f, 2.0f);
	models.back().transparent = true;

	models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
	models.back().origin = glm::vec3(-1.0f, 0.0f, 1.0f);
	models.back().transparent = true;

	models.emplace_back("resources/objects/triangle.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
	models.back().origin = glm::vec3(1.0f, 0.0f, 1.0f);
	models.back().transparent = true;

	// Keep the cube non-transparent
	models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/minecraft_glass.png");
	models.back().origin = glm::vec3(2.0f, 2.0f, 2.0f);
	models.back().transparent = true;

	models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
	models.back().origin = glm::vec3(0.0f, 2.0f, 0.0f);
	models.back().transparent = true;

	// Initialize projection matrix
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	if (windowHeight <= 0)
		windowHeight = 1; // Avoid division by 0
	float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 20000.0f);
	glViewport(0, 0, windowWidth, windowHeight);

	// Initialize cursor position
	glfwGetCursorPos(window, &cursorLastX, &cursorLastY);
}

int App::run(void)
{
	if (!window)
		return -1;

	double lastTime = glfwGetTime(); // Time of last FPS update
	int frameCount = 0;				 // Number of frames since last update

	try
	{
		glEnable(GL_DEPTH_TEST);
		// glCullFace(GL_BACK);
		// glEnable(GL_CULL_FACE);

		// Disable cursor for FPS-style control
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Get uniform location in GPU program. This will not change, so it can be moved out of the game loop.
		GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
		if (uniform_color_location == -1)
		{
			std::cerr << "Uniform location is not found in active shader program. Did you forget to activate it?\n";
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!glfwWindowShouldClose(window))
		{
			// Measure time
			double currentTime = glfwGetTime();
			float totalTime = static_cast<float>(currentTime - startTime);
			float deltaTime = static_cast<float>(currentTime - lastTime); // Add deltaTime for camera

			frameCount++;

			// Update FPS every second
			if (currentTime - lastTime >= 1.0)
			{
				double fps = frameCount / (currentTime - lastTime);
				std::string title = "FPS: " + std::to_string(static_cast<int>(fps + 0.5)) +
									" | VSync: " + (vsyncEnabled ? "On" : "Off");
				glfwSetWindowTitle(window, title.c_str());

				frameCount = 0;
				lastTime = currentTime;
			}

			// Clear OpenGL canvas, both color buffer and Z-buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update camera position based on input
			glm::vec3 movement = camera.ProcessInput(window, deltaTime);
			camera.Position += movement;

			glUseProgram(shader_prog_ID);

			// set uniform parameter for shader
			// (try to change the color in key callback)
			if (uniform_color_location != -1) // Optional if shaders use textures only
				glUniform4f(uniform_color_location, r, g, b, a);

			// Set view and projection matrices using camera
			glm::mat4 viewMatrix = camera.GetViewMatrix();
			glUniformMatrix4fv(glGetUniformLocation(shader_prog_ID, "uV_m"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(glGetUniformLocation(shader_prog_ID, "uP_m"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			// Update time for all models
			for (auto &model : models)
			{
				model.update(totalTime);
			}

			std::vector<Model *> transparentModels;
			for (auto &model : models)
			{
				if (!model.transparent)
				{
					model.draw();
				}
				else
				{
					transparentModels.push_back(&model);
				}
			}

			std::sort(transparentModels.begin(), transparentModels.end(), [&](Model *a, Model *b)
					  {
				glm::vec3 posA = a->origin;
				glm::vec3 posB = b->origin;
				return glm::distance(camera.Position, posA) > glm::distance(camera.Position, posB); });

			glEnable(GL_BLEND);
			// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
			for (auto &model : transparentModels)
			{
				model->draw();
				// std::cout << "Drawing transparent model\n";
			}
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			// Poll for and process events
			glfwPollEvents();
			// Swap front and back buffers
			glfwSwapBuffers(window);
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << "App failed : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Finished OK...\n";
	return EXIT_SUCCESS;
}

void App::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	windowWidth = width;
	windowHeight = height > 0 ? height : 1; // Avoid division by 0
	float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 20000.0f);
	glViewport(0, 0, windowWidth, windowHeight);
}

void App::error_callback(int error, const char *description)
{
	std::cerr << "GLFW Error " << error << ": " << description << '\n';
}

void App::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	// Calculate mouse offset
	double xoffset = xpos - cursorLastX;
	double yoffset = ypos - cursorLastY; // No need to invert unless your coordinate system requires it
	cursorLastX = xpos;
	cursorLastY = ypos;

	camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	fov -= static_cast<float>(yoffset) * 5.0f;
	fov = glm::clamp(fov, 10.0f, 120.0f);
	float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 20000.0f);
	std::cout << "FOV: " << fov << "\n";
}

void App::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT))
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_F12:							// Changed from GLFW_KEY_V to GLFW_KEY_F12
			vsyncEnabled = !vsyncEnabled;			// Toggle VSync state
			glfwSwapInterval(vsyncEnabled ? 1 : 0); // Apply VSync setting
			// Title will update in the next FPS cycle
			break;
		case GLFW_KEY_R:
			if (r == 0.0f)
			{
				r = 1.0f;
			}
			else if (r == 1.0f)
			{
				r = 0.0f;
			}
			std::cout << "r = " << r << std::endl;
			break;
		case GLFW_KEY_G:
			if (g == 0.0f)
			{
				g = 1.0f;
			}
			else if (g == 1.0f)
			{
				g = 0.0f;
			}
			std::cout << "g = " << g << std::endl;
			break;
		case GLFW_KEY_B:
			if (b == 0.0f)
			{
				b = 1.0f;
			}
			else if (b == 1.0f)
			{
				b = 0.0f;
			}
			std::cout << "b = " << b << std::endl;
			break;
		default:
			break;
		}
	}
}

void App::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (r == 1.0f)
		{
			r = 0.0f;
			g = 1.0f;
			b = 0.0f;
		}
		else if (g == 1.0f)
		{
			r = 0.0f;
			g = 0.0f;
			b = 1.0f;
		}
		else if (b == 1.0f)
		{
			r = 1.0f;
			g = 0.0f;
			b = 0.0f;
		}
		std::cout << "Mouse left click, r = " << r << ", g = " << g << ", b = " << b << std::endl;
	}
}

App::~App()
{
	// clean-up
	if (window)
		glfwDestroyWindow(window);
	glfwTerminate();

	// cleanup GL data
	glDeleteProgram(shader_prog_ID);
	// glDeleteBuffers(1, &VBO_ID);
	// glDeleteVertexArrays(1, &VAO_ID);

	exit(EXIT_SUCCESS);
	cv::destroyAllWindows();
	std::cout << "Bye...\n";
}
