#ifndef S11N_INCLUDE_ME_S11N_UTILS_SIZE_CACHE_H
#define S11N_INCLUDE_ME_S11N_UTILS_SIZE_CACHE_H
#include <typeindex>
#include <unordered_map>
namespace me {
namespace s11n {
template <typename T = void> // use this way
// to implement inline variable feature in c++ 17
class SizeCache {
private:
  static thread_local std::unordered_map<
      std::type_index, std::unordered_map<const void *, std::size_t>>
      size_tables_;

public:
  template <typename U> static void Set(const U &u, std::size_t size) {
    size_tables_[typeid(U)].emplace(static_cast<const void *>(&u), size);
  }
  template <typename U> static std::size_t Get(const U &u) {
    return size_tables_[typeid(U)][static_cast<const void *>(&u)];
  }

  template <typename U, std::size_t SIZE>
  static void Set(const U (&u)[SIZE], std::size_t size) {
    size_tables_[typeid(U[SIZE])].emplace(static_cast<const void *>(&u), size);
  }
  template <typename U, std::size_t SIZE>
  static std::size_t Get(const U (&u)[SIZE]) {
    return size_tables_[typeid(U[SIZE])][static_cast<const void *>(&u)];
  }
  class WriteGuard {
  public:
    ~WriteGuard() { size_tables_.clear(); }
  };
};
template <typename T>
thread_local std::unordered_map<std::type_index,
                                std::unordered_map<const void *, std::size_t>>
    SizeCache<T>::size_tables_;
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_UTILS_SIZE_CACHE_H
