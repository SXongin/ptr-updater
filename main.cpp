#include <iostream>
#include <memory>
#include <cstddef>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

constexpr static std::size_t kTimes = 200000;

struct LockData {
  void Destory() {
    std::lock_guard<std::mutex> m{data_mutex_};
    delete data_;
    data_ = nullptr;
  }

  void Change(std::size_t const n) {
    static std::vector<int> proto{};
    proto.push_back(n);
    Destory();
    std::lock_guard<std::mutex> m{data_mutex_};
    data_ = std::make_unique<std::vector<int>>(proto).release();
  }

  void Use() const {
    std::lock_guard<std::mutex> m{data_mutex_};
    if (data_) {
      auto cur = data_;
      std::cout << cur->size() << std::endl;
      // for (std::size_t i = 0; i < cur->size(); ++i) {
      //   std::cout << (*cur)[i] << std::endl;
      // }
    }
  }

  mutable std::mutex data_mutex_{};
  std::vector <int> * data_{};
};

constexpr std::uint64_t kOccupiedFlag = ~(static_cast<std::uint64_t>(-1) >> 1);
struct LockFreeData {
  void Destory() {
    std::uint64_t ptr_num;
    while (true) {
      ptr_num = data_.load(std::memory_order_acquire);
      if (data_.compare_exchange_weak(ptr_num, static_cast<std::uint64_t>(0), std::memory_order_release)) {
        break;
      }
      std::this_thread::yield();
    }
    if (ptr_num & kOccupiedFlag) {
      return;
    }
    delete reinterpret_cast<std::vector<int>*>(ptr_num);
  }

  void Change(std::size_t const n) {
    static std::vector<int> proto{};
    proto.push_back(n);
    std::cout << "Change : " << n << std::endl;
    Destory();
    data_.store(reinterpret_cast<std::uint64_t>(std::make_unique<std::vector<int>>(proto).release()), std::memory_order_release);
  }

  void Use() {
    std::uint64_t ptr_num;
    while(true) {
      ptr_num = data_.load(std::memory_order_acquire);
      if (data_.compare_exchange_weak(ptr_num, ptr_num | kOccupiedFlag, std::memory_order_release)) {
        break;
      }
      std::this_thread::yield();
    }
    auto ptr = reinterpret_cast<std::vector<int>*>(ptr_num & ~kOccupiedFlag);
    if (ptr) {
      std::cout << ptr->size() << std::endl;
      // for (std::size_t i = 0; i < ptr->size(); ++i) {
      //   std::cout << (*ptr)[i] << std::endl;
      // }
      std::uint64_t new_ptr_num = data_.load(std::memory_order_acquire);
      if (reinterpret_cast<std::vector<int> *>(new_ptr_num & ~kOccupiedFlag) == ptr) {
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

template<typename T>
void UseData() {
  T d;
  std::thread c{[&d]() {
    for (std::size_t i = 0; i < kTimes; ++i) {
      d.Change(i);
    }
  }};
  for (std::size_t i = 0; i < kTimes; ++i) {
    d.Use();
  }
  std::cout << "join ..." << std::endl;
  c.join();
  d.Destory();
}

int main() {
  // UseData<LockData>();
  UseData<LockFreeData>();
}
