set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(me_s11n_test
        built_in_type_s11n_test.cpp
        me_s11n_core.cpp
        custom_type_s11n_test.cpp
        rtti_type_test.cpp
        )
target_link_libraries(me_s11n_test ${GTEST_BOTH_LIBRARIES})
target_include_directories(me_s11n_test PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(me_s11n_test Threads::Threads)
target_include_directories(me_s11n_test PRIVATE ../include)


add_test(AllTests me_s11n_test)