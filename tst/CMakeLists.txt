cmake_minimum_required(VERSION 3.3)

project(cmdlineflags_tests VERSION 1.0.0)

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

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
         "Choose the type of build. Options are: {Release, Debug}." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} --coverage)

set(CMDLINEFLAGS_TESTS_API_DIR
)

set(CMDLINEFLAGS_TESTS_INC_DIR
)

set(CMDLINEFLAGS_TESTS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

function(add_test_executable name)
    add_executable(${name} ${name})
    target_link_libraries(${name} PRIVATE cmdlineflags)
endfunction()

add_test_executable(cmdlineflags_no_module_tests)
add_test_executable(cmdlineflags_module_tests)

add_test(NAME test01 COMMAND $<TARGET_FILE:cmdlineflags_no_module_tests>
    -i2 -j2 -v -c configuration.file -v -cconfiguration.file - -v -cconfiguration.file)

add_test(NAME test02 COMMAND $<TARGET_FILE:cmdlineflags_no_module_tests>
    -i1 -j1 -varg1 -c -v arg2 -c)

add_test(NAME test03 COMMAND $<TARGET_FILE:cmdlineflags_no_module_tests>
    --expected_v_cnt=2 --expected_c_cnt=2 --version --configuration=configuration.file --version --configuration configuration.file)

add_test(NAME test04 COMMAND $<TARGET_FILE:cmdlineflags_no_module_tests>
    --expected_v_cnt=0 --expected_c_cnt=1 --invalid-option --version=arg1 --configuration --version arg2 --configuration)

add_test(NAME test05 COMMAND $<TARGET_FILE:cmdlineflags_module_tests>
    -i2 -j2 -h module_name -v -c configuration.file -v -cconfiguration.file)

add_test(NAME test06 COMMAND $<TARGET_FILE:cmdlineflags_module_tests>
    -i1 -j1 -h module_name -varg1 -c -v arg2 -c)

add_test(NAME test07 COMMAND $<TARGET_FILE:cmdlineflags_module_tests>
    --expected_v_cnt=2 --expected_c_cnt=2 --help module_name --version --configuration=configuration.file --version --configuration configuration.file)

add_test(NAME test08 COMMAND $<TARGET_FILE:cmdlineflags_module_tests>
    --expected_v_cnt=0 --expected_c_cnt=1 --help module_name --invalid-option --version=arg1 --configuration --version arg2 --configuration)
