set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ME_SERIALIZAED_OUT ${CMAKE_BINARY_DIR}/me-serialized-out)
file(MAKE_DIRECTORY ${ME_SERIALIZAED_OUT})
add_custom_target(class-info
        COMMAND me-compiler --readable -p ${CMAKE_BINARY_DIR}/compile_commands.json
        -o ${ME_SERIALIZAED_OUT}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alias_types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/scene.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/entity.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/component/camera.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/component/component.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/component/light.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/component/transform.h)

add_executable(me-serialization-example main.cpp)
add_dependencies(me-serialization-example class-info)
target_include_directories(me-serialization-example PRIVATE include)