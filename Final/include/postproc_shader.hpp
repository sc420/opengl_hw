#pragma once

#include "shader.hpp"

namespace shader {
class PostprocShader : public Shader {
 public:
  enum class PostprocFramebufferTypes {
    kDrawOriginal,
    kDrawScaling,
    kBlurScalingHorizontal,
    kBlurScalingVertical,
    kCombining,
  };

  enum class PostprocTextureTypes {
    kOriginal1,
    kHdr1,
    kOriginal2,
    kHdr2,
  };

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
    int scaling_idx;        // 4*6=24, +4->28
    float time;             // 4*7=28, +4->32

    float effect_amount;      // 4*8=32, +4->36
    bool use_shaking_effect;  // 4*9=36, +1->37

    bool pad_use_blur_effect[3];  // +3->40
    bool use_blurring_effect;     // 4*10=40, +1->41

    bool pad_use_gamma_correct[3];  // +3->44
    bool use_gamma_correct;         // 4*11=44, +1->45

    bool pad[3];  // +3->48
  };

  PostprocShader();

  /* GL Initializations */

  void Init();

  void LoadModel();

  void InitFramebuffers();

  void InitVertexArrays();

  void InitUniformBlocks();

  /* GL Drawing Methods */

  void UpdatePostprocTextures(const GLsizei width, const GLsizei height);

  void UpdatePostprocRenderbuffer(const GLsizei width, const GLsizei height);

  void UsePostprocFramebuffer(
      const PostprocFramebufferTypes postproc_framebuffer_type,
      const int scaling_idx);

  void UseDefaultPostprocTextures();

  void UsePostprocTexture(
      const PostprocFramebufferTypes postproc_framebuffer_type,
      const PostprocTextureTypes postproc_tex_type, const int scaling_idx);

  void DrawBloom(const glm::ivec2 &window_size);

  void DrawPostprocEffects();

  /* State Updaters */

  void UpdateEnabled(const bool enabled);

  void UpdateMousePos(const glm::ivec2 &mouse_pos);

  void UpdateEffectIdx(const int effect_idx);

  void UpdatePassIdx(const int pass_idx);

  void UpdateScalingIdx(const int scaling_idx);

  void UpdateTime(const float time);

  void UpdateEffectAmount(const float amount);

  void UpdateUseShakingEffect(const bool use_shaking_effect);

  void UpdateUseBlurringEffect(const bool use_blurring_effect);

  void UpdateUseGammaCorrect(const bool use_gamma_correct);

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Name Management */

  std::string GetPostprocInputsBufferName() const;

  std::string GetPostprocFramebufferName(
      const PostprocFramebufferTypes postproc_framebuffer_type,
      const int scaling_idx) const;

  std::string GetQuadVertexArrayGroupName() const;

  std::string GetPostprocTextureName(
      const PostprocTextureTypes postproc_tex_type,
      const int scaling_idx) const;

  std::string GetPostprocTextureUnitName(
      const PostprocTextureTypes postproc_tex_type,
      const int scaling_idx) const;

  std::string GetPostprocDepthRenderbufferName(
      const PostprocFramebufferTypes postproc_framebuffer_type,
      const int scaling_idx) const;

  std::string GetPostprocInputsUniformBlockName() const;

 private:
  /* Constants */
  static const GLsizei kNumMipmapLevels;
  static const int kNumBloomScaling;

  /* Models */
  as::Model quad_model_;

  /* States */
  PostprocInputs postproc_inputs_;

  /* State Getters */

  PostprocTextureTypes GetPassOriginalTextureType(const int pass_idx,
                                                  const bool read) const;

  PostprocTextureTypes GetPassHdrTextureType(const int pass_idx,
                                             const bool read) const;

  /* State Updaters */

  void UpdatePostprocInputs();

  /* GL Drawing Methods */

  void SetTextureUnitIdxs(const int pass_idx, const int scaling_idx);

  void SetScalingTextureUnitIdxs(const int pass_idx);

  void DrawToTextures();

  /* Type Conversions */

  std::string PostprocFramebufferTypeToName(
      const PostprocFramebufferTypes postproc_framebuffer_type) const;

  int PostprocFramebufferTypeToNum(
      const PostprocFramebufferTypes postproc_framebuffer_type) const;

  int GetPostprocTextureTypeNumBloomScaling(
      const PostprocTextureTypes postproc_tex_type) const;

  std::string PostprocTextureTypeToName(
      const PostprocTextureTypes postproc_tex_type) const;

  int PostprocTextureTypeToNum(
      const PostprocTextureTypes postproc_tex_type) const;
};
}  // namespace shader
