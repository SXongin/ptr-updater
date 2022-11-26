#include <iostream>
#include <memory>
#include <cstddef>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

struct LockData {
  void Destroy() {
    std::lock_guard<std::mutex> m{data_mutex_};
    delete data_;
    data_ = nullptr;
  }

  void Change(std::size_t const n) {
    Destroy();
    std::lock_guard<std::mutex> m{data_mutex_};
    data_ = std::make_unique<int>(n).release();
  }

  void Use() const {
    std::lock_guard<std::mutex> m{data_mutex_};
    if (data_) {
      auto cur = data_;
      ++*cur;
    }
  }

  mutable std::mutex data_mutex_{};
  int * data_{};
};

constexpr std::uint64_t kOccupiedFlag = ~(static_cast<std::uint64_t>(-1) >> 1);
struct LockFreeData {
  void Destroy() {
    std::uint64_t ptr_num;
    ptr_num = data_.load(std::memory_order_acquire);
    while (!data_.compare_exchange_weak(ptr_num, static_cast<std::uint64_t>(0), std::memory_order_release)) {
    }
    if (ptr_num & kOccupiedFlag) {
      return;
    }
    delete reinterpret_cast<int*>(ptr_num);
  }

  void Change(std::size_t const n) {
    Destroy();
    data_.store(reinterpret_cast<std::uint64_t>(std::make_unique<int>(n).release()), std::memory_order_release);
  }

  void Use() {
    std::uint64_t ptr_num;
    ptr_num = data_.load(std::memory_order_acquire);
    while(!data_.compare_exchange_weak(ptr_num, ptr_num | kOccupiedFlag, std::memory_order_release)) {
    }
    auto ptr = reinterpret_cast<int*>(ptr_num & ~kOccupiedFlag);
    if (ptr) {
      ++*ptr;
      std::uint64_t new_ptr_num = data_.load(std::memory_order_acquire);
      if (reinterpret_cast<int*>(new_ptr_num & ~kOccupiedFlag) == ptr) {
        if (!data_.compare_exchange_strong(new_ptr_num, new_ptr_num & ~kOccupiedFlag, std::memory_order_release)) {
          delete ptr;
        }
      } else {
        delete ptr;
      }
    }
  }

  std::atomic<std::uint64_t> data_{};
};

struct SharedData {
  void Destroy() {
    // donothing
  }

  void Change(std::size_t i) {
    std::atomic_store_explicit(&data_, std::make_shared<int>(i), std::memory_order_release);
  }

  void Use() {
    std::shared_ptr<int> tmp = std::atomic_load_explicit(&data_, std::memory_order_acquire);
    if (tmp) {
      ++(*tmp);
    }
  }

  std::shared_ptr<int> data_{};
};

template<typename T>
void UseData(std::size_t const times) {
  T d;
  std::thread c{[&d, times]() {
    for (std::size_t i = 0; i < times; ++i) {
      d.Change(i);
    }
  }};
  for (std::size_t i = 0; i < times; ++i) {
    d.Use();
  }
  c.join();
  d.Destroy();
}
