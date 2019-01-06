#include "aircraft_controller.hpp"

ctrl::AircraftController::AircraftController(const glm::vec3& pos,
                                             const glm::vec3& rot,
                                             const float pos_adjust_factor,
                                             const float rot_adjust_factor,
                                             const float pos_bounce_force,
                                             const float rot_bounce_force)
    : pos_(pos),
      rot_(rot),
      prefer_pos_(pos),
      prefer_rot_(rot),
      pos_adjust_factor_(pos_adjust_factor),
      rot_adjust_factor_(rot_adjust_factor),
      pos_bounce_force_(pos_bounce_force),
      rot_bounce_force_(rot_bounce_force) {}

glm::vec3 ctrl::AircraftController::GetPos() const { return pos_; }

glm::vec3 ctrl::AircraftController::GetRot() const { return rot_; }

void ctrl::AircraftController::SetPreferPos(const glm::vec3& prefer_pos) {
  prefer_pos_ = prefer_pos;
}

void ctrl::AircraftController::SetPreferRot(const glm::vec3& prefer_rot) {
  prefer_rot_ = prefer_rot;
}

void ctrl::AircraftController::AddPos(const glm::vec3& add_pos) {
  pos_ += pos_adjust_factor_ * add_pos;
}

void ctrl::AircraftController::AddRot(const glm::vec3& add_rot) {
  rot_ += rot_adjust_factor_ * add_rot;
}

void ctrl::AircraftController::Update() {
  pos_ += CalcPosBounceForce() * (-CalcPosDrift());
  rot_ += CalcRotBounceForce() * (-CalcRotDrift());
}

float ctrl::AircraftController::CalcPosBounceForce() const {
  const float linear_force = pos_bounce_force_ * glm::length(CalcPosDrift());
  return glm::exp(linear_force) - 1.0f;
}

float ctrl::AircraftController::CalcRotBounceForce() const {
  const float linear_force = rot_bounce_force_ * glm::length(CalcRotDrift());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::AircraftController::CalcPosDrift() const {
  return pos_ - prefer_pos_;
}

glm::vec3 ctrl::AircraftController::CalcRotDrift() const {
  return rot_ - prefer_rot_;
}
