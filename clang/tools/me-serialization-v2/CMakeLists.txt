set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Support
        )

add_clang_tool(me-compiler-v2
        compiler/compiler.cpp
        compiler/record_info.cpp
        compiler/serializable_generator.cpp
        compiler/indexer.cpp
        )

clang_target_link_libraries(me-compiler-v2
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        )