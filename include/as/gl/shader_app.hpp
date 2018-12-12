#pragma once

#include "as/gl/gl_tools.hpp"

namespace app {
enum class ShaderTypes { kVertex, kFragment };

class ShaderApp {
 public:
  virtual std::string GetId() const = 0;

  virtual void InitUniformBlocks() = 0;

  std::string GetProgramName() const;

  std::string GetShaderPath(const ShaderTypes &shader_type) const;

  void RegisterGLManagers(as::GLManagers &gl_managers);

  void Init();

  void Use() const;

 protected:
  as::GLManagers *gl_managers_;

 private:
  void CreateShaders();

  void CreatePrograms();
};
}  // namespace app
