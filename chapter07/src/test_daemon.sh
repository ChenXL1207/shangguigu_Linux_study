#!/bin/bash

# 守护进程测试脚本
# 用法: ./test_daemon.sh [start|stop|status|test|restart]

DAEMON_NAME="daemon_server"
# 获取脚本所在目录的父目录（chapter07目录）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PARENT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DAEMON_PATH="$PARENT_DIR/build/daemon_server.out"
CLIENT_PATH="$PARENT_DIR/build/daemon_client.out"
PID_FILE="/tmp/daemon_server.pid"
LOG_FILE="/tmp/daemon_server.log"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 检查守护进程是否运行
is_running() {
    if [ -f "$PID_FILE" ]; then
        PID=$(cat "$PID_FILE")
        if ps -p $PID > /dev/null 2>&1; then
            return 0
        else
            rm -f "$PID_FILE"
            return 1
        fi
    else
        return 1
    fi
}

# 启动守护进程
start_daemon() {
    if is_running; then
        print_warning "守护进程已经在运行中 (PID: $(cat $PID_FILE))"
        return 1
    fi

    if [ ! -f "$DAEMON_PATH" ]; then
        print_error "守护进程可执行文件不存在: $DAEMON_PATH"
        print_info "请先编译: make"
        return 1
    fi

    print_info "启动守护进程..."
    $DAEMON_PATH

    sleep 1

    if is_running; then
        print_info "守护进程启动成功 (PID: $(cat $PID_FILE))"
        print_info "日志文件: $LOG_FILE"
        return 0
    else
        print_error "守护进程启动失败"
        return 1
    fi
}

# 停止守护进程
stop_daemon() {
    if ! is_running; then
        print_warning "守护进程没有运行"
        return 1
    fi

    PID=$(cat "$PID_FILE")
    print_info "停止守护进程 (PID: $PID)..."

    # 发送 SIGTERM 信号
    kill -TERM $PID 2>/dev/null

    # 等待进程退出
    for i in {1..10}; do
        if ! ps -p $PID > /dev/null 2>&1; then
            print_info "守护进程已停止"
            rm -f "$PID_FILE"
            return 0
        fi
        sleep 1
    done

    # 如果还在运行，强制终止
    if ps -p $PID > /dev/null 2>&1; then
        print_warning "守护进程没有响应，强制终止..."
        kill -9 $PID 2>/dev/null
        rm -f "$PID_FILE"
        print_info "守护进程已强制终止"
    fi

    return 0
}

# 查看守护进程状态
status_daemon() {
    if is_running; then
        PID=$(cat "$PID_FILE")
        print_info "守护进程正在运行"
        print_info "PID: $PID"
        print_info "日志文件: $LOG_FILE"
        
        # 显示进程信息
        echo ""
        print_info "进程信息:"
        ps -p $PID -o pid,ppid,cmd,etime
        
        # 显示最近日志
        if [ -f "$LOG_FILE" ]; then
            echo ""
            print_info "最近日志 (最后10行):"
            tail -10 "$LOG_FILE"
        fi
        return 0
    else
        print_warning "守护进程没有运行"
        return 1
    fi
}

# 测试守护进程
test_daemon() {
    if ! is_running; then
        print_error "守护进程没有运行，请先启动守护进程"
        print_info "运行: $0 start"
        return 1
    fi

    if [ ! -f "$CLIENT_PATH" ]; then
        print_error "客户端可执行文件不存在: $CLIENT_PATH"
        print_info "请先编译: make"
        return 1
    fi

    print_info "测试守护进程..."
    print_info "启动客户端连接到守护进程..."
    echo ""

    # 运行客户端
    $CLIENT_PATH

    echo ""
    print_info "测试完成"
}

# 重新加载配置（发送 SIGHUP 信号）
reload_daemon() {
    if ! is_running; then
        print_error "守护进程没有运行"
        return 1
    fi

    PID=$(cat "$PID_FILE")
    print_info "重新加载配置 (PID: $PID)..."
    kill -HUP $PID 2>/dev/null
    
    if [ $? -eq 0 ]; then
        print_info "已发送 SIGHUP 信号"
    else
        print_error "发送信号失败"
        return 1
    fi
}

# 重启守护进程
restart_daemon() {
    print_info "重启守护进程..."
    stop_daemon
    sleep 1
    start_daemon
}

# 查看日志
view_log() {
    if [ -f "$LOG_FILE" ]; then
        if [ "$1" == "-f" ] || [ "$1" == "--follow" ]; then
            print_info "实时查看日志 (按 Ctrl+C 退出)..."
            tail -f "$LOG_FILE"
        else
            print_info "查看日志文件:"
            cat "$LOG_FILE"
        fi
    else
        print_warning "日志文件不存在: $LOG_FILE"
    fi
}

# 主函数
main() {
    case "$1" in
        start)
            start_daemon
            ;;
        stop)
            stop_daemon
            ;;
        status)
            status_daemon
            ;;
        test)
            test_daemon
            ;;
        restart)
            restart_daemon
            ;;
        reload)
            reload_daemon
            ;;
        log)
            view_log "$2"
            ;;
        *)
            echo "用法: $0 {start|stop|status|test|restart|reload|log}"
            echo ""
            echo "命令说明:"
            echo "  start   - 启动守护进程"
            echo "  stop    - 停止守护进程"
            echo "  status  - 查看守护进程状态"
            echo "  test    - 测试守护进程（运行客户端）"
            echo "  restart - 重启守护进程"
            echo "  reload  - 重新加载配置（发送 SIGHUP 信号）"
            echo "  log     - 查看日志文件（可使用 -f 参数实时查看）"
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"

