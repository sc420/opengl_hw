#include "assignment/shader.hpp"

void ShaderManager::CreateShader(const GLenum type, const std::string& path,
                                 const std::string& name) {
  // Create a shader object
  GLuint hdlr = glCreateShader(type);
  // Load the shader source
  GLchar* source = LoadShaderSource(path);
  // Replace the source code in the shader object
  glShaderSource(hdlr, 1, &source, NULL);
  // Free the shader source
  FreeShaderSource(source);
  // Compile the shader object
  glCompileShader(hdlr);
  // Check the compilation status
  CheckShaderCompilation(hdlr);
  // Save the handler
  hdlrs_[name] = hdlr;
}

GLuint ShaderManager::GetShaderHdlr(const std::string& name) {
  if (hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the shader name '" + name + "'");
  }
  return hdlrs_.at(name);
}

void ShaderManager::CheckShaderCompilation(const GLuint hdlr) {
  GLint isCompiled = 0;
  // Get compilation status
  glGetShaderiv(hdlr, GL_COMPILE_STATUS, &isCompiled);
  // Report the log if compilation is unsuccessful
  if (isCompiled == GL_FALSE) {
    // Get the length of the log
    GLint max_len = 0;
    glGetShaderiv(hdlr, GL_INFO_LOG_LENGTH, &max_len);
    // Get the log
    std::string log(static_cast<int>(max_len), '\0');
    glGetShaderInfoLog(hdlr, max_len, &max_len, static_cast<GLchar*>(&log[0]));
    // Throw an error
    throw std::runtime_error(log);
  }
}

GLchar* ShaderManager::LoadShaderSource(const std::string& file) {
  FILE* fp;
  fopen_s(&fp, file.c_str(), "rb");
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* src = new char[sz + 1];
  fread(src, sizeof(char), sz, fp);
  src[sz] = '\0';
  fclose(fp);
  return static_cast<GLchar*>(src);
}

void ShaderManager::FreeShaderSource(GLchar* src) { delete src; }
