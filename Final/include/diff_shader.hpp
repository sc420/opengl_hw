#pragma once

#include "shader.hpp"

namespace shader {
class DiffShader : public Shader {
 public:
  enum class DiffTypes { kObj, kNoObj, kBg };

  struct Diff {
    int display_mode;  // 4*0=0, +4->4
  };

  DiffShader();

  /* GL Initializations */

  void Init() override;

  void LoadModel();

  void InitFramebuffers();

  void InitUniformBlocks();

  void InitVertexArrays();

  /* GL Drawing Methods */

  void Draw() override;

  void UseDiffFramebuffer(const DiffTypes diff_type);

  void UpdateObjDiffRenderbuffer(const GLsizei width, const GLsizei height);

  void UpdateDiffFramebufferTextures(const GLsizei width, const GLsizei height);

  /* State Updaters */

  void ToggleDisplayMode();

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Name Management */

  std::string GetDiffFramebufferName(const DiffTypes diff_type) const;

  std::string GetObjDiffDepthRenderbufferName() const;

  std::string GetDiffFramebufferTextureName(const DiffTypes diff_type) const;

  std::string GetDiffFramebufferTextureUnitName(
      const DiffTypes diff_type) const;

  std::string GetQuadVertexArrayGroupName() const;

  std::string GetDiffBufferName() const;

  std::string GetDiffUniformBlockName() const;

 private:
  /* Models */
  as::Model quad_model_;

  /* GL States */
  Diff diff_;

  /* GL Drawing Methods */

  void SetTextureUnitIdxs();

  /* Name Management */

  std::string DiffTypeToName(const DiffTypes diff_type) const;
};
}  // namespace shader
