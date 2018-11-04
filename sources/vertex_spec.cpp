#include "assignment/vertex_spec.hpp"

VertexSpecManager::~VertexSpecManager() {
  // Delete all vertex array objects
  for (const auto& pair : hdlrs_) {
    glDeleteVertexArrays(1, &pair.second);
  }
}

void VertexSpecManager::GenVertexArray(const std::string& name) {
  // Generate a vertex array object
  GLuint hdlr;
  glGenVertexArrays(1, &hdlr);
  // Save the vertex array object
  hdlrs_[name] = hdlr;
}

void VertexSpecManager::BindVertexArray(const std::string& name) const {
  const GLuint hdlr = GetVertexArrayHdlr(name);
  glBindVertexArray(hdlr);
}

void VertexSpecManager::SpecifyVertexArrayOrg(const std::string& name,
  const GLuint attrib_idx,
  const GLint size,
  const GLenum type,
  const GLboolean normalized,
  const GLuint relative_ofs) const {
  // Bind the vertex array
  BindVertexArray(name);
  // Enable the generic vertex attribute array
  glEnableVertexAttribArray(attrib_idx);
  // Specify the organization of the vertex array
  glVertexAttribFormat(attrib_idx, size, type, normalized, relative_ofs);
}

void VertexSpecManager::AssocVertexAttribToBindingPoint(
  const std::string& va_name, const GLuint attrib_idx,
  const GLuint binding_idx) {
  // Bind the vertex array
  BindVertexArray(va_name);
  // Associate the vertex attribute to the binding point
  glVertexAttribBinding(attrib_idx, binding_idx);
  // Save the binding point
  binding_points_[va_name][attrib_idx] = binding_idx;
}

/*
 * Note that stride should be greater than 0
 * References:
 * https://www.opengl.org/discussion_boards/showthread.php/182043-glVertexAttribFormat-glDrawArrays-issue
 */
void VertexSpecManager::BindBufferToBindingPoint(const std::string& va_name,
  const GLuint binding_idx,
  const GLuint buffer_hdlr,
  const GLintptr ofs,
  const GLsizei stride) {
  // Bind the vertex array
  BindVertexArray(va_name);
  // Bind the buffer to the binding point
  glBindVertexBuffer(binding_idx, buffer_hdlr, ofs, stride);
  // Save the parameters
  BindBufferToBindingPointPrevParams prev_params = { ofs , stride };
  bind_buffer_to_binding_point_prev_params_[va_name][binding_idx] = prev_params;
}

void VertexSpecManager::BindBufferToBindingPoint(const std::string & va_name, const GLuint binding_idx, const GLuint buffer_hdlr)
{
  const BindBufferToBindingPointPrevParams& prev_params = GetBindBufferToBindingPointPrevParams(va_name, binding_idx);
  BindBufferToBindingPoint(va_name, binding_idx, buffer_hdlr, prev_params.ofs, prev_params.stride);
}

void VertexSpecManager::DeleteVertexArray(const std::string& name) {
  const GLuint hdlr = GetVertexArrayHdlr(name);
  glDeleteVertexArrays(1, &hdlr);
  // Delete previous parameters
  bind_buffer_to_binding_point_prev_params_.erase(name);
}

GLuint VertexSpecManager::GetVertexArrayHdlr(const std::string& name)const {
  if (hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the vertex array name '" + name +
      "'");
  }
  return hdlrs_.at(name);
}

GLuint VertexSpecManager::GetVertexAttribBindingPoint(
  const std::string& va_name, const GLuint attrib_idx) const {
  if (binding_points_.count(va_name) == 0) {
    throw std::runtime_error("Could not find the vertex array name '" +
      va_name + "'");
  }
  const std::map<GLuint, GLuint>& attrib_to_points = binding_points_.at(va_name);
  if (attrib_to_points.count(attrib_idx) == 0) {
    throw std::runtime_error("Could not find the attribute index '" +
      std::to_string(attrib_idx) + "'");
  }
  return attrib_to_points.at(attrib_idx);
}

const VertexSpecManager::BindBufferToBindingPointPrevParams & VertexSpecManager::GetBindBufferToBindingPointPrevParams(const std::string & va_name, const GLuint binding_idx) const
{
  if (bind_buffer_to_binding_point_prev_params_.count(va_name) == 0) {
    throw std::runtime_error("Could not find the previous parameters for vertex array name '" + va_name + "'");
  }
  const std::map<GLuint, BindBufferToBindingPointPrevParams> &binding_idx_to_prev_params = bind_buffer_to_binding_point_prev_params_.at(va_name);
  if (binding_idx_to_prev_params.count(binding_idx) == 0) {
    throw std::runtime_error("Could not find the previous parameters for binding index '" + std::to_string(binding_idx )+ "'");
  }
  return binding_idx_to_prev_params.at(binding_idx);
}
