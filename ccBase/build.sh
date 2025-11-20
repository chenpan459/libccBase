#!/bin/bash

# 编译脚本 - Linux/macOS
# 使用方法: ./build.sh [debug|release] [clean]

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 默认构建类型
BUILD_TYPE=${1:-release}
CLEAN=${2:-}

# 项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

# 检查构建类型
if [ "$BUILD_TYPE" != "debug" ] && [ "$BUILD_TYPE" != "release" ]; then
    echo -e "${RED}错误: 构建类型必须是 'debug' 或 'release'${NC}"
    echo "使用方法: $0 [debug|release] [clean]"
    exit 1
fi

# 转换为 CMake 构建类型
if [ "$BUILD_TYPE" == "debug" ]; then
    CMAKE_BUILD_TYPE="Debug"
else
    CMAKE_BUILD_TYPE="Release"
fi

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}    开始编译项目${NC}"
echo -e "${GREEN}========================================${NC}"
echo "项目根目录: $PROJECT_ROOT"
echo "构建目录: $BUILD_DIR"
echo "构建类型: $CMAKE_BUILD_TYPE"
echo ""

# 清理构建目录
if [ "$CLEAN" == "clean" ] || [ "$CLEAN" == "c" ]; then
    echo -e "${YELLOW}清理构建目录...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}清理完成${NC}"
    echo ""
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置 CMake
echo -e "${YELLOW}配置 CMake...${NC}"
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_CXX_FLAGS="-std=c++17 -Wall -Wextra"

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake 配置失败${NC}"
    exit 1
fi

# 编译
echo ""
echo -e "${YELLOW}开始编译...${NC}"
cmake --build . --config "$CMAKE_BUILD_TYPE" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo -e "${RED}编译失败${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}    编译成功！${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "可执行文件位置:"
echo "  - 线程池测试: $BUILD_DIR/thread/thread_poo_test"
echo "  - 发布订阅测试: $BUILD_DIR/design_patterns/pub_sub_test"
echo "  - 观察者模式测试: $BUILD_DIR/design_patterns/observer_test"
echo "  - 责任链模式测试: $BUILD_DIR/design_patterns/chain_test"
echo "  - 异步回调测试: $BUILD_DIR/design_patterns/async_callback_test"
echo "  - 命令模式测试: $BUILD_DIR/design_patterns/command_test"
echo "  - 对象池测试: $BUILD_DIR/design_patterns/object_pool_test"
echo "  - 策略模式测试: $BUILD_DIR/design_patterns/strategy_test"
echo "  - 状态机测试: $BUILD_DIR/design_patterns/state_machine_test"
echo "  - 单例模式测试: $BUILD_DIR/design_patterns/singleton_test"
echo "  - 工厂模式测试: $BUILD_DIR/design_patterns/factory_test"
echo "  - 日志模块测试: $BUILD_DIR/logger/logger_test"
echo ""
echo "运行测试:"
echo "  cd $BUILD_DIR"
echo "  ./thread/thread_poo_test"
echo "  ./design_patterns/pub_sub_test"
echo "  ./design_patterns/observer_test"
echo "  ./design_patterns/chain_test"
echo "  ./design_patterns/async_callback_test"
echo "  ./design_patterns/command_test"
echo "  ./design_patterns/object_pool_test"
echo "  ./design_patterns/strategy_test"
echo "  ./design_patterns/state_machine_test"
echo "  ./design_patterns/singleton_test"
echo "  ./design_patterns/factory_test"
echo "  ./logger/logger_test"

