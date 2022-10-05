#include "ptr_updater.h"

constexpr static std::size_t kTimes = 200000;

int main() {
  UseData<LockData>(kTimes);
  UseData<LockFreeData>(kTimes);
}
