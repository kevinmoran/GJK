#pragma once
// #include <GLFW/glfw3.h>
// #include <stdio.h>
// #include "Input.h"

bool init_gl(GLFWwindow* &window, const char* title, int window_width, int window_height) {

	/* start GL context and O/S window using the GLFW helper library */
	if(!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		getchar();
		return false;
	}

	#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#endif
	
	window = glfwCreateWindow(window_width, window_height, "GJK", NULL, NULL);
	if(!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		getchar();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	//Setup callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_resize_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);
	
	//Load OpenGL functions
	if(!gl_lite_init()){
		printf("Error in gl_lite_init\n");
		return false;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	printf("GLSL version supported: %s\n", glsl_version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	return true;
}

//TODO put this somewhere sensible!
//Note: for MSVC, change __builtin_trap() to __debugbreak()
#define assert(exp) \
	{if(!(exp)) { \
		printf("Assertion failed in %s, Line %d:\n%s\n...", __FILE__, __LINE__, #exp); \
		__builtin_trap(); \
	}} \

#define check_gl_error() _checkOglError(__FILE__, __LINE__)

static int _checkOglError(const char *file, int line){
    GLenum glErr = glGetError();
    if(glErr != GL_NO_ERROR) {
        printf("glError in file %s @ line %d:\n%d - ", file, line, glErr);
		switch(glErr) {
			case GL_INVALID_OPERATION:				printf("INVALID_OPERATION\n");				return 1;
			case GL_INVALID_ENUM:					printf("INVALID_ENUM\n");					return 1;
			case GL_INVALID_VALUE:					printf("INVALID_VALUE\n");      			return 1;
			case GL_OUT_OF_MEMORY:					printf("OUT_OF_MEMORY\n");      			return 1;
			case GL_INVALID_FRAMEBUFFER_OPERATION:	printf("INVALID_FRAMEBUFFER_OPERATION\n");	return 1;
			default:								printf("UNRECOGNISED ERROR\n");				return 1;
        }
    }
    return 0;
}