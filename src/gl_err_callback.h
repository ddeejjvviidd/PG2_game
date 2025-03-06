#ifndef __gl_err_callback_h__
#define __gl_err_callback_h__

#include <GL/glew.h> 
#ifdef _WIN32
	#include <GL/wglew.h>
#endif
#include <GLFW/glfw3.h>

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#endif //__gl_err_callback_h__
