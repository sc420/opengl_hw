#pragma once

#include "as/common.hpp"

namespace ctrl {
class AircraftController {
 public:
  AircraftController(const glm::vec3 &pos, const glm::vec3 &dir,
                     const glm::vec3 &drift_dir, const float speed,
                     const glm::vec3 &drift_dir_adjust_factor,
                     const float speed_adjust_factor,
                     const glm::vec3 &drift_dir_bounce_force,
                     const float speed_bounce_force);

  glm::vec3 GetPos() const;

  glm::vec3 GetDir() const;

  glm::vec3 GetDriftDir() const;

  float GetSpeed() const;

  void AddDriftDir(const glm::vec3 &add_drift_dir);

  void AddSpeed(const float add_speed);

  void Update();

 private:
  glm::vec3 pos_;
  // Quaternion format
  glm::vec3 dir_;
  // Quaternion format
  glm::vec3 drift_dir_;
  float speed_;

  glm::vec3 prefer_drift_dir_;
  float prefer_speed_;

  glm::vec3 drift_dir_adjust_factor_;
  float speed_adjust_factor_;

  glm::vec3 drift_dir_bounce_force_;
  float speed_bounce_force_;

  glm::vec3 CalcDriftDirBounceForce() const;

  float CalcSpeedBounceForce() const;

  glm::vec3 CalcDriftDirDrift() const;

  float CalcSpeedDrift() const;

  glm::mat4 CalcRotMatrix(const glm::vec3 &angles) const;
};
}  // namespace ctrl
