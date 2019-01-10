#pragma once

#include "as/common.hpp"

namespace ctrl {
class AircraftController {
 public:
  AircraftController(
      const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &drift_dir,
      const float speed, const glm::vec3 &drift_dir_adjust_factor,
      const float speed_adjust_factor, const glm::vec3 &max_drift_dir_change,
      const float max_speed_change, const glm::vec3 &drift_dir_change_decay,
      const float speed_change_decay, const glm::vec3 &drift_dir_bounce_force,
      const float speed_bounce_force);

  glm::vec3 GetPos() const;

  glm::vec3 GetDir() const;

  glm::vec3 GetDriftDir() const;

  float GetSpeed() const;

  void SetPos(const glm::vec3 &pos);

  void SetDir(const glm::vec3 &dir);

  void SetPreferSpeed(const float prefer_speed);

  void SetWind(const glm::vec3 &drift_dir_wind, const float speed_wind,
               const glm::vec3 &drift_dir_wind_adjust_factor,
               const float speed_wind_adjust_factor,
               const glm::vec3 &max_drift_dir_wind, const float max_speed_wind);

  void AddDriftDir(const glm::vec3 &add_drift_dir);

  void AddSpeed(const float add_speed);

  void Update();

 private:
  std::random_device rand_device_;
  std::mt19937 rand_engine_;
  std::uniform_real_distribution<float> distrib_;

  glm::vec3 pos_;
  // Quaternion format
  glm::vec3 dir_;
  // Quaternion format
  glm::vec3 drift_dir_;
  float speed_;

  glm::vec3 drift_dir_change_;
  float speed_change_;

  glm::vec3 prefer_drift_dir_;
  float prefer_speed_;

  glm::vec3 drift_dir_adjust_factor_;
  float speed_adjust_factor_;

  glm::vec3 max_drift_dir_change_;
  float max_speed_change_;

  glm::vec3 drift_dir_change_decay_;
  float speed_change_decay_;

  glm::vec3 drift_dir_bounce_force_;
  float speed_bounce_force_;

  /* Wind */
  glm::vec3 drift_dir_wind_;
  float speed_wind_;

  glm::vec3 drift_dir_wind_adjust_factor_;
  float speed_wind_adjust_factor_;

  glm::vec3 max_drift_dir_wind_;
  float max_speed_wind_;

  glm::vec3 CalcDriftDirBounceForce() const;

  float CalcSpeedBounceForce() const;

  glm::vec3 CalcDriftDirDiff() const;

  float CalcSpeedDiff() const;

  glm::mat4 CalcRotMatrix(const glm::vec3 &angles) const;

  /* Random Numbers */

  glm::vec3 GenRand();
};
}  // namespace ctrl
