# 通用 Makefile 快速使用指南

## 🎯 特点

这是一个**通用的 Makefile**，会自动发现 `src/` 目录下的所有 `.c` 文件并编译。
**添加新的 C 文件后，无需修改 Makefile！**

## 📋 快速开始

### 1. 编译所有程序
```bash
make
```

**输出示例**：
```
创建编译输出目录: build
编译: src/tcp_echo_client.c -> build/tcp_echo_client.o
链接: build/tcp_echo_client.o -> build/tcp_echo_client
编译完成: build/tcp_echo_client
编译: src/tcp_echo_server.c -> build/tcp_echo_server.o
链接: build/tcp_echo_server.o -> build/tcp_echo_server
编译完成: build/tcp_echo_server
==========================================
所有项目编译完成！
可执行文件：
  - build/tcp_echo_client
  - build/tcp_echo_server
==========================================
```

### 2. 编译指定程序
```bash
make tcp_echo_server    # 只编译 server
make tcp_echo_client    # 只编译 client
```

### 3. 运行程序

**运行 Server**：
```bash
# 方法一：直接运行
./build/tcp_echo_server

# 方法二：使用 make run-<程序名>
make run-tcp_echo_server
```

**运行 Client**：
```bash
# 方法一：直接运行
./build/tcp_echo_client

# 方法二：使用 make run-<程序名>
make run-tcp_echo_client
```

### 4. 列出所有可编译的程序
```bash
make list
```

### 5. 清理编译文件
```bash
make clean
```

### 6. 重新编译
```bash
make rebuild
```

### 7. 显示帮助信息
```bash
make help
```

## 📝 常用命令

| 命令 | 说明 | 示例 |
|------|------|------|
| `make` | 编译所有程序 | `make` |
| `make <程序名>` | 编译指定程序 | `make tcp_echo_server` |
| `make run-<程序名>` | 运行指定程序 | `make run-tcp_echo_server` |
| `make list` | 列出所有程序 | `make list` |
| `make clean` | 清理编译文件 | `make clean` |
| `make rebuild` | 重新编译 | `make rebuild` |
| `make help` | 显示帮助 | `make help` |

## 🆕 添加新程序

### 步骤 1：创建新的 C 文件

在 `src/` 目录下创建新文件：

```bash
vim src/my_program.c
```

### 步骤 2：编写代码

```c
#include "common.h"

int main(int argc, char const *argv[]) {
    printf("Hello from my_program!\n");
    return 0;
}
```

### 步骤 3：编译和运行

```bash
# 自动识别并编译新文件
make

# 或只编译新程序
make my_program

# 运行新程序
make run-my_program
```

**就这么简单！无需修改 Makefile！**

## 🔍 编译输出

编译成功后，会在 `build/` 目录下生成：

**每个程序都会生成**：
- `<程序名>.o` - 目标文件
- `<程序名>` - 可执行文件

例如：
- `build/tcp_echo_server.o` 和 `build/tcp_echo_server`
- `build/tcp_echo_client.o` 和 `build/tcp_echo_client`

## 🧪 测试 Server 和 Client

### 方法一：使用两个终端

**终端 1 - 运行 Server**：
```bash
make run-server
# 或
./build/tcp_echo_server
```

**终端 2 - 运行 Client**：
```bash
make run-client
# 或
./build/tcp_echo_client
```

### 方法二：使用 telnet 测试 Server

```bash
# 运行 server
./build/tcp_echo_server

# 在另一个终端使用 telnet 连接
telnet localhost 8888
```

## ⚠️ 注意事项

1. **编译警告**：当前代码有一些编译警告（不影响运行）：
   - `accept` 函数的参数类型警告（server）
   - 未使用的参数警告（server 和 client）

2. **端口占用**：如果运行时报 "Address already in use"，说明 8888 端口被占用：
   ```bash
   # 查找占用端口的进程
   sudo lsof -i :8888
   # 或
   sudo netstat -tlnp | grep 8888
   
   # 杀死进程
   sudo kill -9 <PID>
   ```

3. **权限问题**：如果无法运行，检查文件权限：
   ```bash
   chmod +x build/tcp_echo_server
   chmod +x build/tcp_echo_client
   ```

4. **Client 连接问题**：确保 Server 先运行，Client 才能连接成功。

## 💡 工作原理

Makefile 使用以下机制自动工作：

1. **自动发现**：使用 `wildcard` 函数查找所有 `.c` 文件
2. **模式规则**：使用 `%` 通配符处理所有文件
3. **依赖检查**：只编译修改过的文件

## 📚 更多信息

- **详细教程**：`Makefile使用教程.md`
- **通用 Makefile 说明**：`通用Makefile使用说明.md`

