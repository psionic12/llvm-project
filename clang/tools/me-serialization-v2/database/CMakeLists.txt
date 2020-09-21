add_clang_library(s11n_database
        record_database.cpp
        )

clang_target_link_libraries(s11n_database
        PRIVATE
        clangBasic
        clangLex
        clangFrontend
        clangToolingCore
        clangRewrite
        )
