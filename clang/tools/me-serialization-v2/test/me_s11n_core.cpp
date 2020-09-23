#include "base_derive.s11n.h"
#include "foo.s11n.h"
namespace me {
namespace s11n {
CoderWrapperImpl<Base> _coder_wrapper_17;
CoderWrapperImpl<DerivedOne> _coder_wrapper_18;
CoderWrapperImpl<DerivedTwo> _coder_wrapper_19;

std::unordered_map<std::size_t, CoderWrapper *> CoderWrapper::IdToCoderMap{
    {129, &_coder_wrapper_17},
    {130, &_coder_wrapper_18},
    {131, &_coder_wrapper_19}};
std::unordered_map<std::type_index, std::size_t> CoderWrapper::CppIdToIdMap{
    {typeid(Base), 129},
    {typeid(DerivedOne), 130},
    {typeid(DerivedTwo), 131}};

} // namespace s11n
} // namespace me