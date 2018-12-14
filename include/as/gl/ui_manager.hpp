#pragma once

#include "as/common.hpp"

namespace as {
class UiManager {
 public:
  /* Clock */

  void ResetClock();

  double CalcElapsedSeconds();

 private:
  std::chrono::time_point<std::chrono::system_clock> start_time_;
};
}  // namespace as
