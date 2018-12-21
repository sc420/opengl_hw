#include "as/gl/framebuffer_manager.hpp"

as::FramebufferManager::FramebufferManager() : texture_manager_(nullptr) {}

as::FramebufferManager::~FramebufferManager() {
  // Delete all framebuffers
  for (const auto& pair : framebuffer_hdlrs_) {
    glDeleteFramebuffers(1, &pair.second);
  }
  // Delete all renderbuffers
  for (const auto& pair : renderbuffer_hdlrs_) {
    glDeleteRenderbuffers(1, &pair.second);
  }
}

/*******************************************************************************
 * Manager Registrations
 ******************************************************************************/

void as::FramebufferManager::RegisterTextureManager(
    const TextureManager& texture_manager) {
  texture_manager_ = &texture_manager;
}

/*******************************************************************************
 * Generations
 ******************************************************************************/

void as::FramebufferManager::GenFramebuffer(
    const std::string& framebuffer_name) {
  GLuint hdlr;
  glGenFramebuffers(1, &hdlr);
  framebuffer_hdlrs_[framebuffer_name] = hdlr;
}

void as::FramebufferManager::GenRenderbuffer(
    const std::string& renderbuffer_name) {
  GLuint hdlr;
  glGenRenderbuffers(1, &hdlr);
  renderbuffer_hdlrs_[renderbuffer_name] = hdlr;
}

/*******************************************************************************
 * Bindings
 ******************************************************************************/

void as::FramebufferManager::BindFramebuffer(
    const std::string& framebuffer_name, const GLenum framebuffer_target) {
  const GLuint hdlr = GetFramebufferHdlr(framebuffer_name);
  glBindFramebuffer(framebuffer_target, hdlr);
  // Save the parameters
  BindFramebufferPrevParams prev_params = {framebuffer_target};
  bind_framebuffer_prev_params_[framebuffer_name] = prev_params;
}

void as::FramebufferManager::BindFramebuffer(
    const std::string& framebuffer_name) {
  const BindFramebufferPrevParams& prev_params =
      GetFramebufferPrevParams(framebuffer_name);
  BindFramebuffer(framebuffer_name, prev_params.target);
}

void as::FramebufferManager::BindDefaultFramebuffer(
    const GLenum framebuffer_target) {
  glBindFramebuffer(framebuffer_target, 0);
}

void as::FramebufferManager::BindRenderbuffer(
    const std::string& renderbuffer_name, const GLenum renderbuffer_target) {
  const GLuint hdlr = GetRenderbufferHdlr(renderbuffer_name);
  glBindRenderbuffer(renderbuffer_target, hdlr);
  // Save the parameters
  BindRenderbufferPrevParams prev_params = {renderbuffer_target};
  bind_renderbuffer_prev_params_[renderbuffer_name] = prev_params;
}

void as::FramebufferManager::BindRenderbuffer(
    const std::string& renderbuffer_name) {
  const BindRenderbufferPrevParams& prev_params =
      GetRenderbufferPrevParams(renderbuffer_name);
  BindRenderbuffer(renderbuffer_name, prev_params.target);
}

/*******************************************************************************
 * Memory Initialization
 ******************************************************************************/

void as::FramebufferManager::InitRenderbuffer(
    const std::string& renderbuffer_name, const GLenum renderbuffer_target,
    const GLenum internal_fmt, const GLsizei width, const GLsizei height) {
  BindRenderbuffer(renderbuffer_name, renderbuffer_target);
  glRenderbufferStorage(renderbuffer_target, internal_fmt, width, height);
}

/*******************************************************************************
 * Binding Connections
 ******************************************************************************/

void as::FramebufferManager::AttachTextureToFramebuffer(
    const std::string& framebuffer_name, const std::string& tex_name,
    const GLenum framebuffer_target, const GLenum attachment,
    const GLint mipmap_level) {
  BindFramebuffer(framebuffer_name, framebuffer_target);
  const GLuint tex_hdlr = texture_manager_->GetTextureHdlr(tex_name);
  glFramebufferTexture(framebuffer_target, attachment, tex_hdlr, mipmap_level);
}

