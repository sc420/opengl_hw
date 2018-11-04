#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class ShaderManager {
 public:
  void CreateShader(const GLenum type, const std::string& path,
                    const std::string& name);

  GLuint GetShaderHdlr(const std::string& name);

 private:
  std::map<std::string, GLuint> hdlrs_;

  void CheckShaderCompilation(const GLuint hdlr);

  GLchar* LoadShaderSource(const std::string& file);

  void FreeShaderSource(GLchar* src);
};
