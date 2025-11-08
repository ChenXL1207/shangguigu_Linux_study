#!/bin/bash

# epoll 服务器测试脚本
# 用法: ./test_epoll.sh [选项]

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PARENT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SERVER_PATH="$PARENT_DIR/build/epoll_test.out"
CLIENT_PATH="$PARENT_DIR/build/daemon_client.out"
PORT=8888

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_test() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

# 检查文件是否存在
check_files() {
    if [ ! -f "$SERVER_PATH" ]; then
        print_error "服务器文件不存在: $SERVER_PATH"
        print_info "请先编译: cd chapter07 && make"
        exit 1
    fi

    if [ ! -f "$CLIENT_PATH" ]; then
        print_error "客户端文件不存在: $CLIENT_PATH"
        print_info "请先编译: cd chapter07 && make"
        exit 1
    fi
}

# 检查端口是否被占用
check_port() {
    if lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1 ; then
        print_warning "端口 $PORT 已被占用"
        print_info "正在停止占用端口的进程..."
        lsof -ti:$PORT | xargs kill -9 2>/dev/null
        sleep 1
    fi
}

# 启动服务器
start_server() {
    print_info "启动 epoll 服务器..."
    cd "$PARENT_DIR"
    "$SERVER_PATH" &
    SERVER_PID=$!
    sleep 1

    if ps -p $SERVER_PID > /dev/null 2>&1; then
        print_info "服务器已启动 (PID: $SERVER_PID)"
        return 0
    else
        print_error "服务器启动失败"
        return 1
    fi
}

# 停止服务器
stop_server() {
    if [ -n "$SERVER_PID" ] && ps -p $SERVER_PID > /dev/null 2>&1; then
        print_info "停止服务器 (PID: $SERVER_PID)..."
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
        print_info "服务器已停止"
    fi
}

# 测试1: 单个客户端连接
test_single_client() {
    print_test "测试1: 单个客户端连接"
    cd "$PARENT_DIR"
    
    # 使用 echo 发送消息，通过管道传递给客户端
    echo "test message 1" | timeout 2 "$CLIENT_PATH" 2>/dev/null
    if [ $? -eq 0 ] || [ $? -eq 124 ]; then
        print_info "单个客户端测试完成"
    else
        print_warning "单个客户端测试可能有问题"
    fi
    echo ""
}

# 测试2: 多个客户端并发连接
test_multiple_clients() {
    print_test "测试2: 多个客户端并发连接"
    cd "$PARENT_DIR"
    
    for i in {1..3}; do
        (echo "client $i message" | timeout 1 "$CLIENT_PATH" 2>/dev/null) &
    done
    wait
    print_info "多个客户端测试完成"
    echo ""
}

# 测试3: 使用 nc 测试
test_with_nc() {
    print_test "测试3: 使用 nc (netcat) 测试"
    
    if command -v nc >/dev/null 2>&1; then
        echo "nc test message" | timeout 1 nc 127.0.0.1 $PORT 2>/dev/null
        print_info "nc 测试完成"
    else
        print_warning "nc 未安装，跳过此测试"
    fi
    echo ""
}

# 测试4: 使用 telnet 测试
test_with_telnet() {
    print_test "测试4: 使用 telnet 测试"
    
    if command -v telnet >/dev/null 2>&1; then
        (echo "telnet test"; sleep 1) | timeout 2 telnet 127.0.0.1 $PORT 2>/dev/null
        print_info "telnet 测试完成"
    else
        print_warning "telnet 未安装，跳过此测试"
    fi
    echo ""
}

# 测试5: 连接数测试
test_connections() {
    print_test "测试5: 连接数测试"
    cd "$PARENT_DIR"
    
    print_info "创建 5 个并发连接..."
    for i in {1..5}; do
        (echo "connection $i" | timeout 1 "$CLIENT_PATH" 2>/dev/null) &
    done
    wait
    print_info "连接数测试完成"
    echo ""
}

# 交互式测试
interactive_test() {
    print_test "交互式测试模式"
    print_info "服务器已在后台运行"
    print_info "请手动运行客户端进行测试:"
    echo ""
    echo "  cd chapter07"
    echo "  ./build/daemon_client.out"
    echo ""
    print_info "或者使用 telnet:"
    echo "  telnet 127.0.0.1 $PORT"
    echo ""
    print_info "按任意键停止服务器并退出..."
    read -n 1 -s
}

# 主函数
main() {
    echo "=========================================="
    echo "      epoll 服务器测试脚本"
    echo "=========================================="
    echo ""

    # 检查文件
    check_files

    # 检查端口
    check_port

    # 启动服务器
    if ! start_server; then
        exit 1
    fi

    # 捕获退出信号
    trap stop_server EXIT INT TERM

    # 根据参数选择测试模式
    case "${1:-all}" in
        single)
            test_single_client
            ;;
        multiple)
            test_multiple_clients
            ;;
        nc)
            test_with_nc
            ;;
        telnet)
            test_with_telnet
            ;;
        connections)
            test_connections
            ;;
        interactive)
            interactive_test
            ;;
        all)
            test_single_client
            test_multiple_clients
            test_with_nc
            test_with_telnet
            test_connections
            ;;
        *)
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  single      - 单个客户端测试"
            echo "  multiple    - 多个客户端测试"
            echo "  nc          - 使用 nc 测试"
            echo "  telnet      - 使用 telnet 测试"
            echo "  connections - 连接数测试"
            echo "  interactive - 交互式测试模式"
            echo "  all         - 运行所有测试（默认）"
            exit 1
            ;;
    esac

    print_info "所有测试完成"
}

# 执行主函数
main "$@"

