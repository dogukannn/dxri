cmake_minimum_required(VERSION 3.15)

project(dxri)

set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External)

add_subdirectory(${EXTERNAL_DIR}/DirectX-Headers)
add_subdirectory(${EXTERNAL_DIR}/DirectXTK12)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
)

add_library(${PROJECT_NAME} ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source/pch.h)

target_link_libraries(${PROJECT_NAME} PUBLIC DirectX-Headers DirectXTK12 d3d12 dxcompiler dxgi dxguid)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/include)

#should I need to install as library??
# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/Source/include/dxri.h)
