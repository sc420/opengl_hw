#pragma once

#include "as/common.hpp"

namespace as {
class ShaderManager {
 public:
  ~ShaderManager();

  void CreateShader(const std::string& shader_name, const GLenum type,
                    const std::string& path);

  void DeleteShader(const std::string& shader_name) const;

  GLuint GetShaderHdlr(const std::string& shader_name) const;

 private:
  std::map<std::string, GLuint> hdlrs_;

  std::string LoadShaderSource(const std::string& file) const;

  void CheckShaderCompilation(const GLuint shader_hdlr) const;
};
}  // namespace as
