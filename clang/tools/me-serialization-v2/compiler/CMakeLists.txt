set(LLVM_LINK_COMPONENTS
        Support
        )

add_clang_tool(serializationc
        compiler.cpp
        record_info.cpp
        serializable_generator.cpp
        indexer.cpp
        record_database.cpp
        )

clang_target_link_libraries(serializationc
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        ClangDiagnosticMES11n
        )

clang_tablegen(DiagnosticS11nKinds.inc -gen-clang-diags-defs -clang-component=MES11n
        SOURCE Diagnostic.td
        TARGET ClangDiagnosticMES11n)