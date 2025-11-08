# TCP/UDP 回声服务器

## 项目简介

这是一个同时支持 TCP 和 UDP 协议的回声服务器（Echo Server）项目。服务器接收客户端发送的数据，并将数据原样返回给客户端。

## 项目特点

- ✅ 支持 TCP 协议（面向连接，可靠传输）
- ✅ 支持 UDP 协议（无连接，快速响应）
- ✅ 支持多客户端同时连接（TCP）
- ✅ 支持并发处理（UDP）
- ✅ 清晰的日志输出
- ✅ 完善的错误处理

## 快速开始

### 1. 编译项目

```bash
cd project/tcp_udp_server
make
```

### 2. 运行服务器

**TCP 服务器：**
```bash
./build/tcp_echo_server
```

**UDP 服务器：**
```bash
./build/udp_echo_server
```

### 3. 运行客户端测试

**TCP 客户端：**
```bash
./build/tcp_echo_client
```

**UDP 客户端：**
```bash
./build/udp_echo_client
```

## 项目结构

```
tcp_udp_server/
├── src/              # 源代码
├── build/            # 编译输出
├── tests/             # 测试脚本
├── docs/              # 文档
├── Makefile           # 构建脚本
└── README.md          # 项目说明
```

## 技术栈

- **编程语言**：C
- **网络编程**：Socket API
- **多线程**：pthread
- **协议**：TCP、UDP

## 端口配置

- **TCP 端口**：8888
- **UDP 端口**：8889

## 开发文档

详细的技术路线文档请参考：
- [技术路线文档.md](技术路线文档.md)

## 测试

运行测试脚本：
```bash
./tests/test_tcp.sh
./tests/test_udp.sh
./tests/test_all.sh
```

## 许可证

本项目用于学习目的。

## 作者

基于 Linux 应用层开发学习路线开发

