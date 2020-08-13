set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_target(class_info
        COMMAND me-compiler --readable -p ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/include/scene.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alias_types.h)

add_executable(me-serialization-example main.cpp)
target_include_directories(me-serialization-example PRIVATE include)