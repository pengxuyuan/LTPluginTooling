set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} Support)

set(LLVM_OPTIONAL_SOURCES
    OCCPlusPlusBridge.h
    CPlusPlusCommunicationInterface.h
    CPlusPlusCommunicationInterface.m
    OCObjectRegisteredObserverCenter.h
    OCObjectRegisteredObserverCenter.m
    )   

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
#aux_source_directory(. DIR_LIB_SRCS)

# 生成链接库
#add_library (OCCPlusPlusBridge ${DIR_LIB_SRCS})

add_clang_library(OCCPlusPlusBridge
    OCCPlusPlusBridge.h
    CPlusPlusCommunicationInterface.h
    CPlusPlusCommunicationInterface.m
    OCObjectRegisteredObserverCenter.h
    OCObjectRegisteredObserverCenter.m
)

#add_clang_executable(OCCPlusPlusBridge ${DIR_SRCS})

#add_clang_executable(OCCPlusPlusBridge
    #OCCPlusPlusBridge.h
    #CPlusPlusCommunicationInterface.h
    #CPlusPlusCommunicationInterface.m
    #OCObjectRegisteredObserverCenter.h
    #OCObjectRegisteredObserverCenter.m
#)

#target_link_libraries(clang-proto-to-cxx PRIVATE OCCPlusPlusBridge)
