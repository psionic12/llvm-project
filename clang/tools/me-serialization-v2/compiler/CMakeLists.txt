set(LLVM_LINK_COMPONENTS
        Support
        )

add_clang_tool(serializationc
        compiler.cpp
        RecordInfo.cpp
        SerializableConsumer.cpp
        CodeGuard.cpp
        fmt/format.cc
        fmt/os.cc
        RecordDatabase.cpp
        CodeGenerator.cpp
        )

target_include_directories(serializationc PUBLIC ../include)

clang_target_link_libraries(serializationc
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        s11n_database
        )