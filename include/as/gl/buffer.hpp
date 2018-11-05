#pragma once

#include "as/common.hpp"

namespace as {
class BufferManager {
 public:
  struct BindBufferPrevParams {
    GLenum target;
  };

  struct UpdateBufferPrevParams {
    GLenum target;
    GLintptr ofs;
    GLsizeiptr size;
    const GLvoid *data;
  };

  ~BufferManager();

  void GenBuffer(const std::string &buffer_name);

  void BindBuffer(const std::string &buffer_name, const GLenum target);

  void BindBuffer(const std::string &buffer_name);

  void InitBuffer(const std::string &buffer_name, const GLenum target,
                  const GLsizeiptr size, const GLvoid *data,
                  const GLenum usage);

  void UpdateBuffer(const std::string &buffer_name, const GLenum target,
                    const GLintptr ofs, const GLsizeiptr size,
                    const GLvoid *data);

  void UpdateBuffer(const std::string &buffer_name);

  void DeleteBuffer(const std::string &buffer_name);

  GLuint GetBufferHdlr(const std::string &buffer_name) const;

 private:
  std::map<std::string, GLuint> hdlrs_;

  std::map<std::string, BindBufferPrevParams> bind_buffer_prev_params_;

  std::map<std::string, UpdateBufferPrevParams> update_buffer_prev_params_;

  const BindBufferPrevParams &GetBindBufferPrevParams(
      const std::string &buffer_name) const;

  const UpdateBufferPrevParams &GetUpdateBufferPrevParams(
      const std::string &buffer_name) const;
};
}  // namespace as
