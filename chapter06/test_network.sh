#!/bin/bash

# 网络编程测试脚本
echo "=== 网络编程测试脚本 ==="

# 检查可执行文件是否存在
if [ ! -f "./build/single_conn_server.out" ]; then
    echo "错误: 服务器可执行文件不存在，请先运行 make"
    exit 1
fi

if [ ! -f "./build/single_conn_client.out" ]; then
    echo "错误: 客户端可执行文件不存在，请先运行 make"
    exit 1
fi

echo "1. 启动服务器..."
./build/single_conn_server.out &
SERVER_PID=$!

# 等待服务器启动
echo "2. 等待服务器启动..."
sleep 3

# 检查服务器是否正在运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "错误: 服务器启动失败"
    exit 1
fi

echo "3. 服务器已启动 (PID: $SERVER_PID)"
echo "4. 启动客户端进行测试..."

# 创建测试输入文件
echo "Hello from client!" > test_input.txt
echo "This is a test message" >> test_input.txt
echo "quit" >> test_input.txt

# 运行客户端（使用输入重定向）
timeout 10 ./build/single_conn_client.out < test_input.txt

echo "5. 测试完成，清理进程..."
kill $SERVER_PID 2>/dev/null
rm -f test_input.txt

echo "=== 测试结束 ==="
