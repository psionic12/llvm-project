#ifndef S11N_INCLUDE_ME_S11N_RTTI_TYPE_CODER_H
#define S11N_INCLUDE_ME_S11N_RTTI_TYPE_CODER_H
#include "type_coder.h"
namespace me {
namespace s11n {
struct CoderPicker {
  static std::unordered_map<std::size_t, CoderPicker *> IdToCoderMap;
  static std::unordered_map<std::type_index, std::size_t> CppIdToIdMap;
  virtual uint8_t *Write(const void *value, uint8_t *ptr) = 0;
  virtual const uint8_t *Read(void *out, const uint8_t *ptr) = 0;
  virtual std::size_t Size(const void *value) = 0;
  virtual void *New() = 0;
};

template <typename T> struct CoderPickerImpl : public CoderPicker {
  uint8_t *Write(const void *value, uint8_t *ptr) override {
    return Encode(*reinterpret_cast<const T *>(value), ptr);
  }
  const uint8_t *Read(void *out, const uint8_t *ptr) override {
    return Decode(*reinterpret_cast<T *>(out), ptr);
  }
  std::size_t Size(const void *value) override {
    return Capacity(*reinterpret_cast<const T *>(value));
  }
  void *New() override {
    // this means a serializable must has a default constructor
    return new T();
  }
};
// unique_ptr Type for polymorphic type
template <typename T, typename... TS>
struct TypeCoder<std::unique_ptr<T, TS...>,
                 typename std::enable_if<std::is_polymorphic<T>::value>::type> {
  static uint8_t *Write(const std::unique_ptr<T, TS...> &unique_ptr,
                        uint8_t *ptr) {

    if (unique_ptr.get() == nullptr) {
      auto id = 0;
      ptr = Encode(id, ptr);
    } else {
      auto id = CoderPicker::CppIdToIdMap[typeid(*unique_ptr)];
      ptr = Encode(id, ptr);
      ptr = CoderPicker::IdToCoderMap[id]->Write(unique_ptr.get(), ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(std::unique_ptr<T, TS...> &unique_ptr,
                             const uint8_t *ptr) {
    std::size_t id;
    ptr = Decode(id, ptr);
    if (id == 0) {
      unique_ptr.reset(nullptr);
    } else {
      void *out = CoderPicker::IdToCoderMap[id]->New();
      ptr = CoderPicker::IdToCoderMap[id]->Read(out, ptr);
      unique_ptr.reset(reinterpret_cast<T *>(out));
    }
    return ptr;
  }
  static std::size_t Size(const std::unique_ptr<T, TS...> &unique_ptr) {
    if (unique_ptr == nullptr) {
      return Capacity(0);
    } else {
      std::size_t id = CoderPicker::CppIdToIdMap[typeid(*unique_ptr)];
      return Capacity(id) + CoderPicker::IdToCoderMap[id]->Size(unique_ptr.get());
    }
  }
};

// unique_ptr Type for non-polymorphic type not supported
template <typename T, typename... TS>
struct TypeCoder<
    std::unique_ptr<T, TS...>,
    typename std::enable_if<!std::is_polymorphic<T>::value>::value> {
  static void CompileError() {
    static_assert(
        DeferredFalse<T>::value,
        "due to ownership principle, ptr of a non polymorphic type makes "
        "little sense, please issue us if you has a reasonable case");
  }
  static uint8_t *Write(const std::unique_ptr<T, TS...> &unique_ptr,
                        uint8_t *ptr) {
    CompileError();
    return nullptr;
  }
  static const uint8_t *Read(std::unique_ptr<T, TS...> &unique_ptr,
                             const uint8_t *ptr) {
    CompileError();
    return nullptr;
  }
  static std::size_t Size(const std::unique_ptr<T, TS...> &unique_ptr) {
    CompileError();
    return 0;
  }
};

// shared_ptrs are not supported
template <typename T> struct TypeCoder<std::shared_ptr<T>> {
  static void CompileError() {
    static_assert(
        DeferredFalse<T>::value,
        "shared_ptr is not supported, serializable should owns something, not "
        "shares something.");
  }
  static uint8_t *Write(const T &value, uint8_t *ptr) {
    CompileError();
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    CompileError();
    return nullptr;
  }
  static std::size_t Size(const T &value) {
    CompileError();
    return 0;
  }
};
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_RTTI_TYPE_CODER_H
