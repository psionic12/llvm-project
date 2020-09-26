#include "base_derive.s11n.h"
#include "foo.s11n.h"
namespace me {
namespace s11n {
CoderWrapperImpl<Base> _coder_wrapper_129;
CoderWrapperImpl<DerivedOne> _coder_wrapper_130;
CoderWrapperImpl<DerivedTwo> _coder_wrapper_131;

std::unordered_map<std::size_t, CoderWrapper *> CoderWrapper::IdToCoderMap{
    {129, &_coder_wrapper_129},
    {130, &_coder_wrapper_130},
    {131, &_coder_wrapper_131},};
std::unordered_map<std::type_index, std::size_t> CoderWrapper::CppIdToIdMap{
    {typeid(Base), 129},
    {typeid(DerivedOne), 130},
    {typeid(DerivedTwo), 131}};

} // namespace s11n
} // namespace me