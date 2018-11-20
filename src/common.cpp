#include "as/common.hpp"

/*******************************************************************************
 * Include Header-only Libraries
 ******************************************************************************/

#pragma warning(push, 0)

// Load tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

// Load stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#pragma warning(pop)

/*******************************************************************************
 * Private Methods
 ******************************************************************************/

void GLAPIENTRY GLMessageCallback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar* message,
                                  const void* userParam) {
  /* Filter based on types */
  // Ignore API_ID_RECOMPILE_FRAGMENT_SHADER
  if (type == 0x8250) {
    return;
  }
  /* Print the message */
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
          message);
}

/*******************************************************************************
 * Public Methods
 ******************************************************************************/

as::texture_data as::load_png(const char* path) {
  texture_data texture;
  int n;
  stbi_uc* data = stbi_load(path, &texture.width, &texture.height, &n, 4);
  if (data != NULL) {
    texture.data = new unsigned char[texture.width * texture.height * 4 *
                                     sizeof(unsigned char)];
    memcpy(texture.data, data,
           texture.width * texture.height * 4 * sizeof(unsigned char));
    // vertical-mirror image data
    for (int i = 0; i < texture.width; i++) {
      for (int j = 0; j < texture.height / 2; j++) {
        for (int k = 0; k < 4; k++) {
          std::swap(
              texture.data[(j * texture.width + i) * 4 + k],
              texture.data[((texture.height - j - 1) * texture.width + i) * 4 +
                           k]);
        }
      }
    }
    stbi_image_free(data);
  }
  return texture;
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
void as::EnableCatchingGLError() {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(GLMessageCallback, 0);
}