void as::FramebufferManager::AttachTexture2DToFramebuffer(
    const std::string& framebuffer_name, const std::string& tex_name,
    const GLenum framebuffer_target, const GLenum attachment,
    const GLenum tex_target, const GLint mipmap_level) {
  BindFramebuffer(framebuffer_name, framebuffer_target);
  const GLuint tex_hdlr = texture_manager_->GetTextureHdlr(tex_name);
  glFramebufferTexture2D(framebuffer_target, attachment, tex_target, tex_hdlr,
                         mipmap_level);
}

void as::FramebufferManager::AttachRenderbufferToFramebuffer(
    const std::string& framebuffer_name, const std::string& renderbuffer_name,
    const GLenum framebuffer_target, const GLenum attachment,
    const GLenum renderbuffer_target) {
  BindFramebuffer(framebuffer_name, framebuffer_target);
  const GLuint renderbuffer_hdlr = GetRenderbufferHdlr(renderbuffer_name);
  glFramebufferRenderbuffer(framebuffer_target, attachment, renderbuffer_target,
                            renderbuffer_hdlr);
}

/*******************************************************************************
 * Deletions
 ******************************************************************************/

void as::FramebufferManager::DeleteFramebuffer(
    const std::string& framebuffer_name) {
  const GLuint hdlr = GetFramebufferHdlr(framebuffer_name);
  glDeleteFramebuffers(1, &hdlr);
  // Delete previous parameters
  bind_framebuffer_prev_params_.erase(framebuffer_name);
}

void as::FramebufferManager::DeleteRenderbuffer(
    const std::string& renderbuffer_name) {
  const GLuint hdlr = GetRenderbufferHdlr(renderbuffer_name);
  glDeleteRenderbuffers(1, &hdlr);
  // Delete previous parameters
  bind_renderbuffer_prev_params_.erase(renderbuffer_name);
}

/*******************************************************************************
 * Handler Getters
 ******************************************************************************/

GLuint as::FramebufferManager::GetFramebufferHdlr(
    const std::string& framebuffer_name) const {
  if (!HasFramebuffer(framebuffer_name)) {
    throw std::runtime_error("Could not find the framebuffer name '" +
                             framebuffer_name + "'");
  }
  return framebuffer_hdlrs_.at(framebuffer_name);
}

GLuint as::FramebufferManager::GetRenderbufferHdlr(
    const std::string& renderbuffer_name) const {
  if (!HasRenderbuffer(renderbuffer_name)) {
    throw std::runtime_error("Could not find the renderbuffer name '" +
                             renderbuffer_name + "'");
  }
  return renderbuffer_hdlrs_.at(renderbuffer_name);
}

/*******************************************************************************
 * Status Checkings
 ******************************************************************************/

bool as::FramebufferManager::HasFramebuffer(
    const std::string& framebuffer_name) const {
  return framebuffer_hdlrs_.count(framebuffer_name) > 0;
}

bool as::FramebufferManager::HasRenderbuffer(
    const std::string& renderbuffer_name) const {
  return renderbuffer_hdlrs_.count(renderbuffer_name) > 0;
}

/*******************************************************************************
 * Previous Parameter Getters (Private)
 ******************************************************************************/

const as::FramebufferManager::BindFramebufferPrevParams&
as::FramebufferManager::GetFramebufferPrevParams(
    const std::string& framebuffer_name) const {
  if (bind_framebuffer_prev_params_.count(framebuffer_name) == 0) {
    throw std::runtime_error(
        "Could not find the previous parameters for framebuffer name '" +
        framebuffer_name + "'");
  }
  return bind_framebuffer_prev_params_.at(framebuffer_name);
}

const as::FramebufferManager::BindRenderbufferPrevParams&
as::FramebufferManager::GetRenderbufferPrevParams(
    const std::string& renderbuffer_name) const {
  if (bind_renderbuffer_prev_params_.count(renderbuffer_name) == 0) {
    throw std::runtime_error(
        "Could not find the previous parameters for renderbuffer name '" +
        renderbuffer_name + "'");
  }
  return bind_renderbuffer_prev_params_.at(renderbuffer_name);
}
