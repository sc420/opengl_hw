#pragma once

#include "as/common.hpp"
#include "as/gl/texture_manager.hpp"

namespace as {
class FramebufferManager {
 public:
  struct BindFramebufferPrevParams {
    GLenum target;
  };

  struct BindRenderbufferPrevParams {
    GLenum target;
  };

  FramebufferManager();

  ~FramebufferManager();

  void RegisterTextureManager(const TextureManager &texture_manager);

  void GenFramebuffer(const std::string &framebuffer_name);

  void GenRenderbuffer(const std::string &renderbuffer_name);

  void BindFramebuffer(const std::string &framebuffer_name,
                       const GLenum framebuffer_target);

  void BindFramebuffer(const std::string &framebuffer_name);

  void BindDefaultFramebuffer(const GLenum framebuffer_target);

  void BindRenderbuffer(const std::string &renderbuffer_name,
                        const GLenum renderbuffer_target);

  void BindRenderbuffer(const std::string &renderbuffer_name);

  void InitRenderbuffer(const std::string &renderbuffer_name,
                        const GLenum renderbuffer_target,
                        const GLenum internal_fmt, const GLsizei width,
                        const GLsizei height);

  void AttachTextureToFramebuffer(const std::string &framebuffer_name,
                                  const std::string &tex_name,
                                  const GLenum framebuffer_target,
                                  const GLenum attachment,
                                  const GLint mipmap_level);

  void AttachTexture2DToFramebuffer(const std::string &framebuffer_name,
                                    const std::string &tex_name,
                                    const GLenum framebuffer_target,
                                    const GLenum attachment,
                                    const GLenum tex_target,
                                    const GLint mipmap_level);

  void AttachRenderbufferToFramebuffer(const std::string &framebuffer_name,
                                       const std::string &renderbuffer_name,
                                       const GLenum framebuffer_target,
                                       const GLenum attachment,
                                       const GLenum renderbuffer_target);

  void DeleteFramebuffer(const std::string &framebuffer_name);

  void DeleteRenderbuffer(const std::string &renderbuffer_name);

  GLuint GetFramebufferHdlr(const std::string &framebuffer_name) const;

  GLuint GetRenderbufferHdlr(const std::string &renderbuffer_name) const;

  bool HasFramebuffer(const std::string &framebuffer_name) const;

  bool HasRenderbuffer(const std::string &renderbuffer_name) const;

 private:
  const TextureManager *texture_manager_;

  std::map<std::string, GLuint> framebuffer_hdlrs_;

  std::map<std::string, GLuint> renderbuffer_hdlrs_;

  std::map<std::string, BindFramebufferPrevParams>
      bind_framebuffer_prev_params_;

  std::map<std::string, BindRenderbufferPrevParams>
      bind_renderbuffer_prev_params_;

  const BindFramebufferPrevParams &GetFramebufferPrevParams(
      const std::string &framebuffer_name) const;

  const BindRenderbufferPrevParams &GetRenderbufferPrevParams(
      const std::string &renderbuffer_name) const;
};
}  // namespace as
