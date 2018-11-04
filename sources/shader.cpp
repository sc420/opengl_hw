#include "assignment/shader.hpp"

ShaderManager::~ShaderManager() {
  // Delete all shader objects
  for (const auto& pair : hdlrs_) {
    glDeleteShader(pair.second);
  }
}

void ShaderManager::CreateShader(const GLenum type, const std::string& path,
                                 const std::string& name) {
  // Create a shader object
  const GLuint hdlr = glCreateShader(type);
  // Load the shader source
  const GLchar* source = LoadShaderSource(path);
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

void ShaderManager::FreeShaderSource(const GLchar* src) { delete src; }

void ShaderManager::CheckShaderCompilation(const GLuint hdlr) {
  GLint status = -1;
  // Get compilation status
  glGetShaderiv(hdlr, GL_COMPILE_STATUS, &status);
  // Report the log if compilation is unsuccessful
  if (status == GL_FALSE) {
    // Get the length of the log
    GLint len = 0;
    glGetShaderiv(hdlr, GL_INFO_LOG_LENGTH, &len);
    // Get the log
    std::string log(static_cast<int>(len), '\0');
    glGetShaderInfoLog(hdlr, len, &len, static_cast<GLchar*>(&log[0]));
    // Throw an error
    throw std::runtime_error(log);
  }
}
