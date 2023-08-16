cmake_minimum_required(VERSION 3.3)

project(cmdlineflags VERSION 1.0.1)

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

option(BUILD_SHARED_LIBS        "Build shared libraries" ON)
option(BUILD_CMDLINEFLAGS_DOCS  "Build documentation" OFF)
option(BUILD_CMDLINEFLAGS_TESTS "Enable testing" OFF)

if(BUILD_CMDLINEFLAGS_DOCS)
    find_package(Doxygen REQUIRED)
endif()

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

if(BUILD_CMDLINEFLAGS_TESTS)
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

configure_file(version.h.in version.h)

#------------------------------------------------------------------------------
#                             TARGET(S) DEFINITION(S)
#------------------------------------------------------------------------------
set(CMDLINEFLAGS_PUBLIC_HEADERS
    ${CMDLINEFLAGS_API_DIR}/cmdlineflags.h
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
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR} // this is the directory where 'version.h' will be configured
        ${CMDLINEFLAGS_INC_DIR}
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER ${CMDLINEFLAGS_PUBLIC_HEADERS}
)

#------------------------------------------------------------------------------
#                                 INSTALLATION
#------------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(FILES ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Add all targets to the install-tree export set
install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Add all targets to the build-tree export set
export(EXPORT ${PROJECT_NAME}-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-targets.cmake
)

#------------------------------------------------------------------------------
#                                DOCUMENTATION
#------------------------------------------------------------------------------
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})

    add_custom_target(docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif(DOXYGEN_FOUND)

#------------------------------------------------------------------------------
#                                    TESTS
#------------------------------------------------------------------------------
if(BUILD_CMDLINEFLAGS_TESTS)
    enable_testing()
    add_subdirectory(tst)
endif(BUILD_CMDLINEFLAGS_TESTS)
