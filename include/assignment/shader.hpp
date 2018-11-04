#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class ShaderManager {
 public:
  ~ShaderManager();

  void CreateShader(const GLenum type, const std::string& path,
                    const std::string& name);

  void DeleteShader(const std::string& name) const;

  GLuint GetShaderHdlr(const std::string& name) const;

 private:
  std::map<std::string, GLuint> hdlrs_;

  std::string LoadShaderSource(const std::string& file) const;

  void CheckShaderCompilation(const GLuint hdlr) const;
};
