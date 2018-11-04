#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class ShaderManager {
 public:
  ~ShaderManager();

  void CreateShader(const GLenum type, const std::string& path,
                    const std::string& name);

  GLuint GetShaderHdlr(const std::string& name);

 private:
  std::map<std::string, GLuint> hdlrs_;

  GLchar* LoadShaderSource(const std::string& file);

  void FreeShaderSource(const GLchar* src);

  void CheckShaderCompilation(const GLuint hdlr);
};
