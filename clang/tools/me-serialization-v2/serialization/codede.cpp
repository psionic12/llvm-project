#include "codede.h"

uint8_t *me::serialization::WriteIndexToArray(uint16_t index, uint8_t *target) {
  return EncodeHelper<>::EncodeTypeToArray(index, target);
}
uint8_t *me::serialization::WriteTagToArray(bool repeated,
                                            me::serialization::TypeSize size,
                                            uint8_t *target) {
  uint8_t tag = *target;
  tag = repeated ? 0b10000000 : 0;
  tag |= static_cast<int>(size);
  return ++target;
}
