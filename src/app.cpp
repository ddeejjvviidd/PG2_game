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

				// Set antialiasing
				if (settings.contains("antialiasing") && settings["antialiasing"].is_object())
				{
					if (settings["antialiasing"].contains("enabled") && settings["antialiasing"]["enabled"].is_boolean())
					{
						antiAliasingEnabled = settings["antialiasing"]["enabled"].get<bool>();
					}
					if (settings["antialiasing"].contains("samples") && settings["antialiasing"]["samples"].is_number_integer())
					{
						antiAliasingSamples = settings["antialiasing"]["samples"].get<int>();
						glfwWindowHint(GLFW_SAMPLES, antiAliasingSamples);
					}
					std::cout << "Antialiasing enabled: " << (antiAliasingEnabled ? "true" : "false") << "\n";
					std::cout << "Antialiasing samples: " << antiAliasingSamples << "\n";
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

		windowedWidth = resX;
		windowedHeight = resY;

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

void App::UpdateLightUniforms(ShaderProgram &shader)
{
	GLuint program = shader.getID();

	// Directional light
	glUniform3fv(glGetUniformLocation(program, "dirLight.direction"), 1, &sun.direction[0]);
	glUniform3fv(glGetUniformLocation(program, "dirLight.ambient"), 1, &sun.ambient[0]);
	glUniform3fv(glGetUniformLocation(program, "dirLight.diffuse"), 1, &sun.diffuse[0]);
	glUniform3fv(glGetUniformLocation(program, "dirLight.specular"), 1, &sun.specular[0]);

	// Point lights
	for (int i = 0; i < 3; i++)
	{
		std::string prefix = "pointLights[" + std::to_string(i) + "].";
		glUniform3fv(glGetUniformLocation(program, (prefix + "position").c_str()), 1, &pointLights[i].position[0]);
		glUniform3fv(glGetUniformLocation(program, (prefix + "ambient").c_str()), 1, &pointLights[i].ambient[0]);
		glUniform3fv(glGetUniformLocation(program, (prefix + "diffuse").c_str()), 1, &pointLights[i].diffuse[0]);
		glUniform3fv(glGetUniformLocation(program, (prefix + "specular").c_str()), 1, &pointLights[i].specular[0]);
		glUniform1f(glGetUniformLocation(program, (prefix + "constant").c_str()), pointLights[i].constant);
		glUniform1f(glGetUniformLocation(program, (prefix + "linear").c_str()), pointLights[i].linear);
		glUniform1f(glGetUniformLocation(program, (prefix + "quadratic").c_str()), pointLights[i].quadratic);
	}

	// Spot light
	glUniform1i(glGetUniformLocation(program, "useSpotLight"), spotLightEnabled);
	glUniform3fv(glGetUniformLocation(program, "spotLight.position"), 1, &spotLight.position[0]);
	glUniform3fv(glGetUniformLocation(program, "spotLight.direction"), 1, &spotLight.direction[0]);
	glUniform1f(glGetUniformLocation(program, "spotLight.cutOff"), spotLight.cutOff);
	glUniform1f(glGetUniformLocation(program, "spotLight.outerCutOff"), spotLight.outerCutOff);
	glUniform3fv(glGetUniformLocation(program, "spotLight.ambient"), 1, &spotLight.ambient[0]);
	glUniform3fv(glGetUniformLocation(program, "spotLight.diffuse"), 1, &spotLight.diffuse[0]);
	glUniform3fv(glGetUniformLocation(program, "spotLight.specular"), 1, &spotLight.specular[0]);
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
				models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/box_rgb888.png");
				models.back().origin = glm::vec3(
					x * cubeSize - 5.0f, // Center the labyrinth around (0, 0, 0)
					0.0f,				 // Y = 0 (ground level)
					z * cubeSize - 5.0f	 // Center the labyrinth
				);
			}
		}
	}

	// Flat floor for labyrinth (20x20 units to match 10x10 grid of 2-unit cubes)
	floor.emplace_back(100.0f, 100.0f, my_shader, "resources/textures/StoneFloorTexture.png");
	floor.back().origin = glm::vec3(0.0f, -0.55f, 0.0f); // Slightly below cubes

	// Heightmap terrain (separate, offset to the right)
	floor.emplace_back("resources/textures/heights.png", my_shader, "resources/textures/StoneFloorTexture.png", 50, 50, 5.0f);
	floor.back().origin = glm::vec3(0.0f, -0.55f, -20.0f); // Offset 30 units along

	camera = Camera(glm::vec3(0.0f, 20.0f, -7.0f));

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

	// Grass
	models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/grass.png");
	models.back().origin = glm::vec3(1.0f, 1.0f, 0.0f);
	models.back().transparent = true;

	// Original sphere (will be animated)
	sphere1Index = models.size();
	models.emplace_back("resources/objects/sphere.obj", my_shader, "resources/textures/sphere_texture.png");
	models.back().origin = glm::vec3(1.0f, 7.0f, 0.0f);
	models.back().transparent = true;

	// Second sphere
	sphere2Index = models.size();
	models.emplace_back("resources/objects/sphere.obj", my_shader, "resources/textures/sphere_texture.png");
	models.back().origin = glm::vec3(-2.0f, 7.0f, 3.0f);
	models.back().transparent = true;

	// Third sphere
	sphere3Index = models.size();
	models.emplace_back("resources/objects/sphere.obj", my_shader, "resources/textures/sphere_texture.png");
	models.back().origin = glm::vec3(-2.0f, 10.0f, 0.0f);
	models.back().transparent = true;

	models.emplace_back("resources/objects/cube.obj", my_shader, "resources/textures/mirek_vyspely_512.png");
	models.back().origin = glm::vec3(0.0f, 2.0f, 0.0f);
	models.back().transparent = true;

	// Add the sun model
	models.emplace_back(32, my_shader, glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow color passed here

	sunModelIndex = models.size() - 1;
	models[sunModelIndex].isSun = true;
	models[sunModelIndex].transparent = false;

	// Initialize point lights
	pointLights[0].position = glm::vec3(0.0f, 2.0f, 0.0f);
	pointLights[0].diffuse = glm::vec3(1.0f, 0.0f, 0.0f); // Red light
	pointLights[0].linear = 0.09f;
	pointLights[0].quadratic = 0.032f;

	pointLights[1].position = glm::vec3(5.0f, 1.0f, 5.0f);
	pointLights[1].diffuse = glm::vec3(0.0f, 1.0f, 0.0f); // Green light
	pointLights[1].linear = 0.22f;
	pointLights[1].quadratic = 0.20f;

	pointLights[2].position = glm::vec3(-5.0f, 1.5f, -3.0f);
	pointLights[2].diffuse = glm::vec3(0.0f, 0.0f, 1.0f); // Blue light
	pointLights[2].linear = 0.14f;
	pointLights[2].quadratic = 0.07f;

	// Initialize spot light
	spotLight.direction = glm::vec3(0.0f, 0.0f, 1.0f);
	spotLight.cutOff = glm::cos(glm::radians(12.5f));
	spotLight.outerCutOff = glm::cos(glm::radians(17.5f));

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

bool App::checkFloorCollision(const glm::vec3 &position, float playerHalfHeight, float &floorHeight)
{
	floorHeight = -FLT_MAX;

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (auto &floorModel : floor)
	{
		float currentFloorY = -FLT_MAX;

		if (floorModel.type == Model::FLAT_FLOOR)
		{
			float minX = floorModel.origin.x - floorModel.width / 4;// + 7.5; //- camera.playerRadius;
			float maxX = floorModel.origin.x + floorModel.width / 4;// - 7.5; //+ camera.playerRadius;
			float minZ = floorModel.origin.z - floorModel.depth / 4;// + 7.5; //- camera.playerRadius;
			float maxZ = floorModel.origin.z + floorModel.depth / 4;// - 7.5; //+ camera.playerRadius;
			//std::cout << "minX: " << minX << ", maxX: " << maxX << ", minZ: " << minZ << ", maxZ: " << maxZ << std::endl;
			//std::cout << "width: " << floorModel.width << ", depth: " << floorModel.depth << std::endl;
			// Check flat floor collision
			if (position.x >= minX && position.x <= maxX &&
				position.z >= minZ && position.z <= maxZ)
			{
				currentFloorY = floorModel.origin.y + 0.55f;
			}
		}
		else if (floorModel.type == Model::HEIGHTMAP)
		{

			// Check heightmap collision
			if (position.x >= floorModel.origin.x - floorModel.width / 4 &&
				position.x <= floorModel.origin.x + floorModel.width / 4 &&
				position.z >= floorModel.origin.z - floorModel.depth / 4 &&
				position.z <= floorModel.origin.z + floorModel.depth / 4)
			{
				currentFloorY = floorModel.origin.y + floorModel.getHeightAt(position.x, position.z);
			}
		}

		if (currentFloorY > floorHeight)
		{
			floorHeight = currentFloorY;
		}
		// floorModel.draw();
	}
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return (position.y - playerHalfHeight) <= floorHeight;
}

// bool checkObjectCollision(const glm::vec3& position, const glm::vec3& size) {
//	for (auto& model : models) {
//		// Skip transparent models and floor models
//		if (model.transparent) continue;
//
//		// Simple AABB collision check
//		glm::vec3 modelMin = model.origin - glm::vec3(0.5f);
//		glm::vec3 modelMax = model.origin + glm::vec3(0.5f);
//
//		glm::vec3 playerMin = position - size;
//		glm::vec3 playerMax = position + size;
//
//		if (playerMax.x > modelMin.x && playerMin.x < modelMax.x &&
//			playerMax.y > modelMin.y && playerMin.y < modelMax.y &&
//			playerMax.z > modelMin.z && playerMin.z < modelMax.z) {
//			return true;
//		}
//	}
//	return false;
// }

int App::run(void)
{
	if (!window)
		return -1;

	double lastTime = glfwGetTime();
	int frameCount = 0;
	double startTime = lastTime; // Moved startTime here for clarity

	try
	{
		glEnable(GL_DEPTH_TEST);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
		if (uniform_color_location == -1)
		{
			std::cerr << "Uniform 'uniform_Color' not found.\n";
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!glfwWindowShouldClose(window))
		{
			double currentTime = glfwGetTime();
			float deltaTime = static_cast<float>(currentTime - lastFrameTime);
			lastFrameTime = currentTime;
			float totalTime = static_cast<float>(currentTime - startTime);

			// Update FPS
			frameCount++;
			if (currentTime - lastFpsUpdate >= 1.0)
			{
				double fps = frameCount / (currentTime - lastFpsUpdate);
				std::string title = "FPS: " + std::to_string(static_cast<int>(fps + 0.5)) +
									" | VSync: " + (vsyncEnabled ? "On" : "Off");
				glfwSetWindowTitle(window, title.c_str());
				frameCount = 0;
				lastFpsUpdate = currentTime;
			}

			float angle = totalTime * 15.0f;
			sun.direction = glm::normalize(glm::vec3(
				sin(glm::radians(angle)),
				cos(glm::radians(angle)),
				0.0f));

			float sunHeight = sun.direction.y;
			sun.ambient = glm::vec3(0.2f) * (0.75f + 0.75f * sunHeight);
			sun.diffuse = glm::vec3(0.5f) * (0.75f + 0.75f * sunHeight);

			// Update sun model position
			float distance = 20.0f; // Closer sun
			glm::vec3 sunPosition = sun.direction * distance;
			models[sunModelIndex].origin = sunPosition;

			// std::cout << "sun.direction.y: " << sun.direction.y << ", ambient: " << sun.ambient.x << ", diffuse: " << sun.diffuse.x << std::endl;

			// Animate spheres (orbiting only, no rotation)
			float angle1 = totalTime * 1.0f;
			models[sphere1Index].origin.x = -2.0f + 3.0f * cos(angle1);
			models[sphere1Index].origin.z = 0.0f + 3.0f * sin(angle1);
			models[sphere1Index].origin.y = 7.0f;

			float angle2 = totalTime * 1.5f + 2.0f * 3.1415926535f / 3.0f;
			models[sphere2Index].origin.x = -2.0f + 3.0f * cos(angle2);
			models[sphere2Index].origin.y = 7.0f + 3.0f * sin(angle2);
			models[sphere2Index].origin.z = 0.0f;

			float angle3 = totalTime * 2.0f + 4.0f * 3.1415926535f / 3.0f;
			models[sphere3Index].origin.y = 7.0f + 3.0f * cos(angle3);
			models[sphere3Index].origin.z = 0.0f + 3.0f * sin(angle3);
			models[sphere3Index].origin.x = -2.0f;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::vec3 movement = camera.ProcessInput(window, deltaTime);
			glm::vec3 newPosition = camera.Position + movement;

			// Player collision parameters
			const float playerHalfHeight = camera.playerHeight / 2.0f;
			const glm::vec3 playerSize(camera.playerRadius, playerHalfHeight, camera.playerRadius);

			// Floor collision
			float floorHeight;
			if (checkFloorCollision(newPosition, playerHalfHeight, floorHeight))
			{
				// Snap to floor and stop vertical movement
				newPosition.y = floorHeight + playerHalfHeight;
				camera.Velocity.y = 0.0f;
				camera.isGrounded = true;
			}
			else
			{
				camera.isGrounded = false;
			}

			// Object collision checks (separate axes)
			// bool collisionX = checkObjectCollision(glm::vec3(newPosition.x, camera.Position.y, camera.Position.z), playerSize);
			// bool collisionY = checkObjectCollision(glm::vec3(camera.Position.x, newPosition.y, camera.Position.z), playerSize);
			// bool collisionZ = checkObjectCollision(glm::vec3(camera.Position.x, camera.Position.y, newPosition.z), playerSize);

			// Update position with collision response
			// if (collisionX) newPosition.x = camera.Position.x;
			// if (collisionY) {
			//	newPosition.y = camera.Position.y;
			//	camera.Velocity.y = 0.0f; // Stop vertical movement if hitting ceiling
			//}
			// if (collisionZ) newPosition.z = camera.Position.z;

			// Update final camera position
			camera.Position = newPosition;

			// Update spot light to follow camera
			spotLight.position = camera.Position;
			spotLight.direction = camera.Front;

			// Update all light uniforms
			UpdateLightUniforms(models[0].shader); // Assuming first model has the shader

			glUseProgram(shader_prog_ID);

			if (uniform_color_location != -1)
				glUniform4f(uniform_color_location, r, g, b, a);

			// Update view and projection matrices
			glm::mat4 viewMatrix = camera.GetViewMatrix();
			glUniformMatrix4fv(glGetUniformLocation(shader_prog_ID, "uV_m"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(glGetUniformLocation(shader_prog_ID, "uP_m"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			// Draw floor
			// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			for (auto &model : floor)
			{
				model.update(totalTime);
				model.draw();
			}
			// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// Update view position
			glm::vec3 cameraPos = camera.Position;
			GLint viewPosLoc = glGetUniformLocation(shader_prog_ID, "viewPos");
			if (viewPosLoc != -1)
				glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

			// Update sun uniforms
			GLint sunDirLoc = glGetUniformLocation(shader_prog_ID, "sun.direction");
			if (sunDirLoc != -1)
				glUniform3fv(sunDirLoc, 1, glm::value_ptr(sun.direction));

			GLint sunAmbientLoc = glGetUniformLocation(shader_prog_ID, "sun.ambient");
			if (sunAmbientLoc != -1)
				glUniform3fv(sunAmbientLoc, 1, glm::value_ptr(sun.ambient));

			GLint sunDiffuseLoc = glGetUniformLocation(shader_prog_ID, "sun.diffuse");
			if (sunDiffuseLoc != -1)
				glUniform3fv(sunDiffuseLoc, 1, glm::value_ptr(sun.diffuse));

			GLint sunSpecularLoc = glGetUniformLocation(shader_prog_ID, "sun.specular");
			if (sunSpecularLoc != -1)
				glUniform3fv(sunSpecularLoc, 1, glm::value_ptr(sun.specular));

			// Update models
			for (auto &model : models)
			{
				model.update(totalTime);
			}

			// Draw non-transparent models
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

			// Sort and draw transparent models
			std::sort(transparentModels.begin(), transparentModels.end(), [&](Model *a, Model *b)
					  {
                glm::vec3 posA = a->origin;
                glm::vec3 posB = b->origin;
                return glm::distance(camera.Position, posA) > glm::distance(camera.Position, posB); });

			glEnable(GL_BLEND);
			glDepthMask(GL_FALSE);
			for (auto &model : transparentModels)
			{
				model->draw();
			}
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);

			glfwPollEvents();
			glfwSwapBuffers(window);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "App failed: " << e.what() << std::endl;
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

void App::toggleFullscreen()
{
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);

	if (!isFullscreen)
	{
		// Save current window position and size
		glfwGetWindowPos(window, &windowPosX, &windowPosY);
		glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

		// Switch to fullscreen
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		isFullscreen = true;
	}
	else
	{
		// Restore windowed mode
		glfwSetWindowMonitor(window, nullptr, windowPosX, windowPosY, windowedWidth, windowedHeight, 0);
		isFullscreen = false;
	}
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
		case GLFW_KEY_F11:
			toggleFullscreen(); // 🔁 Toggle fullscreen when F11 is pressed
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
		case GLFW_KEY_L:
			spotLightEnabled = !spotLightEnabled;
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
