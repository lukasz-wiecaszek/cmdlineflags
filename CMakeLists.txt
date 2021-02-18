cmake_minimum_required(VERSION 3.3)

project(cmdlineflags VERSION 1.0.0)

# sets various paths used in e.g. pc.in files as well as install target
include(GNUInstallDirs)

message(STATUS "Processing CMakeLists.txt for: " ${PROJECT_NAME} " " ${PROJECT_VERSION})

# if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise
# this is the top level directory of your build tree
message(STATUS "CMAKE_BINARY_DIR:         " ${CMAKE_BINARY_DIR})

# if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this
# is the directory where the compiled or generated files from the current CMakeLists.txt will go to
message(STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR})

# this is the directory, from which cmake was started, i.e. the top level source directory
message(STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR})

# this is the directory where the currently processed CMakeLists.txt is located in
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})

# CMDLINEFLAGS_SUBDIRECTORY_USAGE is set to TRUE when cmdlineflags is included
# as subproject (via add_subdirectory()) in the source tree  of a project that uses it.
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMDLINEFLAGS_SUBDIRECTORY_USAGE FALSE)
else()
    set(CMDLINEFLAGS_SUBDIRECTORY_USAGE TRUE)
endif()

option(CMDLINEFLAGS_TESTS "Enable testing" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
         "Choose the type of build. Options are: {Release, Debug}." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)

add_compile_options(-Wall -Werror -pedantic)

if("${CMAKE_BUILD_TYPE_LOWER}" STREQUAL "debug")
    add_compile_options(-ggdb3)
endif()

if(CMDLINEFLAGS_TESTS)
    add_compile_options(-fprofile-arcs -ftest-coverage)
endif()

set(CMDLINEFLAGS_API_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/api
)

set(CMDLINEFLAGS_INC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

set(CMDLINEFLAGS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(CMDLINEFLAGS_SRCS
    ${CMDLINEFLAGS_SRC_DIR}/cmdlineflags.c
)

add_library(${PROJECT_NAME}
    ${CMDLINEFLAGS_SRCS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMDLINEFLAGS_API_DIR}>
    PRIVATE
        ${CMDLINEFLAGS_INC_DIR}
)

#------------------------------------------------------------------------------
#                                 INSTALLATION
#------------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(DIRECTORY ${CMDLINEFLAGS_API_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

#------------------------------------------------------------------------------
#                                    TESTS
#------------------------------------------------------------------------------
if(CMDLINEFLAGS_TESTS)
    enable_testing()
    add_subdirectory(tst)
endif()
