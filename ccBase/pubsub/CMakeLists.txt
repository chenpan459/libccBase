# PubSub 模块
add_executable(pub_sub_test
    test/pub_sub_test.cc
)

target_link_libraries(pub_sub_test pthread)

