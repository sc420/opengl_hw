#pragma once

#include "as/common.hpp"

namespace as {
class UiManager {
 public:
  UiManager();

  /* Window */

  void SaveWindowSize(const glm::ivec2 &window_size);

  void SaveActualWindowSize(const glm::ivec2 &window_size);

  glm::ivec2 GetWindowSize() const;

  glm::ivec2 GetActualWindowSize() const;

  float GetWindowAspectRatio() const;

  void MarkWindowClosed();

  bool IsWindowClosed() const;

  /* Keyboard */

  void MarkKeyDown(const unsigned char key);

  void MarkKeyUp(const unsigned char key);

  bool IsKeyDown(const unsigned char key) const;

  /* Mouse */

  void MarkMouseDown(const int button, const glm::ivec2 &mouse_pos);

  void MarkMouseMotion(const glm::ivec2 &mouse_pos);

  void MarkMouseUp(const int button, const glm::ivec2 &mouse_pos);

  bool IsMouseDown(const int button) const;

  glm::ivec2 GetMousePos() const;

  glm::ivec2 GetMouseDownPos() const;

  glm::ivec2 GetMouseMotionPos() const;

  /* Clock */

  void ResetClock();

  void StartClock();

  void StopClock();

  double CalcElapsedSeconds();

 private:
  /* Constants */
  static const int kNumKeyboardKeys = 256;
  static const int kNumMouseButtons = 3;

  /* Window */
  glm::ivec2 window_size_;
  glm::ivec2 actual_window_size_;
  bool is_window_closed_;

  /* Keyboard */
  bool key_down_[kNumKeyboardKeys];

  /* Mouse */
  bool mouse_down_[kNumMouseButtons];
  glm::ivec2 mouse_pos_;
  glm::ivec2 mouse_down_pos_;
  glm::ivec2 mouse_motion_pos_;

  /* Clock */
  bool is_clock_running_;
  std::chrono::time_point<std::chrono::system_clock> start_time_;
  std::chrono::time_point<std::chrono::system_clock> end_time_;

  /* Clock */

  void UpdateEndTime();
};
}  // namespace as
