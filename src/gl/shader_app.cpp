#include "as/gl/shader_app.hpp"

namespace fs = std::experimental::filesystem;

std::string app::ShaderApp::GetProgramName() const { return GetId(); }

std::string app::ShaderApp::GetShaderPath(
    const ShaderTypes& shader_type) const {
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

void app::ShaderApp::RegisterGLManagers(as::GLManagers& gl_managers) {
  gl_managers_ = &gl_managers;
}

void app::ShaderApp::Init() {
  CreateShaders();
  CreatePrograms();
  InitUniformBlocks();
}

void app::ShaderApp::CreateShaders() {
  as::ShaderManager& shader_manager = gl_managers_->GetShaderManager();
  const std::string& vertex_path = GetShaderPath(ShaderTypes::kVertex);
  const std::string& fragment_path = GetShaderPath(ShaderTypes::kFragment);
  shader_manager.CreateShader(vertex_path, GL_VERTEX_SHADER, vertex_path);
  shader_manager.CreateShader(fragment_path, GL_FRAGMENT_SHADER, fragment_path);
}

void app::ShaderApp::CreatePrograms() {
  as::ProgramManager& program_manager = gl_managers_->GetProgramManager();
  const std::string& program_name = GetProgramName();
  const std::string& vertex_path = GetShaderPath(ShaderTypes::kVertex);
  const std::string& fragment_path = GetShaderPath(ShaderTypes::kFragment);
  program_manager.CreateProgram(program_name);
  program_manager.AttachShader(program_name, vertex_path);
  program_manager.AttachShader(program_name, fragment_path);
  program_manager.LinkProgram(program_name);
}
