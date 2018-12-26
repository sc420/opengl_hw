#include "as\gl\ui_manager.hpp"

as::UiManager::UiManager()
    : window_size_(glm::ivec2(0)),
      is_window_closed_(false),
      key_down_(),
      mouse_down_(),
      mouse_pos_(glm::ivec2(0)),
      mouse_down_pos_(glm::ivec2(0)),
      mouse_motion_pos_(glm::ivec2(0)),
      is_clock_running_(false),
      start_time_(std::chrono::system_clock::now()),
      end_time_(std::chrono::system_clock::now()) {}

/*******************************************************************************
 * Window
 ******************************************************************************/

void as::UiManager::SaveWindowSize(const glm::ivec2& window_size) {
  window_size_ = window_size;
}

glm::ivec2 as::UiManager::GetWindowSize() const { return window_size_; }

float as::UiManager::GetWindowAspectRatio() const {
  return static_cast<float>(window_size_.x) /
         static_cast<float>(window_size_.y);
}

void as::UiManager::MarkWindowClosed() { is_window_closed_ = true; }

bool as::UiManager::IsWindowClosed() const { return is_window_closed_; }

/*******************************************************************************
 * Keyboard
 ******************************************************************************/

void as::UiManager::MarkKeyDown(const unsigned char key) {
  key_down_[key] = true;
}

void as::UiManager::MarkKeyUp(const unsigned char key) {
  key_down_[key] = false;
}

bool as::UiManager::IsKeyDown(const unsigned char key) const {
  return key_down_[key];
}

/*******************************************************************************
 * Mouse
 ******************************************************************************/

void as::UiManager::MarkMouseDown(const int button,
                                  const glm::ivec2& mouse_pos) {
  mouse_down_[button] = true;
  mouse_pos_ = mouse_pos;
  mouse_down_pos_ = mouse_pos;
}

void as::UiManager::MarkMouseMotion(const glm::ivec2& mouse_pos) {
  mouse_pos_ = mouse_pos;
  mouse_motion_pos_ = mouse_pos;
}

void as::UiManager::MarkMouseUp(const int button, const glm::ivec2& mouse_pos) {
  mouse_down_[button] = false;
  mouse_pos_ = mouse_pos;
}

bool as::UiManager::IsMouseDown(const int button) const {
  return mouse_down_[button];
}

glm::ivec2 as::UiManager::GetMousePos() const { return mouse_pos_; }

glm::ivec2 as::UiManager::GetMouseDownPos() const { return mouse_down_pos_; }

glm::ivec2 as::UiManager::GetMouseMotionPos() const {
  return mouse_motion_pos_;
}

/*******************************************************************************
 * Clock
 ******************************************************************************/

void as::UiManager::ResetClock() {
  start_time_ = std::chrono::system_clock::now();
  end_time_ = start_time_;
}

void as::UiManager::StartClock() {
  UpdateEndTime();
  is_clock_running_ = true;
}

void as::UiManager::StopClock() {
  UpdateEndTime();
  is_clock_running_ = false;
}

double as::UiManager::CalcElapsedSeconds() {
  if (is_clock_running_) {
    UpdateEndTime();
  }
  const std::chrono::duration<double> elapsed_seconds = end_time_ - start_time_;
  return elapsed_seconds.count();
}

/*******************************************************************************
 * Clock (Private)
 ******************************************************************************/

void as::UiManager::UpdateEndTime() {
  end_time_ = std::chrono::system_clock::now();
}
