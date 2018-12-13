#include "skybox_shader.hpp"

std::string shader::SkyboxShader::GetId() const { return "skybox"; }

void shader::SkyboxShader::InitUniformBlocks() {
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Bind the uniform block to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint("skybox", "GlobalMvp",
                                                   "global_mvp");
}
