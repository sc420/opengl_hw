#pragma warning(push, 0)
#include "assignment/common.hpp"

// Load tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include <TinyOBJ/tiny_obj_loader.h>

// Load stb
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#pragma warning(pop)

texture_data load_png(const char* path) {
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

void DumpGLInfo(void) {
  std::cerr << "GL Vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cerr << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cerr << "GL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cerr << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;
  // std::cerr << "GL Extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
}

void EnableCatchingError() {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar* message, const void* userParam) {
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
