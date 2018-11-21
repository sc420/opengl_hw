#include "as/gl/uniform.hpp"

as::UniformManager::UniformManager()
    : program_manager_(nullptr), buffer_manager_(nullptr) {}

void as::UniformManager::RegisterProgramManager(
    const ProgramManager &program_manager) {
  program_manager_ = &program_manager;
}

void as::UniformManager::RegisterBufferManager(
    const BufferManager &buffer_manager) {
  buffer_manager_ = &buffer_manager;
}

void as::UniformManager::SetUniform1Float(const std::string &program_name,
                                          const std::string &var_name,
                                          const GLfloat v0) {
  const GLint var_hdlr = GetUniformVarHdlr(program_name, var_name);
  glUniform1f(var_hdlr, v0);
}

void as::UniformManager::SetUniform1Int(const std::string &program_name,
                                        const std::string &var_name,
                                        const GLint v0) {
  const GLint var_hdlr = GetUniformVarHdlr(program_name, var_name);
  glUniform1i(var_hdlr, v0);
}

void as::UniformManager::AssignUniformBlockToBindingPoint(
    const std::string &program_name, const std::string &block_name,
    const GLuint binding_idx) {
  // Get program handler
  const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
  // Get uniform block handler
  const GLuint block_hdlr = GetUniformBlockHdlr(program_name, block_name);
  // Assign the binding point
  glUniformBlockBinding(program_hdlr, block_hdlr, binding_idx);
  // Save the binding point
  binding_points_[program_name][block_name] = binding_idx;
}

void as::UniformManager::BindBufferBaseToBindingPoint(
    const std::string &buffer_name, const GLuint binding_idx) const {
  // Get buffer handler
  const GLuint buffer_hdlr = buffer_manager_->GetBufferHdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferBase(GL_UNIFORM_BUFFER, binding_idx, buffer_hdlr);
}

void as::UniformManager::BindBufferRangeToBindingPoint(
    const std::string &buffer_name, const GLuint binding_idx,
    const GLintptr ofs, const GLsizeiptr size) const {
  // Get buffer handler
  const GLuint buffer_hdlr = buffer_manager_->GetBufferHdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferRange(GL_UNIFORM_BUFFER, binding_idx, buffer_hdlr, ofs, size);
}

GLuint as::UniformManager::GetUniformBlockBindingPoint(
    const std::string &program_name, const std::string &block_name) const {
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

GLint as::UniformManager::GetUniformVarHdlr(const std::string &program_name,
                                            const std::string &block_name) {
  GLint var_hdlr;
  // Check whether to retrieve the index of a named uniform variable lazily
  if (var_hdlrs_.count(program_name) > 0 &&
      var_hdlrs_.at(program_name).count(block_name) > 0) {
    var_hdlr = var_hdlrs_.at(program_name).at(block_name);
  } else {
    const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
    // Retrieve the index of a named uniform variable
    var_hdlr = glGetUniformLocation(program_hdlr, block_name.c_str());
    // Save the variable handler
    var_hdlrs_[program_name][block_name] = var_hdlr;
  }
  return var_hdlr;
}

GLuint as::UniformManager::GetUniformBlockHdlr(const std::string &program_name,
                                               const std::string &block_name) {
  GLuint block_hdlr;
  // Check whether to retrieve the index of a named uniform block lazily
  if (block_hdlrs_.count(program_name) > 0 &&
      block_hdlrs_.at(program_name).count(block_name) > 0) {
    block_hdlr = block_hdlrs_.at(program_name).at(block_name);
  } else {
    const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
    // Retrieve the index of a named uniform block
    block_hdlr = glGetUniformBlockIndex(program_hdlr, block_name.c_str());
    // Save the block handler
    block_hdlrs_[program_name][block_name] = block_hdlr;
  }
  return block_hdlr;
}
