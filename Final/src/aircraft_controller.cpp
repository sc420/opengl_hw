#include "aircraft_controller.hpp"

ctrl::AircraftController::AircraftController(
    const glm::vec3 &pos, const glm::vec3 &vel, const glm::vec3 &rot,
    const glm::vec3 &pos_adjust_factor, const glm::vec3 &vel_adjust_factor,
    const glm::vec3 &rot_adjust_factor, const glm::vec3 &pos_bounce_force,
    const glm::vec3 &vel_bounce_force, const glm::vec3 &rot_bounce_force)
    : pos_(pos),
      vel_(vel),
      rot_(rot),
      prefer_pos_(pos),
      prefer_vel_(vel),
      prefer_rot_(rot),
      pos_adjust_factor_(pos_adjust_factor),
      vel_adjust_factor_(vel_adjust_factor),
      rot_adjust_factor_(rot_adjust_factor),
      pos_bounce_force_(pos_bounce_force),
      vel_bounce_force_(vel_bounce_force),
      rot_bounce_force_(rot_bounce_force) {}

glm::vec3 ctrl::AircraftController::GetPos() const { return pos_; }

glm::vec3 ctrl::AircraftController::GetVel() const { return vel_; }

glm::vec3 ctrl::AircraftController::GetRot() const { return rot_; }

glm::vec3 ctrl::AircraftController::GetRotDir() const {
  const glm::mat4 rot_matrix = CalcRotMatrix(rot_);
  const glm::vec4 axis = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
  return rot_matrix * axis;
}

glm::vec3 ctrl::AircraftController::GetRotUp() const {
  const glm::mat4 rot_matrix = CalcRotMatrix(rot_);
  const glm::vec4 axis = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
  return rot_matrix * axis;
}

void ctrl::AircraftController::SetPreferPos(const glm::vec3 &prefer_pos) {
  prefer_pos_ = prefer_pos;
}

void ctrl::AircraftController::SetPreferVel(const glm::vec3 &prefer_vel) {
  prefer_vel_ = prefer_vel;
}

void ctrl::AircraftController::SetPreferRot(const glm::vec3 &prefer_rot) {
  prefer_rot_ = prefer_rot;
}

void ctrl::AircraftController::AddPos(const glm::vec3 &add_pos) {
  pos_ += pos_adjust_factor_ * add_pos;
}

void ctrl::AircraftController::AddVel(const glm::vec3 &add_vel) {
  vel_ += vel_adjust_factor_ * add_vel;
}

void ctrl::AircraftController::RotateVel(const glm::vec3 &rotate_vel) {
  const glm::mat4 rot_matrix = CalcRotMatrix(rotate_vel);
  vel_ = rot_matrix * glm::vec4(vel_, 0.0f);
}

void ctrl::AircraftController::AddRot(const glm::vec3 &add_rot) {
  rot_ += rot_adjust_factor_ * add_rot;
}

void ctrl::AircraftController::Update() {
  // Update the position
  pos_ += vel_;

  // Bounce back the values
  pos_ += CalcPosBounceForce() * (-CalcPosDrift());
  vel_ += CalcVelBounceForce() * (-CalcVelDrift());
  rot_ += CalcRotBounceForce() * (-CalcRotDrift());
}

glm::vec3 ctrl::AircraftController::CalcPosBounceForce() const {
  const glm::vec3 linear_force =
      pos_bounce_force_ * glm::length(CalcPosDrift());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::AircraftController::CalcVelBounceForce() const {
  const glm::vec3 linear_force =
      vel_bounce_force_ * glm::length(CalcVelDrift());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::AircraftController::CalcRotBounceForce() const {
  const glm::vec3 linear_force =
      rot_bounce_force_ * glm::length(CalcRotDrift());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::AircraftController::CalcPosDrift() const {
  return pos_ - prefer_pos_;
}

glm::vec3 ctrl::AircraftController::CalcVelDrift() const {
  return vel_ - prefer_vel_;
}

glm::vec3 ctrl::AircraftController::CalcRotDrift() const {
  return rot_ - prefer_rot_;
}

glm::mat4 ctrl::AircraftController::CalcRotMatrix(
    const glm::vec3 &rot_axes) const {
  // TODO: Merge with other code
  const glm::quat pitch =
      glm::angleAxis(rot_axes.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::quat yaw = glm::angleAxis(rot_axes.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::quat roll =
      glm::angleAxis(rot_axes.z, glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::quat orientation = glm::normalize(pitch * yaw * roll);
  return glm::mat4_cast(orientation);
}
