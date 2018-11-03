#pragma warning(push, 0)
#include "assignment/common.hpp"

// Load tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include <TinyOBJ/tiny_obj_loader.h>

// Load stb
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#pragma warning(pop)

// Print OpenGL context related information.
void dumpInfo(void) {
  printf("Vendor: %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version: %s\n", glGetString(GL_VERSION));
  printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void shaderLog(GLuint shader) {
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    GLchar* errorLog = new GLchar[maxLength];
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    printf("%s\n", errorLog);
    delete[] errorLog;
  }
}

void printGLError() {
  GLenum code = glGetError();
  switch (code) {
    case GL_NO_ERROR:
      std::cout << "GL_NO_ERROR" << std::endl;
      break;
    case GL_INVALID_ENUM:
      std::cout << "GL_INVALID_ENUM" << std::endl;
      break;
    case GL_INVALID_VALUE:
      std::cout << "GL_INVALID_VALUE" << std::endl;
      break;
    case GL_INVALID_OPERATION:
      std::cout << "GL_INVALID_OPERATION" << std::endl;
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
      break;
    case GL_OUT_OF_MEMORY:
      std::cout << "GL_OUT_OF_MEMORY" << std::endl;
      break;
    case GL_STACK_UNDERFLOW:
      std::cout << "GL_STACK_UNDERFLOW" << std::endl;
      break;
    case GL_STACK_OVERFLOW:
      std::cout << "GL_STACK_OVERFLOW" << std::endl;
      break;
    default:
      std::cout << "GL_ERROR" << std::endl;
  }
}

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

void GLAPIENTRY
MessageCallback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam)
{
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
    type, severity, message);
}
