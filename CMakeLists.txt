cmake_minimum_required(VERSION 2.8)

project(swtec_test)

add_compile_options(-std=gnu++11 -fno-rtti)

############# Source code structure ############################################
set(SOURCES
    test.cpp
)

set(HEADERS
    Any.hpp
    AnyTypeStorage.hpp
    Observer.hpp
    UserTypes.hpp
    UserTypesOperations.hpp
)

############# Target ###########################################################
add_executable(swtec_test ${SOURCES} ${HEADERS})

target_link_libraries(swtec_test PRIVATE pthread)