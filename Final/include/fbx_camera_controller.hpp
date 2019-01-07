#pragma once

#include "as/common.hpp"

namespace ctrl {
class FbxCameraController {
 public:
  FbxCameraController(
      const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scaling,
      const glm::vec3 &pos_adjust_factor, const glm::vec3 &rot_adjust_factor,
      const glm::vec3 &scaling_adjust_factor, const glm::vec3 &max_pos_change,
      const glm::vec3 &max_rot_change, const glm::vec3 &max_scaling_change,
      const glm::vec3 &pos_change_decay, const glm::vec3 &rot_change_decay,
      const glm::vec3 &scaling_change_decay, const glm::vec3 &pos_bounce_force,
      const glm::vec3 &rot_bounce_force, const glm::vec3 &scaling_bounce_force);

  glm::vec3 GetPos() const;

  glm::vec3 GetRot() const;

  glm::vec3 GetScaling() const;

  glm::vec3 GetRotDir() const;

  glm::vec3 GetRotUp() const;

  void SetPreferPos(const glm::vec3 &prefer_pos);

  void SetPreferRot(const glm::vec3 &prefer_rot);

  void SetPreferScaling(const glm::vec3 &prefer_scaling);

  void AddPos(const glm::vec3 &add_pos);

  void AddRot(const glm::vec3 &add_rot);

  void AddScaling(const glm::vec3 &add_scaling);

  void Update();

 private:
  glm::vec3 pos_;
  glm::vec3 rot_;
  glm::vec3 scaling_;

  glm::vec3 pos_change_;
  glm::vec3 rot_change_;
  glm::vec3 scaling_change_;

  glm::vec3 prefer_pos_;
  glm::vec3 prefer_rot_;
  glm::vec3 prefer_scaling_;

  glm::vec3 pos_adjust_factor_;
  glm::vec3 rot_adjust_factor_;
  glm::vec3 scaling_adjust_factor_;

  glm::vec3 max_pos_change_;
  glm::vec3 max_rot_change_;
  glm::vec3 max_scaling_change_;

  glm::vec3 pos_change_decay_;
  glm::vec3 rot_change_decay_;
  glm::vec3 scaling_change_decay_;

  glm::vec3 pos_bounce_force_;
  glm::vec3 rot_bounce_force_;
  glm::vec3 scaling_bounce_force_;

  glm::vec3 CalcPosBounceForce() const;

  glm::vec3 CalcRotBounceForce() const;

  glm::vec3 CalcScalingBounceForce() const;

  glm::vec3 CalcPosDiff() const;

  glm::vec3 CalcRotDiff() const;

  glm::vec3 CalcScalingDiff() const;

  glm::mat4 CalcRotMatrix(const glm::vec3 &angles) const;
};
}  // namespace ctrl
