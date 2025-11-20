cmake_minimum_required(VERSION 3.0.0)
project(wzq_utils)

message( STATUS "CMake version ${CMAKE_VERSION}" )
message (STATUS "source dir: ${CMAKE_SOURCE_DIR}")
message (STATUS "build type: ${CMAKE_BUILD_TYPE}")
message (STATUS "compiler: ${CMAKE_CXX_COMPILER}")

set (CMAKE_CXX_FLAGS "--std=c++17")

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/thread/include/
                    ${CMAKE_SOURCE_DIR}/design_patterns/include/
                    ${CMAKE_SOURCE_DIR}/logger/include/
#                    ${CMAKE_SOURCE_DIR}/pubsub/include/
#                    ${CMAKE_SOURCE_DIR}/observer/include/
#                    ${CMAKE_SOURCE_DIR}/chain/include/
#                    ${CMAKE_SOURCE_DIR}/async_callback/include/
#                    ${CMAKE_SOURCE_DIR}/command/include/
#                    ${CMAKE_SOURCE_DIR}/object_pool/include/
#                    ${CMAKE_SOURCE_DIR}/strategy/include/
#                    ${CMAKE_SOURCE_DIR}/state_machine/include/
#                    ${CMAKE_SOURCE_DIR}/singleton/include/
#                    ${CMAKE_SOURCE_DIR}/factory/include/
#                    ${CMAKE_SOURCE_DIR}/timer/include/
#                    ${CMAKE_SOURCE_DIR}/common/include/
                    )

add_subdirectory(thread)
add_subdirectory(design_patterns)
add_subdirectory(logger)
#add_subdirectory(pubsub)
#add_subdirectory(observer)
#add_subdirectory(chain)
#add_subdirectory(async_callback)
#add_subdirectory(command)
#add_subdirectory(object_pool)
#add_subdirectory(strategy)
#add_subdirectory(state_machine)
#add_subdirectory(singleton)
#add_subdirectory(factory)
#add_executable(test_wzq test.cc)
#target_link_libraries(test_wzq pthread)

