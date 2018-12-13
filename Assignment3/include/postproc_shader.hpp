#pragma once

#include "as/gl/shader_app.hpp"

namespace shader {
class PostprocShader : public app::ShaderApp {
 public:
  enum Effects {
    kEffectImgAbs,
    kEffectLaplacian,
    kEffectSharpness,
    kEffectPixelation,
    kEffectBloomEffect,
    kEffectMagnifier,
    kEffectSpecial,
  };

  struct PostprocInputs {
    // Use int[2] instead of bool to avoid alignment mismatch problem (OpenGL
    // will pad the memory for alignment, but C++ sizeof() won't)
    int enabled[2];
    glm::vec2 mouse_pos;
    glm::vec2 window_size;
    int effect_idx[2];
    int pass_idx[2];
    int time[2];
  };

  /* GL initialization methods */

  void Init() override;

  void LoadModel();

  void InitFramebuffers();

  void InitVertexArrays();

  void InitUniformBlocks();

  /* GL drawing methods */

  void Draw() override;

  void UseDefaultFramebuffer();

  void UseScreenFramebuffer(const int screen_idx);

  /* State updating methods */

  void UpdateEnabled(const bool enabled);

  void UpdateMousePos(const glm::ivec2 &mouse_pos);

  void UpdateWindowSize(const glm::ivec2 &window_size);

  void UpdateEffectIdx(const int effect_idx);

  void UpdatePassIdx(const int pass_idx);

  void UpdateTime(const int time);

  /* Name management */

  std::string GetId() const override;

 protected:
  std::string GetPostprocInputsBufferName() const;

  std::string GetPostprocInputsUniformBlockName() const;

  std::string GetScreenFramebufferName(const int screen_idx) const;

 private:
  as::Model screen_quad_model_;

  PostprocInputs postproc_inputs_;

  /* GL drawing methods */

  void DrawScreenWithTexture(const int tex_idx);
};
}  // namespace shader
