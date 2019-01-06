#pragma once

#include "as/common.hpp"

namespace ctrl {
class AircraftController {
 public:
  AircraftController(const glm::vec3 &pos, const glm::vec3 &rot,
                     const glm::vec3 &pos_adjust_factor,
                     const glm::vec3 &rot_adjust_factor,
                     const glm::vec3 &pos_bounce_force,
                     const glm::vec3 &rot_bounce_force);

  glm::vec3 GetPos() const;

  glm::vec3 GetRot() const;

  glm::vec3 GetRotDir() const;

  glm::vec3 GetRotUp() const;

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
  glm::vec3 pos_adjust_factor_;
  glm::vec3 rot_adjust_factor_;
  glm::vec3 pos_bounce_force_;
  glm::vec3 rot_bounce_force_;

  glm::vec3 CalcPosBounceForce() const;

  glm::vec3 CalcRotBounceForce() const;

  glm::vec3 CalcPosDrift() const;

  glm::vec3 CalcRotDrift() const;

  glm::mat4 CalcRotMatrix() const;
};
}  // namespace ctrl
