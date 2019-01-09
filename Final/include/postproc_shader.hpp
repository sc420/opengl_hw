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
      const PostprocFramebufferTypes postproc_framebuffer_type);

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

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Name Management */

  std::string GetPostprocInputsBufferName() const;

  std::string GetPostprocFramebufferName(
      const PostprocFramebufferTypes postproc_framebuffer_type) const;

  std::string GetQuadVertexArrayGroupName() const;

  std::string GetPostprocTextureName(
      const PostprocTextureTypes postproc_tex_type,
      const int scaling_idx) const;

  std::string GetPostprocTextureUnitName(
      const PostprocTextureTypes postproc_tex_type,
      const int scaling_idx) const;

  std::string GetPostprocDepthRenderbufferName(
      const PostprocFramebufferTypes postproc_framebuffer_type) const;

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
