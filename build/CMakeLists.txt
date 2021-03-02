cmake_minimum_required(VERSION 3.4.1)
project(adlServer)
set(CMAKE_CXX_STANDARD 14)
link_libraries(-pthread)
SET(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} -g)

aux_source_directory(../src/base WS_BASE)
aux_source_directory(../src/http WS_HTTP)
aux_source_directory(../src/log WS_LOG)
aux_source_directory(../src/net  WS_NET)
aux_source_directory(../src/tool WS_TOOL)

add_executable(adlServer
        ../src/httpServer.cpp
        ${WS_HTTP}
        ${WS_NET}
        ${WS_LOG}
        ${WS_BASE}
        ${WS_TOOL}
)