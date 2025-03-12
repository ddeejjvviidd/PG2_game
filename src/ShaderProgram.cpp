#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

// set uniform according to name
// https://docs.gl/gl4/glUniform

ShaderProgram::ShaderProgram(const std::filesystem::path &VS_file, const std::filesystem::path &FS_file)
{
	std::vector<GLuint> shader_ids;
	try
	{

		// compile shaders and store IDs for linker
		shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
		shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

		// link all compiled shaders into shader_program
		ID = link_shader(shader_ids);
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << "ShaderProgram creation failed: " << e.what() << std::endl;
		ID = 0; // Ensure ID is invalid on failure
		throw;	// Re-throw to propagate the error
	}
}

void ShaderProgram::setUniform(const std::string &name, const float val)
{
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1)
	{
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const std::string &name, const int val)
{
	// TODO: implement
}

void ShaderProgram::setUniform(const std::string &name, const glm::vec3 val)
{
	// TODO: get location
	// glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string &name, const glm::vec4 in_vec4)
{
	// TODO: implement
}

void ShaderProgram::setUniform(const std::string &name, const glm::mat3 val)
{
	// TODO: get location
	// glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string &name, const glm::mat4 val)
{
	// TODO: implement
}

std::string ShaderProgram::getShaderInfoLog(const GLuint obj)
{
	GLint log_length = 0;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length <= 0) return "";

    std::vector<char> log(log_length);
    glGetShaderInfoLog(obj, log_length, nullptr, log.data());
    return std::string(log.data());
}

std::string ShaderProgram::getProgramInfoLog(const GLuint obj)
{
	// TODO: implement, print info
	return "Not implemented";
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path &source_file, const GLenum type)
{
	// implement, try to compile, check for error; if any, print compiler result (or print allways, if you want to see warnings as well)
	// if err, throw error
	std::string source = textFileRead(source_file);
	const char *source_cstr = source.c_str();
	GLuint shader_h = glCreateShader(type);
	glShaderSource(shader_h, 1, &source_cstr, NULL);
	glCompileShader(shader_h);
	
	// Check for compilation errors
    GLint success;
    glGetShaderiv(shader_h, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::string log = getShaderInfoLog(shader_h);
        std::cerr << "Shader compilation failed (" << source_file << "):\n" << log << std::endl;
        glDeleteShader(shader_h);
        throw std::runtime_error("Shader compilation failed");
    }

    // Print compilation log even on success (for warnings)
    std::string log = getShaderInfoLog(shader_h);
    if (!log.empty()) {
        std::cout << "Shader compilation log (" << source_file << "):\n" << log << std::endl;
    }

	return shader_h;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids)
{
	GLuint prog_h = glCreateProgram();

	for (const GLuint id : shader_ids)
		glAttachShader(prog_h, id);

	glLinkProgram(prog_h);
	
	// implement: check link result, print info & throw error (if any)
	// Check link result
    GLint success;
    glGetProgramiv(prog_h, GL_LINK_STATUS, &success);
    if (!success) {
        std::string log = getProgramInfoLog(prog_h);
        std::cerr << "Shader program linking failed:\n" << log << std::endl;
        glDeleteProgram(prog_h);
        for (const GLuint id : shader_ids) {
            glDeleteShader(id);
        }
        throw std::runtime_error("Shader program linking failed");
    }

    // Print link log even on success (for warnings)
    std::string log = getProgramInfoLog(prog_h);
    if (!log.empty()) {
        std::cout << "Shader program link log:\n" << log << std::endl;
    }

    // Clean up shaders after linking
    for (const GLuint id : shader_ids) {
        glDetachShader(prog_h, id);
        glDeleteShader(id);
    }
	
	return prog_h;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path &filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error(std::string("Error opening file: ") + filename.string());
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
