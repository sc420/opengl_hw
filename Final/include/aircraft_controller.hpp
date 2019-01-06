#pragma once

#include "as/common.hpp"

namespace ctrl {
class AircraftController {
 public:
  AircraftController(const glm::vec3 &pos, const glm::vec3 &rot,
                     const float pos_adjust_factor,
                     const float rot_adjust_factor,
                     const float pos_bounce_force,
                     const float rot_bounce_force);

  glm::vec3 GetPos() const;

  glm::vec3 GetRot() const;

  void SetPreferPos(const glm::vec3 &prefer_pos);

  void SetPreferRot(const glm::vec3 &prefer_rot);

  void AddPos(const glm::vec3 &add_pos);

  void AddRot(const glm::vec3 &add_rot);

  void Update();

 private:
  glm::vec3 pos_;
  glm::vec3 rot_;
  glm::vec3 prefer_pos_;
  glm::vec3 prefer_rot_;
  float pos_adjust_factor_;
  float rot_adjust_factor_;
  float pos_bounce_force_;
  float rot_bounce_force_;

  float CalcPosBounceForce() const;

  float CalcRotBounceForce() const;

  glm::vec3 CalcPosDrift() const;

  glm::vec3 CalcRotDrift() const;
};
}  // namespace ctrl
