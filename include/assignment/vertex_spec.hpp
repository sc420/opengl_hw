#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"
#include "assignment/buffer.hpp"

class VertexSpecManager {
 public:
   struct BindBufferToBindingPointPrevParams {
     GLintptr ofs;
     GLsizei stride;
   };

   VertexSpecManager();

  ~VertexSpecManager();

  void RegisterBufferManager(const BufferManager &buffer_manager);

  void GenVertexArray(const std::string &va_name);

  void BindVertexArray(const std::string &va_name) const;

  void SpecifyVertexArrayOrg(const std::string &va_name, const GLuint attrib_idx, const GLint size,
                             const GLenum type, const GLboolean normalized,
                             const GLuint relative_ofs) const;

  void AssocVertexAttribToBindingPoint(const std::string &va_name,
                                       const GLuint attrib_idx,
                                       const GLuint binding_idx);

  void BindBufferToBindingPoint(const std::string &va_name,
                                const std::string &buffer_name,
                                const GLuint binding_idx,
                                const GLintptr ofs,
                                const GLsizei stride);

  void BindBufferToBindingPoint(const std::string &va_name,
    const std::string &buffer_name,
    const GLuint binding_idx);

  void DeleteVertexArray(const std::string &va_name);

  GLuint GetVertexArrayHdlr(const std::string &va_name) const;

  GLuint GetVertexAttribBindingPoint(const std::string &va_name,
                                     const GLuint attrib_idx) const;

 private:
   const BufferManager * buffer_manager_;

  std::map<std::string, GLuint> hdlrs_;

  std::map<std::string, std::map<GLuint, GLuint>> binding_points_;

  std::map<std::string, std::map<GLuint, BindBufferToBindingPointPrevParams>> bind_buffer_to_binding_point_prev_params_;

  const BindBufferToBindingPointPrevParams &GetBindBufferToBindingPointPrevParams(const std::string &va_name, const GLuint binding_idx) const;
};
