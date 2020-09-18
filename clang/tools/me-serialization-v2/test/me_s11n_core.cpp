#include "base_derive.s11n.h"
#include "foo.s11n.h"
namespace me {
namespace s11n {

CoderWrapperImpl<bool> _coder_wrapper_bool;
CoderWrapperImpl<char> _coder_wrapper_char;
CoderWrapperImpl<unsigned char> _coder_wrapper_unsigned_char;
CoderWrapperImpl<signed char> _coder_wrapper_signed_char;
CoderWrapperImpl<float> _coder_wrapper_float;
CoderWrapperImpl<double> _coder_wrapper_double;
CoderWrapperImpl<long double> _coder_wrapper_long_double;
CoderWrapperImpl<short> _coder_wrapper_short;
CoderWrapperImpl<int> _coder_wrapper_int;
CoderWrapperImpl<long> _coder_wrapper_long;
CoderWrapperImpl<long long> _coder_wrapper_long_long;
CoderWrapperImpl<unsigned short> _coder_wrapper_unsigned_short;
CoderWrapperImpl<unsigned int> _coder_wrapper_unsigned_int;
CoderWrapperImpl<unsigned long> _coder_wrapper_unsigned_long;
CoderWrapperImpl<unsigned long long> _coder_wrapper_unsigned_long_long;

CoderWrapperImpl<Foo> _coder_wrapper_Foo;
CoderWrapperImpl<Base> _coder_wrapper_Base;
CoderWrapperImpl<DerivedOne> _coder_wrapper_DerivedOne;
CoderWrapperImpl<DerivedTwo> _coder_wrapper_DerivedTwo;

std::unordered_map<std::size_t, CoderWrapper *> CoderWrapper::IdToCoderMap{
    {1, &_coder_wrapper_bool},
    {2, &_coder_wrapper_char},
    {3, &_coder_wrapper_unsigned_char},
    {4, &_coder_wrapper_signed_char},
    {5, &_coder_wrapper_float},
    {6, &_coder_wrapper_double},
    {7, &_coder_wrapper_long_double},
    {8, &_coder_wrapper_short},
    {9, &_coder_wrapper_int},
    {10, &_coder_wrapper_long},
    {11, &_coder_wrapper_long_long},
    {12, &_coder_wrapper_unsigned_short},
    {13, &_coder_wrapper_unsigned_int},
    {14, &_coder_wrapper_unsigned_long},
    {15, &_coder_wrapper_unsigned_long_long},
    {128, &_coder_wrapper_Foo},
    {129, &_coder_wrapper_Base},
    {130, &_coder_wrapper_DerivedOne},
    {131, &_coder_wrapper_DerivedTwo}};
std::unordered_map<std::type_index, std::size_t> CoderWrapper::CppIdToIdMap{
    {typeid(bool), 1},
    {typeid(char), 2},
    {typeid(unsigned char), 3},
    {typeid(signed char), 4},
    {typeid(float), 5},
    {typeid(double), 6},
    {typeid(long double), 7},
    {typeid(short), 8},
    {typeid(int), 9},
    {typeid(long), 10},
    {typeid(long long), 11},
    {typeid(unsigned short), 12},
    {typeid(unsigned int), 13},
    {typeid(unsigned long), 14},
    {typeid(unsigned long long), 15},
    {typeid(Foo), 128},
    {typeid(Base), 129},
    {typeid(DerivedOne), 130},
    {typeid(DerivedTwo), 131}};

} // namespace serialization
} // namespace me