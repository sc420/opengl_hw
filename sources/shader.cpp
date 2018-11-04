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
  std::string src = LoadShaderSource(path);
  // Replace the source code in the shader object
  const char *str = src.c_str();
  glShaderSource(hdlr, 1, &str, NULL);
  // Compile the shader object
  glCompileShader(hdlr);
  // Check the compilation status
  CheckShaderCompilation(hdlr);
  // Save the handler
  hdlrs_[name] = hdlr;
}

void ShaderManager::DeleteShader(const std::string& name) const {
  const GLuint hdlr = GetShaderHdlr(name);
  glDeleteShader(hdlr);
}

GLuint ShaderManager::GetShaderHdlr(const std::string& name) const {
  if (hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the shader name '" + name + "'");
  }
  return hdlrs_.at(name);
}

std::string ShaderManager::LoadShaderSource(const std::string& file) const {
  FILE* fp;
  fopen_s(&fp, file.c_str(), "rb");
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  std::string src(sz + 1, '\0');
  fread(&src[0], sizeof(char), sz, fp);
  fclose(fp);
  return src;
}

void ShaderManager::CheckShaderCompilation(const GLuint hdlr)const {
  GLint status = -1;
  // Get compilation status
  glGetShaderiv(hdlr, GL_COMPILE_STATUS, &status);
  // Report the log if compilation is unsuccessful
  if (status == GL_FALSE) {
    // Get the length of the log
    GLint len = 0;
    glGetShaderiv(hdlr, GL_INFO_LOG_LENGTH, &len);
    // Get the log
    std::string log(len, '\0');
    glGetShaderInfoLog(hdlr, len, &len, &log[0]);
    // Throw an error
    throw std::runtime_error(log);
  }
}
