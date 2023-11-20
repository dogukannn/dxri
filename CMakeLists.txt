cmake_minimum_required(VERSION 3.15)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

project(dxri)

set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External)
set(TARGETS dxri)

add_subdirectory(${EXTERNAL_DIR}/DirectX-Headers)
get_property(DEP DIRECTORY ${EXTERNAL_DIR}/DirectX-Headers PROPERTY BUILDSYSTEM_TARGETS)
list(APPEND TARGETS ${DEP})


add_subdirectory(${EXTERNAL_DIR}/DirectXTK12)
get_property(DEP DIRECTORY ${EXTERNAL_DIR}/DirectXTK12 PROPERTY BUILDSYSTEM_TARGETS)
list(APPEND TARGETS ${DEP})

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
)

add_library(${PROJECT_NAME} ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source/pch.h)

target_link_libraries(${PROJECT_NAME} PUBLIC DirectX-Headers DirectXTK12 d3d12 dxcompiler dxgi dxguid)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/include)

FOREACH(DEP ${TARGETS})
    if(TARGET ${DEP})
        set_property(TARGET ${DEP} PROPERTY FOLDER "dxri")
    endif()
ENDFOREACH(DEP)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "dxri")
#should I need to install as library??
# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/Source/include/dxri.h)
