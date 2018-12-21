#pragma once

#include "as/common.hpp"
#include "as/gl/buffer_manager.hpp"

namespace as {
class VertexSpecManager {
 public:
  struct BindBufferToBindingPointPrevParams {
    GLintptr ofs;
    GLsizei stride;
  };

  VertexSpecManager();

  ~VertexSpecManager();

  /* Manager Registrations */

  void RegisterBufferManager(const BufferManager &buffer_manager);

  /* Generations */

  void GenVertexArray(const std::string &va_name);

  /* Bindings */

  void BindVertexArray(const std::string &va_name) const;

  /* Deselections */

  void DeselectVertexArray() const;

  /* Memory Specifications */

  void SpecifyVertexArrayOrg(const std::string &va_name,
                             const GLuint attrib_idx, const GLint size,
                             const GLenum type, const GLboolean normalized,
                             const GLuint relative_ofs) const;

  /* Binding Connections */

  void AssocVertexAttribToBindingPoint(const std::string &va_name,
                                       const GLuint attrib_idx,
                                       const GLuint binding_idx);

  void BindBufferToBindingPoint(const std::string &va_name,
                                const std::string &buffer_name,
                                const GLuint binding_idx, const GLintptr ofs,
                                const GLsizei stride);

  void BindBufferToBindingPoint(const std::string &va_name,
                                const std::string &buffer_name,
                                const GLuint binding_idx);

  /* Deletions */

  void DeleteVertexArray(const std::string &va_name);

  /* Handler Getters */

  GLuint GetVertexArrayHdlr(const std::string &va_name) const;

  /* Binding Point Getters */

  GLuint GetVertexAttribBindingPoint(const std::string &va_name,
                                     const GLuint attrib_idx) const;

 private:
  const BufferManager *buffer_manager_;

  std::map<std::string, GLuint> hdlrs_;

  std::map<std::string, std::map<GLuint, GLuint>> binding_points_;

  std::map<std::string, std::map<GLuint, BindBufferToBindingPointPrevParams>>
      bind_buffer_to_binding_point_prev_params_;

  /* Previous Parameter Getters */

  const BindBufferToBindingPointPrevParams &
  GetBindBufferToBindingPointPrevParams(const std::string &va_name,
                                        const GLuint binding_idx) const;
};
}  // namespace as
