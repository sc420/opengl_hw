#include "assignment/uniform.hpp"

void UniformManager::RegisterProgramManager(const ProgramManager & program_manager)
{
  program_manager_ = &program_manager;
}

void UniformManager::RegisterBufferManager(const BufferManager & buffer_manager)
{
  buffer_manager_ = &buffer_manager;
}

void UniformManager::AssignUniformBlockToBindingPoint(
  const std::string &program_name, const std::string &block_name,
  const GLuint binding_idx) {
  // Get program handler
  const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
  // Check whether to retrieve the index of a named uniform block lazily
  GLuint block_hdlr = NULL;
  if (block_idxs_.count(program_name) > 0 &&
    block_idxs_.at(program_name).count(block_name) > 0) {
    block_hdlr = block_idxs_.at(program_name).at(block_name);
  }
  else {
    // Retrieve the index of a named uniform block
    block_hdlr = glGetUniformBlockIndex(program_hdlr, block_name.c_str());
    // Save the block handler
    block_idxs_[program_name][block_name] = block_hdlr;
  }
  // Assign the binding point
  glUniformBlockBinding(program_hdlr, block_hdlr, binding_idx);
  // Save the binding point
  binding_points_[program_name][block_name] = binding_idx;
}

void UniformManager::BindBufferBaseToBindingPoint(const std::string &buffer_name, const GLuint binding_idx) {
  // Get buffer handler
  const GLuint buffer_hdlr = buffer_manager_->GetBufferHdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferBase(GL_UNIFORM_BUFFER, binding_idx, buffer_hdlr);
}

void UniformManager::BindBufferRangeToBindingPoint(const std::string &buffer_name,
  const GLuint binding_idx,
  const GLintptr ofs,
  const GLsizeiptr size) {
  // Get buffer handler
  const GLuint buffer_hdlr = buffer_manager_->GetBufferHdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferRange(GL_UNIFORM_BUFFER, binding_idx, buffer_hdlr, ofs, size);
}

GLuint UniformManager::GetUniformBlockBindingPoint(
  const std::string &program_name, const std::string &block_name) {
  if (binding_points_.count(program_name) == 0) {
    throw std::runtime_error("Could not find the program name '" +
      program_name + "'");
  }
  const std::map<std::string, GLuint> &block_to_points =
    binding_points_.at(program_name);
  if (block_to_points.count(block_name) == 0) {
    throw std::runtime_error("Could not find the block name '" + block_name +
      "'");
  }
  return block_to_points.at(block_name);
}
