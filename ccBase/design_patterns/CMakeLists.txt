# 设计模式统一模块
# 包含所有设计模式的实现和测试

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/design_patterns/include)

# 发布-订阅模式测试
add_executable(pub_sub_test
    test/pub_sub_test.cc
)
target_link_libraries(pub_sub_test pthread)

# 观察者模式测试
add_executable(observer_test
    test/observer_test.cc
)
target_link_libraries(observer_test pthread)

# 责任链模式测试
add_executable(chain_test
    test/chain_of_responsibility_test.cc
)
target_link_libraries(chain_test pthread)

# 异步回调模式测试
add_executable(async_callback_test
    test/async_callback_test.cc
)
target_link_libraries(async_callback_test pthread)

# 命令模式测试
add_executable(command_test
    test/command_test.cc
)
target_link_libraries(command_test pthread)

# 对象池模式测试
add_executable(object_pool_test
    test/object_pool_test.cc
)
target_link_libraries(object_pool_test pthread)

# 策略模式测试
add_executable(strategy_test
    test/strategy_test.cc
)
target_link_libraries(strategy_test pthread)

# 状态机模式测试
add_executable(state_machine_test
    test/state_machine_test.cc
)
target_link_libraries(state_machine_test pthread)

# 单例模式测试
add_executable(singleton_test
    test/singleton_test.cc
)
target_link_libraries(singleton_test pthread)

# 工厂模式测试
add_executable(factory_test
    test/factory_test.cc
)
target_link_libraries(factory_test pthread)

