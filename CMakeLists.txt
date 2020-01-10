cmake_minimum_required(VERSION 2.8)
set(PROJ_NAME NESemu)
project(${PROJ_NAME} CXX)

set(PROJECT_SOURCE_DIR src)
set(PROJECT_INCLUDE_DIR include)
aux_source_directory(${PROJECT_SOURCE_DIR} SRC_FILES)

add_executable(${PROJ_NAME}
    ${SRC_FILES}
)

target_include_directories(${PROJ_NAME}
  PRIVATE ${PROJECT_INCLUDE_DIR}
)

target_compile_options(${PROJ_NAME}
  PUBLIC -Wall -g -O0
)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")
# Find SFML
if (SFML_OS_WINDOWS AND SFML_COMPILER_MSVC)
    find_package( SFML 2 COMPONENTS main audio graphics window system network REQUIRED)
else()
    find_package( SFML 2 COMPONENTS audio graphics window system network REQUIRED)
endif()

if(SFML_FOUND)
        include_directories(${SFML_INCLUDE_DIR})
else()
        set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
        message("\nSFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
        message("Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

target_link_libraries(${PROJ_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

set_property(TARGET ${PROJ_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJ_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(${PROJ_NAME})