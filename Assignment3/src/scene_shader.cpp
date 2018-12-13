#include "scene_shader.hpp"

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void shader::SceneShader::Init() {
  app::ShaderApp::Init();
  InitUniformBlocks();
}

void shader::SceneShader::InitUniformBlocks() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &global_mvp_buffer_name = GetGlobalMvpBufferName();
  const std::string &model_trans_buffer_name = GetModelTransBufferName();
  const std::string &global_mvp_uniform_block_name =
      GetGlobalMvpUniformBlockName();
  const std::string &model_trans_uniform_block_name =
      GetModelTransUniformBlockName();
  const std::string &global_mvp_binding_name = global_mvp_buffer_name;
  const std::string &model_trans_binding_name = model_trans_buffer_name;
  // Initialize the buffers
  InitUniformBuffer(global_mvp_buffer_name, global_mvp_);
  InitUniformBuffer(model_trans_buffer_name, model_trans_);
  // Bind the uniform blocks to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, global_mvp_uniform_block_name, global_mvp_binding_name);
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, model_trans_uniform_block_name, model_trans_binding_name);
  // Bind the buffers to the uniform blocks
  uniform_manager.BindBufferBaseToBindingPoint(global_mvp_buffer_name,
                                               global_mvp_binding_name);
  uniform_manager.BindBufferBaseToBindingPoint(model_trans_buffer_name,
                                               model_trans_binding_name);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::SceneShader::Draw() {}

/*******************************************************************************
 * State Updating Methods
 ******************************************************************************/

void shader::SceneShader::UpdateGlobalMvp(const GlobalMvp &global_mvp) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string &buffer_name = GetGlobalMvpBufferName();
  // Update global MVP
  global_mvp_ = global_mvp;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateModelTrans(const ModelTrans &model_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model trans
  model_trans_ = model_trans;
  // Update the buffer
  const std::string &buffer_name = GetModelTransBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SceneShader::GetId() const { return "scene"; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::SceneShader::GetGlobalMvpBufferName() const {
  return "global_mvp";
}

std::string shader::SceneShader::GetModelTransBufferName() const {
  return "model_trans";
}

std::string shader::SceneShader::GetGlobalMvpUniformBlockName() const {
  return "GlobalMvp";
}

std::string shader::SceneShader::GetModelTransUniformBlockName() const {
  return "ModelTrans";
}
