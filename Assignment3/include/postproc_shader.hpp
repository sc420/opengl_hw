#pragma once

#include "as/gl/shader_app.hpp"

namespace shader {
class PostprocShader : public app::ShaderApp {
 public:
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

  std::string GetId() const override;

  void InitUniformBlocks() override;

  void UpdatePostprocInputs(const PostprocInputs &postproc_inputs);

 protected:
  std::string GetPostprocInputsBufferName() const;

  std::string GetPostprocInputsUniformBlockName() const;

 private:
  PostprocInputs postproc_inputs_;
};
}  // namespace shader
