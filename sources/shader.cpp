#include "assignment/shader.hpp"

ShaderManager::~ShaderManager() {
  // Delete all shader objects
  for (const auto& pair : hdlrs_) {
    glDeleteShader(pair.second);
  }
}

void ShaderManager::CreateShader(const std::string& shader_name, const GLenum type, const std::string& path) {
  // Create a shader object
  const GLuint shader_hdlr = glCreateShader(type);
  // Load the shader source
  std::string src = LoadShaderSource(path);
  // Replace the source code in the shader object
  const char *str = src.c_str();
  glShaderSource(shader_hdlr, 1, &str, NULL);
  // Compile the shader object
  glCompileShader(shader_hdlr);
  // Check the compilation status
  CheckShaderCompilation(shader_hdlr);
  // Save the handler
  hdlrs_[shader_name] = shader_hdlr;
}

void ShaderManager::DeleteShader(const std::string& shader_name) const {
  const GLuint shader_hdlr = GetShaderHdlr(shader_name);
  glDeleteShader(shader_hdlr);
}

GLuint ShaderManager::GetShaderHdlr(const std::string& shader_name) const {
  if (hdlrs_.count(shader_name) == 0) {
    throw std::runtime_error("Could not find the shader name '" + shader_name + "'");
  }
  return hdlrs_.at(shader_name);
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

void ShaderManager::CheckShaderCompilation(const GLuint shader_hdlr)const {
  GLint status = -1;
  // Get compilation status
  glGetShaderiv(shader_hdlr, GL_COMPILE_STATUS, &status);
  // Report the log if compilation is unsuccessful
  if (status == GL_FALSE) {
    // Get the length of the log
    GLint len = 0;
    glGetShaderiv(shader_hdlr, GL_INFO_LOG_LENGTH, &len);
    // Get the log
    std::string log(len, '\0');
    glGetShaderInfoLog(shader_hdlr, len, &len, &log[0]);
    // Throw an error
    throw std::runtime_error(log);
  }
}
