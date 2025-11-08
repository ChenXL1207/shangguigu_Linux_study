# epoll 测试指南

## 测试准备

### 1. 编译程序

```bash
cd chapter07
make
```

确保生成了以下文件：
- `build/epoll_test.out` - epoll 服务器
- `build/daemon_client.out` - 客户端程序

## 测试方法

### 方法一：使用客户端程序测试（推荐）

#### 步骤 1：启动服务器

在终端1中运行：

```bash
cd chapter07
./build/epoll_test.out
```

**预期输出：**
```
epoll服务器启动, 监听端口: 8888
```

#### 步骤 2：运行客户端

在终端2中运行：

```bash
cd chapter07
./build/daemon_client.out
```

**预期输出：**
```
已连接到守护进程服务器 127.0.0.1:8888
输入 'quit' 退出，输入 'EOF' 退出
请输入消息: 
```

#### 步骤 3：发送消息测试

在客户端终端输入消息，例如：

```
请输入消息: Hello Server
发送: Hello Server (12 字节)
接收: Hello Server
请输入消息: 测试epoll
发送: 测试epoll (10 字节)
接收: 测试epoll
请输入消息: quit
退出客户端
```

**服务器端应该显示：**
```
新客户端连接: 127.0.0.1:54321, fd=4
收到fd=4数据: Hello Server
收到fd=4数据: 测试epoll
客户端关闭: fd=4
```

### 方法二：使用 telnet 测试

#### 步骤 1：启动服务器

```bash
cd chapter07
./build/epoll_test.out
```

#### 步骤 2：使用 telnet 连接

在另一个终端：

```bash
telnet 127.0.0.1 8888
```

#### 步骤 3：发送消息

在 telnet 中输入消息，按回车发送。服务器会回显消息。

**退出 telnet：** 按 `Ctrl+]`，然后输入 `quit`

### 方法三：使用 nc (netcat) 测试

#### 步骤 1：启动服务器

```bash
cd chapter07
./build/epoll_test.out
```

#### 步骤 2：使用 nc 连接

```bash
nc 127.0.0.1 8888
```

#### 步骤 3：发送消息

输入消息，服务器会回显。

**退出 nc：** 按 `Ctrl+C`

### 方法四：多客户端并发测试

#### 步骤 1：启动服务器

```bash
cd chapter07
./build/epoll_test.out
```

#### 步骤 2：打开多个终端，同时运行客户端

**终端2：**
```bash
cd chapter07
./build/daemon_client.out
```

**终端3：**
```bash
cd chapter07
./build/daemon_client.out
```

**终端4：**
```bash
cd chapter07
./build/daemon_client.out
```

#### 步骤 3：在不同客户端发送消息

观察服务器是否能同时处理多个客户端连接。

**预期结果：**
- 服务器能同时接受多个连接
- 每个客户端发送的消息都能被正确处理
- 服务器显示不同 fd 的消息

## 测试脚本

### 自动化测试脚本

创建一个测试脚本 `test_epoll.sh`：

```bash
#!/bin/bash

echo "=== epoll 服务器测试 ==="
echo ""

# 检查服务器是否在运行
if pgrep -f "epoll_test.out" > /dev/null; then
    echo "警告: 服务器已经在运行"
    pkill -f "epoll_test.out"
    sleep 1
fi

# 启动服务器（后台运行）
echo "1. 启动服务器..."
cd chapter07
./build/epoll_test.out &
SERVER_PID=$!
sleep 1

# 检查服务器是否启动成功
if ! ps -p $SERVER_PID > /dev/null; then
    echo "错误: 服务器启动失败"
    exit 1
fi

echo "服务器已启动 (PID: $SERVER_PID)"
echo ""

# 测试1: 单个客户端连接
echo "2. 测试单个客户端连接..."
echo "test message 1" | timeout 2 ./build/daemon_client.out 2>/dev/null || echo "客户端测试完成"
echo ""

# 测试2: 多个客户端连接
echo "3. 测试多个客户端连接..."
for i in {1..3}; do
    echo "client $i message" | timeout 1 ./build/daemon_client.out 2>/dev/null &
done
wait
echo ""

# 测试3: 使用 nc 测试
echo "4. 使用 nc 测试..."
echo "nc test message" | timeout 1 nc 127.0.0.1 8888 2>/dev/null || echo "nc 测试完成"
echo ""

# 停止服务器
echo "5. 停止服务器..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
echo "测试完成"
```

