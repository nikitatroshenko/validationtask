cmake_minimum_required(VERSION 3.8)
project(validationtask)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp err_handling.cpp hash_counting.cpp)
set(EXECUTABLE_NAME validationtask)
add_definitions(-DEXECUTABLE_NAME="${EXECUTABLE_NAME}")
add_executable(validationtask ${SOURCE_FILES})

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    link_directories(${OPENSSL_LIBRARIES})
    message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
    target_link_libraries(validationtask ${OPENSSL_LIBRARIES})
endif()

find_package(Boost COMPONENTS regex)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARIES})
    target_link_libraries(validationtask ${Boost_LIBRARIES})
endif()