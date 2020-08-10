set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Support
        )

add_clang_tool(me-compiler
        compiler.cpp
        )

clang_target_link_libraries(me-compiler
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        )
