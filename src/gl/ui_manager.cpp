#include "as\gl\ui_manager.hpp"

void as::UiManager::ResetClock() {
  start_time_ = std::chrono::system_clock::now();
}

double as::UiManager::CalcElapsedSeconds() {
  const auto end_time = std::chrono::system_clock::now();
  const std::chrono::duration<double> elapsed_seconds = end_time - start_time_;
  return elapsed_seconds.count();
}
