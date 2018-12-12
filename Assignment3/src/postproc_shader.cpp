#include "postproc_shader.hpp"

std::string shader::PostprocShader::GetId() const { return "postproc"; }

void shader::PostprocShader::InitUniformBlocks() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Initialize buffers
  const std::string &buffer_name = GetPostprocInputsBufferName();
  buffer_manager.GenBuffer(buffer_name);
  buffer_manager.InitBuffer(buffer_name, GL_UNIFORM_BUFFER,
                            sizeof(PostprocInputs), NULL, GL_STATIC_DRAW);
  buffer_manager.UpdateBuffer(buffer_name, GL_UNIFORM_BUFFER, 0,
                              sizeof(PostprocInputs), &postproc_inputs_);
}

void shader::PostprocShader::UpdatePostprocInputs(
    const PostprocInputs &postproc_inputs) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update postproc inputs
  postproc_inputs_ = postproc_inputs;
  // Update the buffer
  const std::string &buffer_name = GetPostprocInputsBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

std::string shader::PostprocShader::GetPostprocInputsBufferName() const {
  return "postproc_inputs";
}
