#include "as/gl/shader_app.hpp"

namespace fs = std::experimental::filesystem;

std::string app::ShaderApp::GetProgramName() const { return GetId(); }

std::string app::ShaderApp::GetShaderPath(const ShaderTypes shader_type) const {
  const std::string& id = GetId();
  std::string ext;
  switch (shader_type) {
    case ShaderTypes::kVertex: {
      ext = ".vert";
    } break;
    case ShaderTypes::kFragment: {
      ext = ".frag";
    } break;
    default: { throw std::runtime_error("Unknown shader type"); }
  }
  fs::path path("assets/shaders");
  path = path / (id + ext);
  return path.string();
}

void app::ShaderApp::RegisterManagers(as::ProgramManager& program_manager,
                                      as::ShaderManager& shader_manager) {
  program_manager_ = &program_manager;
  shader_manager_ = &shader_manager;
}

void app::ShaderApp::Init() {
  CreateShaders();
  CreatePrograms();
}

void app::ShaderApp::CreateShaders() {
  const std::string& vertex_path = GetShaderPath(ShaderTypes::kVertex);
  const std::string& fragment_path = GetShaderPath(ShaderTypes::kFragment);
  shader_manager_->CreateShader(vertex_path, GL_VERTEX_SHADER, vertex_path);
  shader_manager_->CreateShader(fragment_path, GL_FRAGMENT_SHADER,
                                fragment_path);
}

void app::ShaderApp::CreatePrograms() {
  const std::string& program_name = GetProgramName();
  const std::string& vertex_path = GetShaderPath(ShaderTypes::kVertex);
  const std::string& fragment_path = GetShaderPath(ShaderTypes::kFragment);
  program_manager_->CreateProgram(program_name);
  program_manager_->AttachShader(program_name, vertex_path);
  program_manager_->AttachShader(program_name, fragment_path);
  program_manager_->LinkProgram(program_name);
}
