#pragma once

#include "shader.hpp"

namespace shader {
class DiffShader : public Shader {
 public:
  enum class DiffTypes { kObj, kNoObj, kBg };

  /* GL Initializations */

  void Init() override;

  void InitFramebuffers();

  /* GL Drawing Methods */

  void UseDiffFramebuffer(const DiffTypes diff_type);

  void UpdateObjDiffRenderbuffers(const GLsizei width, const GLsizei height);

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Name Management */

  std::string GetDiffFramebufferName(const DiffTypes diff_type) const;

  std::string GetObjDiffDepthRenderbufferName() const;
};
}  // namespace shader
