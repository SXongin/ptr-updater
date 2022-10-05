#include <benchmark/benchmark.h>

constexpr static std::size_t kTimes = 100000;

#include "ptr_updater.h"

static void BM_LockData(benchmark::State& state) {
  for (auto _ : state) {
    UseData<LockData>(kTimes);
  }
}
BENCHMARK(BM_LockData);

static void BM_LockFreeData(benchmark::State& state) {
  for (auto _ : state) {
    UseData<LockFreeData>(kTimes);
  }
}
BENCHMARK(BM_LockFreeData);
