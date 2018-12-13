#include "postproc_shader.hpp"

std::string shader::PostprocShader::GetId() const { return "postproc"; }

void shader::PostprocShader::InitUniformBlocks() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &buffer_name = GetPostprocInputsBufferName();
  const std::string &uniform_block_name = GetPostprocInputsUniformBlockName();
  const std::string &binding_name = buffer_name;
  // Initialize the buffer
  InitUniformBuffer(buffer_name, postproc_inputs_);
  // Bind the uniform block to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, uniform_block_name, binding_name);
  // Bind the buffer to the uniform block
  uniform_manager.BindBufferBaseToBindingPoint(buffer_name, binding_name);
}

void shader::PostprocShader::UpdatePostprocInputs(
    const PostprocInputs &postproc_inputs) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string &buffer_name = GetPostprocInputsBufferName();
  // Update postproc inputs
  postproc_inputs_ = postproc_inputs;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

std::string shader::PostprocShader::GetPostprocInputsBufferName() const {
  return "postproc_inputs";
}

std::string shader::PostprocShader::GetPostprocInputsUniformBlockName() const {
  return "PostprocInputs";
}
