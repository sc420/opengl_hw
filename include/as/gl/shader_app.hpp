#pragma once

#include "as/gl/gl_tools.hpp"

namespace app {
enum class ShaderTypes { kVertex, kFragment };

class ShaderApp {
 public:
  virtual std::string GetId() const = 0;

  std::string GetProgramName() const;

  std::string GetShaderPath(const ShaderTypes shader_type) const;

  void RegisterManagers(as::ProgramManager &program_manager,
                        as::ShaderManager &shader_manager);

  void Init();

 private:
  as::ProgramManager *program_manager_;

  as::ShaderManager *shader_manager_;

  void CreateShaders();

  void CreatePrograms();
};
}  // namespace app
