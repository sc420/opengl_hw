#pragma once

#include "shader.hpp"

namespace shader {
class PostprocShader : public Shader {
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
    bool enabled;  // 4*0=0, +1->1

    bool pad_mouse_pos[7];  // +7->8
    glm::vec2 mouse_pos;    // 8*1=8, +8->16
    int effect_idx;         // 4*4=16, +4->20
    int pass_idx;           // 4*5=20, +4->24
    float time;             // 4*6=24, +4->28
  };

  PostprocShader();

  /* GL Initializations */

  void Init() override;

  void LoadModel();

  void InitFramebuffers();

  void InitVertexArrays();

  void InitUniformBlocks();

  /* GL Drawing Methods */

  void UpdateScreenTextures(const GLsizei width, const GLsizei height);

  void UpdateScreenRenderbuffers(const GLsizei width, const GLsizei height);

  void UseScreenFramebuffer(const int screen_idx = 0);

  void Draw() override;

  /* State Updaters */

  void UpdateEnabled(const bool enabled);

  void UpdateMousePos(const glm::ivec2 &mouse_pos);

  void UpdateEffectIdx(const int effect_idx);

  void UpdatePassIdx(const int pass_idx);

  void UpdateTime(const float time);

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Model Handlers */

  virtual as::Model &GetModel() override;

  /* Name Management */

  std::string GetPostprocInputsBufferName() const;

  std::string GetPostprocInputsUniformBlockName() const;

  std::string GetScreenFramebufferName(const int screen_idx) const;

  std::string GetScreenTextureName(const int screen_idx) const;

  std::string GetScreenTextureUnitName(const int screen_idx) const;

  std::string GetScreenDepthRenderbufferName(const int screen_idx) const;

 private:
  static const int kNumFramebuffers;
  static const int kNumMultipass;

  as::Model screen_quad_model_;

  PostprocInputs postproc_inputs_;

  /* GL Drawing Methods */

  void SetTextureUnitIdxs();

  void DrawScreenWithTexture(const int tex_idx);
};
}  // namespace shader
