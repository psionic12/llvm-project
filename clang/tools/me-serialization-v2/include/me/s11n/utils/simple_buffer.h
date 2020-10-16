#ifndef S11N_INCLUDE_ME_S11N_UTILS_SIMPLE_BUFFER_H
#define S11N_INCLUDE_ME_S11N_UTILS_SIMPLE_BUFFER_H
#include <cstdint>
namespace me {
namespace s11n {
class SimpleBuffer {
public:
  uint8_t *Data() { return buffer_; }
  const uint8_t *Data() const { return buffer_; }
  uint8_t *End() { return buffer_ + size_; }
  const uint8_t *End() const { return buffer_ + size_; }
  void ReCapacity(std::size_t n) {
    if (size_ < n) {
      delete[] buffer_;
      buffer_ = new uint8_t[n];
      size_ = n;
    }
  }
  std::size_t Capacity() const { return size_; }
  ~SimpleBuffer() { delete[] buffer_; }

private:
  uint8_t *buffer_ = nullptr;
  std::size_t size_ = 0;
};
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_UTILS_SIMPLE_BUFFER_H
