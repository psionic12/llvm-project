set(LLVM_LINK_COMPONENTS
        Support
        )


add_clang_library(serialization
        serialization/serializable.cpp
        serialization/codede.cpp)

add_clang_tool(serializationc
        compiler/compiler.cpp
        compiler/record_info.cpp
        compiler/serializable_generator.cpp
        compiler/indexer.cpp
        )

clang_target_link_libraries(serializationc
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        )
