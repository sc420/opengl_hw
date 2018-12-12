#include "as/gl/uniform_manager.hpp"

as::UniformManager::UniformManager()
    : program_manager_(nullptr), buffer_manager_(nullptr) {}

void as::UniformManager::Init() {
  GetLimits();
  InitUsedBindingIdxs();
}

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
  // Check the binding index
  CheckBindingIdx(binding_idx);
  // Get program handler
  const GLuint program_hdlr = program_manager_->GetProgramHdlr(program_name);
  // Get uniform block handler
  const GLuint block_hdlr = GetUniformBlockHdlr(program_name, block_name);
  // Assign the binding point
  glUniformBlockBinding(program_hdlr, block_hdlr, binding_idx);
  // Indicate the program and block names are using the binding index
  MarkBindingIdxUsedByUniformSide(program_name, block_name, binding_idx);
}

void as::UniformManager::AssignUniformBlockToBindingPoint(
    const std::string &program_name, const std::string &block_name,
    const std::string &binding_name) {
  GLuint binding_idx;
  // Check whether the binding name has an associated binding index
  if (binding_name_to_idxs_.count(binding_name) > 0) {
    binding_idx = binding_name_to_idxs_.at(binding_name);
  } else {
    binding_idx = GetUnusedBindingIdx();
    binding_name_to_idxs_[binding_name] = binding_idx;
  }
  // Assign with the binding index
  AssignUniformBlockToBindingPoint(program_name, block_name, binding_idx);
}

void as::UniformManager::BindBufferBaseToBindingPoint(
    const std::string &buffer_name, const GLuint binding_idx) {
  // Check the binding index
  CheckBindingIdx(binding_idx);
  // Get buffer handler
  const GLuint buffer_hdlr = buffer_manager_->GetBufferHdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferBase(GL_UNIFORM_BUFFER, binding_idx, buffer_hdlr);
  // Indicate the buffer name is using the binding index
  MarkBindingIdxUsedByBufferSide(buffer_name, binding_idx);
}

void as::UniformManager::BindBufferBaseToBindingPoint(
    const std::string &buffer_name, const std::string &binding_name) {
  GLuint binding_idx;
  // Check whether the binding name has an associated binding index
  if (binding_name_to_idxs_.count(binding_name) > 0) {
    binding_idx = binding_name_to_idxs_.at(binding_name);
  } else {
    binding_idx = GetUnusedBindingIdx();
    binding_name_to_idxs_[binding_name] = binding_idx;
  }
  // Bind with the binding index
  BindBufferBaseToBindingPoint(buffer_name, binding_idx);
}

void as::UniformManager::UnassignUniformBlock(const std::string &program_name,
                                              const std::string &block_name) {
  // Indicate the program and block names are not using the binding index
  UnmarkBindingIdxUsedByUniformSide(program_name, block_name);
  // Bind to binding index 0 to avoid programming errors
  AssignUniformBlockToBindingPoint(program_name, block_name, 0);
}

void as::UniformManager::UnbindBufferBase(const std::string &buffer_name) {
  // Indicate the buffer name is not using the binding index
  UnmarkBindingIdxUsedByBufferSide(buffer_name);
  // Bind to binding index 0 to avoid programming errors
  BindBufferBaseToBindingPoint(buffer_name, 0);
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

void as::UniformManager::GetLimits() {
  GLint value;
  glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
  max_uniform_buffer_bindings_ = value;
}

void as::UniformManager::InitUsedBindingIdxs() {
  // Reserve the binding index 0 as a default index to avoid programming errors
  used_binding_idxs_.insert(0);
}

void as::UniformManager::CheckBindingIdx(const GLuint binding_idx) const {
  if (binding_idx >= max_uniform_buffer_bindings_) {
    throw std::runtime_error(
        "Binding index '" + std::to_string(max_uniform_buffer_bindings_) +
        "' exceeds the limit '" + std::to_string(max_uniform_buffer_bindings_) +
        "'");
  }
}

GLuint as::UniformManager::GetUnusedBindingIdx() {
  const auto last_it = used_binding_idxs_.rbegin();
  const GLuint new_binding_idx = *last_it + 1;
  if (new_binding_idx >= max_uniform_buffer_bindings_) {
    throw std::runtime_error("All binding indexes have been used");
  }
  used_binding_idxs_.insert(new_binding_idx);
  return new_binding_idx;
}

void as::UniformManager::MarkBindingIdxUsedByUniformSide(
    const std::string &program_name, const std::string &block_name,
    const GLuint binding_idx) {
  if (binding_idx == 0) {
    return;
  }
  const auto name_key = std::make_tuple(program_name, block_name);
  // Save the program and block names to the binding index link
  uniform_side_to_binding_idx_[name_key] = binding_idx;
  // Add the program and block names to the used set
  if (binding_idx_to_used_uniform_sides_.count(binding_idx) > 0) {
    auto &used_uniform_sides =
        binding_idx_to_used_uniform_sides_.at(binding_idx);
    used_uniform_sides.insert(std::make_tuple(program_name, block_name));
  } else {
    std::set<std::tuple<std::string, std::string>> used_uniform_sides;
    used_uniform_sides.insert(std::make_tuple(program_name, block_name));
    binding_idx_to_used_uniform_sides_[binding_idx] = used_uniform_sides;
  }
}

void as::UniformManager::MarkBindingIdxUsedByBufferSide(
    const std::string &buffer_name, const GLuint binding_idx) {
  if (binding_idx == 0) {
    return;
  }
  // Save the buffer name to the binding index link
  buffer_side_to_binding_idx_[buffer_name] = binding_idx;
  // Add the buffer name to the used set
  if (binding_idx_to_used_buffer_sides_.count(binding_idx) > 0) {
    auto &used_buffer_sides = binding_idx_to_used_buffer_sides_.at(binding_idx);
    used_buffer_sides.insert(buffer_name);
  } else {
    std::set<std::string> used_buffer_sides;
    used_buffer_sides.insert(buffer_name);
    binding_idx_to_used_buffer_sides_[binding_idx] = used_buffer_sides;
  }
}

void as::UniformManager::UnmarkBindingIdxUsedByUniformSide(
    const std::string &program_name, const std::string &block_name) {
  const auto name_key = std::make_tuple(program_name, block_name);
  // Get the binding index
  if (uniform_side_to_binding_idx_.count(name_key) <= 0) {
    return;
  }
  const GLuint binding_idx = uniform_side_to_binding_idx_.at(name_key);
  // Remove the names from the used set
  auto &used_uniform_sides = binding_idx_to_used_uniform_sides_.at(binding_idx);
  used_uniform_sides.erase(name_key);
  // Check whether the binding index is not used by any names
  if (used_uniform_sides.empty()) {
    used_binding_idxs_.erase(binding_idx);
  }
}

void as::UniformManager::UnmarkBindingIdxUsedByBufferSide(
    const std::string &buffer_name) {
  // Get the binding index
  if (buffer_side_to_binding_idx_.count(buffer_name) <= 0) {
    return;
  }
  const GLuint binding_idx = buffer_side_to_binding_idx_.at(buffer_name);
  // Remove the name from the used set
  auto &used_buffer_sides = binding_idx_to_used_buffer_sides_.at(binding_idx);
  used_buffer_sides.erase(buffer_name);
  // Check whether the binding index is not used by any name
  if (used_buffer_sides.empty()) {
    used_binding_idxs_.erase(binding_idx);
  }
}
