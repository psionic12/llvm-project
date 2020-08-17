find_package(Protobuf REQUIRED)
add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)
include_directories(${Protobuf_INCLUDE_DIRS})
set(GENERATED_SRCS ${CMAKE_CURRENT_BINARY_DIR}/generated_srcs)
file(MAKE_DIRECTORY ${GENERATED_SRCS})
include_directories(${GENERATED_SRCS})
set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Support
        )

add_clang_tool(me-compiler
        compiler.cpp
        record_info.proto
        )

clang_target_link_libraries(me-compiler
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        protobuf::libprotobuf
        protobuf::libprotoc
        )
protobuf_generate(TARGET me-compiler IMPORT_DIRS ${Protobuf_INCLUDE_DIRS} PROTOC_OUT_DIR ${GENERATED_SRCS})