set(LLVM_LINK_COMPONENTS
    Support
)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加 math 子目录
#add_subdirectory(OCCPlusPlusBridge)
#add_clang_subdirectory(OCCPlusPlusBridge)

add_clang_executable(LTPluginTooling
    LTPluginTooling.hpp
    LTPluginTooling.cpp
    OCCPlusPlusBridge.h
    CPlusPlusCommunicationInterface.h
    CPlusPlusCommunicationInterface.m
    OCObjectRegisteredObserverCenter.h
    OCObjectRegisteredObserverCenter.m
)

# 指定生成目标
# add_executable(Demo ${DIR_SRCS})


target_link_libraries(LTPluginTooling
    PRIVATE
    clangAST
    clangBasic
    clangDriver
    clangFormat
    clangLex
    clangParse
    clangSema
    clangFrontend
    clangTooling
    clangToolingCore
    clangRewrite
    clangRewriteFrontend
    #OCCPlusPlusBridge
)

if (UNIX)
    set(CLANGXX__LING_OR_COPY create_symlink)
else()
    set(CLANGXX_LINK_OR_COPY copy)
endif()
