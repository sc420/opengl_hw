#pragma once

#include "shader.hpp"

namespace shader {
class PostprocShader : public Shader {
 public:
  enum class PostprocTextureTypes {
    kOriginal,
    kHdr,
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
    float time;             // 4*6=24, +4->28
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

  void UpdatePostprocRenderbuffers(const GLsizei width, const GLsizei height);

  void UsePostprocFramebuffer();

  void Draw(const PostprocTextureTypes postproc_tex_type =
                PostprocTextureTypes::kOriginal);

  /* State Updaters */

  void UpdateEnabled(const bool enabled);

  void UpdateMousePos(const glm::ivec2 &mouse_pos);

  void UpdateEffectIdx(const int effect_idx);

  void UpdatePassIdx(const int pass_idx);

  void UpdateTime(const float time);

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Name Management */

  std::string GetPostprocInputsBufferName() const;

  std::string GetScreenFramebufferName() const;

  std::string GetQuadVertexArrayGroupName() const;

  std::string GetScreenTextureName(
      const PostprocTextureTypes postproc_tex_type) const;

  std::string GetScreenTextureUnitName(
      const PostprocTextureTypes postproc_tex_type) const;

  std::string GetScreenDepthRenderbufferName(
      const PostprocTextureTypes postproc_tex_type) const;

  std::string GetPostprocInputsUniformBlockName() const;

  /* Type Conversions */

  std::string PostprocTextureTypeToName(
      const PostprocTextureTypes postproc_tex_type) const;

  int PostprocTextureTypeToNum(
      const PostprocTextureTypes postproc_tex_type) const;

 private:
  /* Models */
  as::Model quad_model_;

  /* States */
  PostprocInputs postproc_inputs_;

  /* GL Drawing Methods */

  void SetTextureUnitIdxs();

  void DrawToTexture(const PostprocTextureTypes postproc_tex_type);
};
}  // namespace shader
