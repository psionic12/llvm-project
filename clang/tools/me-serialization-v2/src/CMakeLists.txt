set(LLVM_LINK_COMPONENTS
        Support
        )

add_clang_tool(serializationc
        main.cpp
        common/RecordInfo.cpp
        common/RecordDatabase.cpp
        common/CodeGenerator.cpp
        common/CodeGuard.cpp
        fmt/format.cc
        fmt/os.cc
        compiler/SerializableConsumer.cpp
        linker/ObjectDecoder.cpp
        )

target_include_directories(serializationc PUBLIC ../include)

clang_target_link_libraries(serializationc
        PRIVATE
        clangASTMatchers
        clangBasic
        clangTooling
        s11n_database
        )