//Kevin: This is my modified version of Apoorva Joshi's super-lightweight OpenGL loader
//which was in turn based on the loader Fabian Giesen used for Bink
//I am extremely grateful to both of you for sharing your code online <3
// http://apoorvaj.io/loading-opengl-without-glew.html
//-------------------------------------------------------------------------------------

/*
    gl_lite.h - Single-header multi-platform OpenGL function loader
    ----------------------------------------------------------------------------
    USAGE
    ----------------------------------------------------------------------------
    1) Add the following lines in exactly one of your cpp files to compile the
       implementation:
           #define GL_LITE_IMPLEMENTATION
           #include "gl_lite.h"
    2) In all other files in which you want to use OpenGL functions, simply 
       include this header file as follows:
           #include "gl_lite.h"
    3) Call gl_lite_init() before using any OpenGL function and after you have a
       valid OpenGL context.
    ----------------------------------------------------------------------------
    LICENSE
    ----------------------------------------------------------------------------
    This software is in the public domain. Where that dedication is not
    recognized, you are granted a perpetual, irrevocable license to copy,
    distribute, and modify this file as you see fit.
*/
#ifndef GL_LITE_H
#define GL_LITE_H

#if defined(__APPLE__)
#define __gl_h_
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl3.h>
#else //LINUX & WIN32

#include <stddef.h>

//This doesn't work for some reason :/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif

//Dear GCC: Shut up about variadic macros please!
#pragma GCC system_header

#if defined(__linux__)
#include <dlfcn.h>
#define GLDECL // Empty define
#define PAPAYA_GL_LIST_WIN32 // Empty define
#endif // __linux__

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef near
#undef far
#undef wherever_you_are

#define GLDECL WINAPI

// Acquired from: https://www.opengl.org/registry/api/GL/glext.h
#define GL_ARRAY_BUFFER                   0x8892
#define GL_COMPILE_STATUS                 0x8B81
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_LINK_STATUS                    0x8B82
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_STATIC_DRAW                    0x88E4
#define GL_TEXTURE0                       0x84C0
#define GL_VERTEX_SHADER                  0x8B31

typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

#define PAPAYA_GL_LIST_WIN32 \
    /* ret, name, params */ \
    GLE(void,      BlendEquation,           GLenum mode) \
    GLE(void,      ActiveTexture,           GLenum texture) \
    /* end */

#endif // _WIN32

#include <GL/gl.h>