## 验证要点

### 1. 基本功能验证

- [ ] 服务器能正常启动
- [ ] 客户端能成功连接
- [ ] 消息能正确发送和接收
- [ ] 服务器能正确回显消息

### 2. 并发连接验证

- [ ] 能同时处理多个客户端连接
- [ ] 每个连接的消息都能正确处理
- [ ] 不同客户端的消息不会混淆

### 3. 边缘触发验证

- [ ] 使用边缘触发模式（EPOLLET）
- [ ] 非阻塞 IO 正常工作
- [ ] 数据能一次性读取完

### 4. 连接管理验证

- [ ] 客户端断开连接时，服务器能正确清理
- [ ] 从 epoll 中正确移除已关闭的文件描述符
- [ ] 没有文件描述符泄漏

## 常见问题排查

### 问题1：端口被占用

**错误信息：**
```
bind: Address already in use
```

**解决方法：**
```bash
# 查找占用端口的进程
lsof -i :8888
# 或
netstat -tlnp | grep 8888

# 杀死进程
kill -9 <PID>
```

### 问题2：客户端连接失败

**错误信息：**
```
connect: Connection refused
```

**解决方法：**
- 确保服务器已启动
- 检查端口号是否正确（8888）
- 检查防火墙设置

### 问题3：消息发送失败

**可能原因：**
- 客户端已断开连接
- 网络问题
- 服务器处理错误

**解决方法：**
- 检查服务器日志
- 重新连接客户端
- 检查网络连接

### 问题4：服务器无响应

**可能原因：**
- 服务器崩溃
- 死锁
- 资源耗尽

**解决方法：**
```bash
# 检查服务器进程
ps aux | grep epoll_test

# 查看系统资源
top
free -h

# 重启服务器
pkill epoll_test.out
./build/epoll_test.out
```

## 性能测试

### 测试并发连接数

使用压力测试工具测试服务器能处理多少并发连接：

```bash
# 使用 ab (Apache Bench) 测试
ab -n 1000 -c 10 http://127.0.0.1:8888/

# 使用 wrk 测试
wrk -t4 -c100 -d30s http://127.0.0.1:8888/
```

### 监控资源使用

```bash
# 监控 CPU 和内存
top -p $(pgrep epoll_test.out)

# 监控网络连接
watch -n 1 'netstat -an | grep 8888 | wc -l'
```

## 测试检查清单

- [ ] 服务器能正常启动
- [ ] 单个客户端连接正常
- [ ] 消息发送和接收正常
- [ ] 多个客户端能同时连接
- [ ] 每个客户端消息独立处理
- [ ] 客户端断开连接正常
- [ ] 服务器资源正确释放
- [ ] 边缘触发模式正常工作
- [ ] 非阻塞 IO 正常工作
- [ ] 无内存泄漏
- [ ] 无文件描述符泄漏

## 下一步学习

测试完成后，可以：

1. **修改代码**：尝试不同的 epoll 模式（水平触发 vs 边缘触发）
2. **性能优化**：优化代码，提高性能
3. **功能扩展**：添加更多功能（如心跳检测、超时处理等）
4. **学习其他 IO 多路复用**：对比 select、poll、epoll 的性能

## 参考命令

```bash
# 快速测试命令
cd chapter07

# 终端1：启动服务器
./build/epoll_test.out

# 终端2：运行客户端
./build/daemon_client.out

# 终端3：使用 telnet 测试
telnet 127.0.0.1 8888

# 终端4：使用 nc 测试
nc 127.0.0.1 8888
```

