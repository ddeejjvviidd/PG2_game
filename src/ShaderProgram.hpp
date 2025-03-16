#pragma once

#include <string>
#include <filesystem>

#include <GL/glew.h>

class ShaderProgram
{
public:
	// you can add more constructors for pipeline with GS, TS etc.
	ShaderProgram(void) = default;															   // does nothing
	ShaderProgram(const std::filesystem::path &VS_file, const std::filesystem::path &FS_file); // TODO: implementation of load, compile, and link shader

	void activate(void) const { glUseProgram(ID); }; // activate shader
	void deactivate(void) { glUseProgram(0); };		 // deactivate current shader program (i.e. activate shader no. 0)

	void clear(void)
	{ // deallocate shader program
		deactivate();
		glDeleteProgram(ID);
		ID = 0;
	}

	GLuint getID() const { return ID; } // Add getter for ID

	// set uniform according to name
	// https://docs.gl/gl4/glUniform
	void setUniform(const std::string &name, const float val);
	void setUniform(const std::string &name, const int val); // TODO: implement
	void setUniform(const std::string &name, const glm::vec3 val);
	void setUniform(const std::string &name, const glm::vec4 val); // TODO: implement
	void setUniform(const std::string &name, const glm::mat3 val);
	void setUniform(const std::string &name, const glm::mat4 val); // TODO: implement

private:
	GLuint ID{0};									 // default = 0, empty shader
	std::string getShaderInfoLog(const GLuint obj);	 // TODO implement: print compiler output
	std::string getProgramInfoLog(const GLuint obj); // TODO implement: print linker output

	GLuint compile_shader(const std::filesystem::path &source_file, const GLenum type);
	// TODO implement: try to load and compile shader; check for shader compilation error, if any, print compiler output using
	// getShaderInfoLog() function

	// TODO implement: try to link all shader IDs to final program; check for linker error, if any, print output using
	// getProgramInfoLog() function
	GLuint link_shader(const std::vector<GLuint> shader_ids);

	std::string textFileRead(const std::filesystem::path &filename); // load text file
};