#define PAPAYA_GL_LIST \
    /* ret, name, params */ \
    GLE(void,      AttachShader,            GLuint program, GLuint shader) \
    GLE(void,      BindAttribLocation,      GLuint program, GLuint index, const GLchar *name) \
    GLE(void,      BindBuffer,              GLenum target, GLuint buffer) \
    GLE(void,      BindFramebuffer,         GLenum target, GLuint framebuffer) \
    GLE(void,      BindVertexArray,         GLuint array) \
    GLE(void,      BufferData,              GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) \
    GLE(void,      BufferSubData,           GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data) \
    GLE(GLenum,    CheckFramebufferStatus,  GLenum target) \
    GLE(void,      ClearBufferfv,           GLenum buffer, GLint drawbuffer, const GLfloat * value) \
    GLE(void,      CompileShader,           GLuint shader) \
    GLE(GLuint,    CreateProgram,           void) \
    GLE(GLuint,    CreateShader,            GLenum type) \
    GLE(void,      DeleteBuffers,           GLsizei n, const GLuint *buffers) \
    GLE(void,      DeleteFramebuffers,      GLsizei n, const GLuint *framebuffers) \
    GLE(void,      DeleteProgram,           GLuint program) \
    GLE(void,      DeleteShader,            GLuint shader) \
    GLE(void,      DetachShader,            GLuint program, GLuint shader) \
    GLE(void,      EnableVertexAttribArray, GLuint index) \
    GLE(void,      DrawBuffers,             GLsizei n, const GLenum *bufs) \
    GLE(void,      FramebufferTexture2D,    GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
    GLE(void,      GenBuffers,              GLsizei n, GLuint *buffers) \
    GLE(void,      GenFramebuffers,         GLsizei n, GLuint * framebuffers) \
    GLE(void,      GenVertexArrays,         GLsizei n, GLuint *arrays) \
    GLE(GLint,     GetAttribLocation,       GLuint program, const GLchar *name) \
    GLE(void,      GetProgramInfoLog,       GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(void,      GetProgramiv,            GLuint program, GLenum pname, GLint *params) \
    GLE(void,      GetShaderInfoLog,        GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(void,      GetShaderiv,             GLuint shader, GLenum pname, GLint *params) \
    GLE(GLint,     GetUniformLocation,      GLuint program, const GLchar *name) \
    GLE(void,      LinkProgram,             GLuint program) \
    GLE(void,      ShaderSource,            GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) \
    GLE(void,      Uniform1i,               GLint location, GLint v0) \
    GLE(void,      Uniform1f,               GLint location, GLfloat v0) \
    GLE(void,      Uniform2f,               GLint location, GLfloat v0, GLfloat v1) \
    GLE(void,      Uniform2fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      Uniform3f,               GLint location, GLfloat v0, GLfloat v1, GLfloat v3) \
    GLE(void,      Uniform3fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      Uniform4f,               GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    GLE(void,      Uniform4fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      UniformMatrix4fv,        GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GLE(void,      UseProgram,              GLuint program) \
    GLE(void,      VertexAttribPointer,     GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) \
    /* end */

#define GLE(ret, name, ...) typedef ret GLDECL name##proc(__VA_ARGS__); extern name##proc * gl##name;
PAPAYA_GL_LIST
PAPAYA_GL_LIST_WIN32
#undef GLE

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif //__APPLE__

bool gl_lite_init();

#endif //GL_LITE_H

// =============================================================================

#ifdef GL_LITE_IMPLEMENTATION

#if defined(__APPLE__)
bool gl_lite_init() {return true;} //Don't need to load function pointers

#else

//This doesn't work for some reason :/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif

#define GLE(ret, name, ...) name##proc * gl##name;
PAPAYA_GL_LIST
PAPAYA_GL_LIST_WIN32
#undef GLE

bool gl_lite_init()
{
#if defined(__linux__)

    void* libGL = dlopen("libGL.so", RTLD_LAZY);
    if (!libGL) {
        printf("ERROR: libGL.so couldn't be loaded\n");
        return false;
    }

    #define GLE(ret, name, ...)                                                    \
            gl##name = (name##proc *) dlsym(libGL, "gl" #name);                    \
            if (!gl##name) {                                                       \
                printf("Function gl" #name " couldn't be loaded from libGL.so\n"); \
                return false;                                                      \
            }
        PAPAYA_GL_LIST
    #undef GLE

#elif defined(_WIN32)

    HINSTANCE dll = LoadLibraryA("opengl32.dll");
    typedef PROC WINAPI wglGetProcAddressproc(LPCSTR lpszProc);
    if (!dll) {
        OutputDebugStringA("opengl32.dll not found.\n");
        return false;
    }
    wglGetProcAddressproc* wglGetProcAddress =
        (wglGetProcAddressproc*)GetProcAddress(dll, "wglGetProcAddress");

    #define GLE(ret, name, ...)                                                                    \
            gl##name = (name##proc *)wglGetProcAddress("gl" #name);                                \
            if (!gl##name) {                                                                       \
                OutputDebugStringA("Function gl" #name " couldn't be loaded from opengl32.dll\n"); \
                return false;                                                                      \
            }
        PAPAYA_GL_LIST
        PAPAYA_GL_LIST_WIN32
    #undef GLE

#else
    #error "GL loading for this platform is not implemented yet."
#endif

    return true;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif //__APPLE__

#endif //GL_LITE_IMPLEMENTATION
