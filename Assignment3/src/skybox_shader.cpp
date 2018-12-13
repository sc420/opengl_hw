#include "skybox_shader.hpp"

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void shader::SkyboxShader::Init() {
  app::ShaderApp::Init();
  InitUniformBlocks();
}

void shader::SkyboxShader::InitUniformBlocks() {
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &global_mvp_buffer_name = GetGlobalMvpBufferName();
  const std::string &global_mvp_uniform_block_name =
      GetGlobalMvpUniformBlockName();
  const std::string &global_mvp_binding_name = global_mvp_buffer_name;
  // Bind the uniform block to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, global_mvp_uniform_block_name, global_mvp_binding_name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SkyboxShader::GetId() const { return "skybox"; }
