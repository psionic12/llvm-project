#ifndef S11N_INCLUDE_ME_S11N_UTILS_COMMON_H
#define S11N_INCLUDE_ME_S11N_UTILS_COMMON_H
#include <type_traits>
namespace me {
namespace s11n {
// helper to avoid static_assert trigger directly
template <typename> struct DeferredFalse : std::false_type {};

template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

inline const uint8_t *SkipVarint(const uint8_t *ptr) {
  for (std::uint64_t i = 0; i < 10; i++) {
    if (/*likely*/ (static_cast<uint8_t>(ptr[i])) < 128) {
      ptr += (i + 1);
      break;
    }
  }
  return ptr;
}
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_UTILS_COMMON_H
