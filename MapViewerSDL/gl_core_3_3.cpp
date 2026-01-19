#include "gl_core_3_3.hpp"
#include <iostream>

// Define function pointers
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;

PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;

PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;

PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;

bool LoadGL33Functions()
{
    #define LOAD_GL_FUNC(name) \
        name = (decltype(name))SDL_GL_GetProcAddress(#name); \
        if (!name) { \
            std::cerr << "Failed to load GL function: " << #name << std::endl; \
            return false; \
        }

    LOAD_GL_FUNC(glCreateShader);
    LOAD_GL_FUNC(glShaderSource);
    LOAD_GL_FUNC(glCompileShader);
    LOAD_GL_FUNC(glGetShaderiv);
    LOAD_GL_FUNC(glGetShaderInfoLog);
    LOAD_GL_FUNC(glDeleteShader);

    LOAD_GL_FUNC(glCreateProgram);
    LOAD_GL_FUNC(glAttachShader);
    LOAD_GL_FUNC(glLinkProgram);
    LOAD_GL_FUNC(glGetProgramiv);
    LOAD_GL_FUNC(glGetProgramInfoLog);
    LOAD_GL_FUNC(glUseProgram);
    LOAD_GL_FUNC(glDeleteProgram);

    LOAD_GL_FUNC(glGetUniformLocation);
    LOAD_GL_FUNC(glUniformMatrix4fv);

    LOAD_GL_FUNC(glGenVertexArrays);
    LOAD_GL_FUNC(glBindVertexArray);
    LOAD_GL_FUNC(glDeleteVertexArrays);

    LOAD_GL_FUNC(glGenBuffers);
    LOAD_GL_FUNC(glBindBuffer);
    LOAD_GL_FUNC(glBufferData);
    LOAD_GL_FUNC(glDeleteBuffers);

    LOAD_GL_FUNC(glVertexAttribPointer);
    LOAD_GL_FUNC(glEnableVertexAttribArray);
    LOAD_GL_FUNC(glDisableVertexAttribArray);

    #undef LOAD_GL_FUNC

    std::cout << "OpenGL 3.3 functions loaded successfully" << std::endl;
    return true;
}
