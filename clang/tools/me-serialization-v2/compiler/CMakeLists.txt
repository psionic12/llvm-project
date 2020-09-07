set(LLVM_LINK_COMPONENTS
        Support
        )

add_clang_tool(serializationc
        compiler.cpp
        record_info.cpp
        serializable_generator.cpp
        indexer.cpp
        )

clang_target_link_libraries(serializationc
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        )
