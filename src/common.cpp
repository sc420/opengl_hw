#include "as/common.hpp"

/*******************************************************************************
 * Include Header-only Libraries
 ******************************************************************************/

#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)

// Load tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

// Load stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#pragma warning(pop)

/*******************************************************************************
 * Private Method Declarations
 ******************************************************************************/

void GLAPIENTRY GLMessageCallback(const GLenum source, const GLenum type,
                                  const GLuint id, const GLenum severity,
                                  const GLsizei length, const GLchar* message,
                                  const void* userParam);

/*******************************************************************************
 * Public Methods
 ******************************************************************************/

/**
 * Initialize GLEW.
 *
 * Reference: http://glew.sourceforge.net/basic.html
 */
void as::InitGLEW() {
  const GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    throw std::runtime_error("Could not initialize GLEW");
  }
}

/**
 * Print OpenGL context information.
 *
 * Reference: https://www.khronos.org/opengl/wiki/OpenGL_Context
 */
void as::PrintGLContextInfo(const bool print_extensions,
                            const bool print_supported_glsl_versions) {
  std::cerr << "GL version: " << glGetString(GL_VERSION) << std::endl;
  std::cerr << "GL vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cerr << "GL renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cerr << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;
  if (print_extensions) {
    GLint num_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
    std::cerr << "Number of GL extensions: " << num_ext << std::endl;
    std::cerr << "Extensions:" << std::endl;
    for (GLint i = 0; i < num_ext; i++) {
      std::cerr << glGetStringi(GL_EXTENSIONS, i) << std::endl;
    }
    std::cerr << std::endl;
  }
  if (print_supported_glsl_versions) {
    GLint num_ver;
    glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &num_ver);
    std::cerr << "Number of supported GLSL versions: " << num_ver + 1
              << std::endl;
    std::cerr << "Supported GLSL versions:" << std::endl;
    for (GLint i = 0; i < num_ver; i++) {
      std::cerr << glGetStringi(GL_SHADING_LANGUAGE_VERSION, i) << std::endl;
    }
    // Also print the current version
    std::cerr << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cerr << std::endl;
  }
}

/**
 * Enable OpenGL to catch errors and report.
 *
 * Reference: https://www.khronos.org/opengl/wiki/Debug_Output
 */
void as::EnableCatchingGLError(const bool stop_when_error) {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(GLMessageCallback, &stop_when_error);
}

/*******************************************************************************
 * Private Methods
 ******************************************************************************/

void GLAPIENTRY GLMessageCallback(const GLenum source, const GLenum type,
                                  const GLuint id, const GLenum severity,
                                  const GLsizei length, const GLchar* message,
                                  const void* user_param) {
  /* Filter based on types */
  // Ignore API_ID_RECOMPILE_FRAGMENT_SHADER
  if (type == 0x8250) {
    return;
  }
  /* Print the message */
  std::cerr << "GL Callback: "
            << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
            << " type = 0x" << std::hex << type << ", severity = 0x" << std::hex
            << severity << ", message = " << message << std::endl;
  /* Check whether to stop */
  const bool stop_when_error = (*static_cast<const bool*>(user_param));
  if (stop_when_error) {
    throw std::runtime_error("GL error has occurred, stopping now");
  }
}
