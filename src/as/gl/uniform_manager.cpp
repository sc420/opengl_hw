#include "as/gl/uniform_manager.hpp"

as::UniformManager::UniformManager()
    : program_manager_(nullptr), buffer_manager_(nullptr) {}

/*******************************************************************************
 * Initialization
 ******************************************************************************/

void as::UniformManager::Init() { InitLimits(); }

/*******************************************************************************
 * Manager Registrations
 ******************************************************************************/

void as::UniformManager::RegisterProgramManager(
    const ProgramManager &program_manager) {
  program_manager_ = &program_manager;
}

void as::UniformManager::RegisterBufferManager(
    const BufferManager &buffer_manager) {
  buffer_manager_ = &buffer_manager;
}

/*******************************************************************************
 * Setting Value Methods
 ******************************************************************************/

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

/*******************************************************************************
 * Binding Management
 ******************************************************************************/

void as::UniformManager::AssignUniformBlockToBindingPoint(
    const std::string &program_name, const std::string &block_name,
    const GLuint binding_idx) {
  // Check the binding index
  index_manager_.CheckMaxIdx(binding_idx);
  // Get program handler
  const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
  // Get uniform block handler
  const GLuint block_hdlr = GetUniformBlockHdlr(program_name, block_name);
  // Assign the binding point
  glUniformBlockBinding(program_hdlr, block_hdlr, binding_idx);
}

void as::UniformManager::AssignUniformBlockToBindingPoint(
    const std::string &program_name, const std::string &block_name,
    const std::string &binding_name) {
  const auto target = std::make_tuple(program_name, block_name);
  const GLuint binding_idx = index_manager_.BindTarget1(target, binding_name);
  // Assign with the binding index
  AssignUniformBlockToBindingPoint(program_name, block_name, binding_idx);
}

void as::UniformManager::BindBufferBaseToBindingPoint(
    const std::string &buffer_name, const GLuint binding_idx) {
  // Check the binding index
  index_manager_.CheckMaxIdx(binding_idx);
  // Get buffer handler
  const GLuint buffer_hdlr = buffer_manager_->GetBufferHdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferBase(GL_UNIFORM_BUFFER, binding_idx, buffer_hdlr);
}

void as::UniformManager::BindBufferBaseToBindingPoint(
    const std::string &buffer_name, const std::string &binding_name) {
  const auto target = buffer_name;
  const GLuint binding_idx = index_manager_.BindTarget2(target, binding_name);
  // Bind with the binding index
  BindBufferBaseToBindingPoint(buffer_name, binding_idx);
}

void as::UniformManager::UnassignUniformBlock(const std::string &program_name,
                                              const std::string &block_name) {
  const auto target = std::make_tuple(program_name, block_name);
  index_manager_.UnbindTarget1(target);
  // Bind to binding index 0 to avoid programming errors
  AssignUniformBlockToBindingPoint(program_name, block_name, 0);
}

void as::UniformManager::UnbindBufferBase(const std::string &buffer_name) {
  const auto target = buffer_name;
  index_manager_.UnbindTarget2(target);
  // Bind to binding index 0 to avoid programming errors
  BindBufferBaseToBindingPoint(buffer_name, 0);
}

/*******************************************************************************
 * Handler Getters
 ******************************************************************************/

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

/*******************************************************************************
 * Debugging
 ******************************************************************************/

GLint as::UniformManager::GetUniformBlockMemoryOfs(
    const std::string &program_name,
    const std::string &block_member_name) const {
  // Get program handler
  const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
  // Get uniform member index
  GLuint uniform_idx;
  const char *name = block_member_name.c_str();
  glGetUniformIndices(program_hdlr, 1, &name, &uniform_idx);
  // Get uniform member offset
  GLint uniform_ofs;
  glGetActiveUniformsiv(program_hdlr, 1, &uniform_idx, GL_UNIFORM_OFFSET,
                        &uniform_ofs);
  return uniform_ofs;
}

/*******************************************************************************
 * Initialization (Private)
 ******************************************************************************/

void as::UniformManager::InitLimits() {
  GLint value;
  glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
  index_manager_.SetMaxIdx(value);
}
