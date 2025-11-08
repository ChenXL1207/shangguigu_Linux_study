# Makefile 使用教程

## 一、Makefile 简介

Makefile 是一个用于自动化编译 C/C++ 项目的工具文件。它定义了：
- **源文件**：需要编译的 `.c` 文件
- **目标文件**：编译生成的 `.o` 文件
- **可执行文件**：最终生成的可运行程序
- **编译规则**：如何从源文件生成目标文件和可执行文件

## 二、Makefile 基本语法

### 2.1 变量定义

```makefile
CC = gcc                    # 编译器
CFLAGS = -Wall -g           # 编译选项
SRC_DIR = src              # 源代码目录
```

### 2.2 目标和依赖

```makefile
目标: 依赖文件
    <Tab>命令
```

**重要**：命令前必须使用 Tab 键，不能用空格！

### 2.3 常用编译选项说明

- `-Wall`：显示所有警告信息
- `-Wextra`：显示额外警告
- `-g`：生成调试信息（用于 gdb 调试）
- `-O2`：优化级别 2（提升运行速度）
- `-c`：只编译不链接（生成 .o 文件）
- `-o`：指定输出文件名

## 三、项目 Makefile 结构解析

### 3.1 变量定义部分

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -g -O2
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/tcp_echo_server
```

### 3.2 默认目标

```makefile
all: $(TARGET)
```

执行 `make` 或 `make all` 时，会编译生成可执行文件。

### 3.3 编译规则

```makefile
# 链接生成可执行文件
$(TARGET): $(OBJ) | $(BUILD_DIR)
    $(CC) $(OBJ) -o $(TARGET) $(LIBS)

# 编译生成目标文件
$(OBJ): $(SRC) $(HEADERS) | $(BUILD_DIR)
    $(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)
```

**说明**：
- `| $(BUILD_DIR)` 表示顺序依赖，确保目录存在
- `$(CC) $(CFLAGS)` 使用变量展开为实际命令

## 四、使用步骤

### 步骤 1：查看 Makefile

确保 Makefile 文件在项目根目录下：

```bash
cd /home/chenx/Linux_study_shangguigu/linux_shangguigu/project/tcp_udp_server
ls -l Makefile
```

### 步骤 2：编译项目

使用 `make` 命令编译：

```bash
make
```

**输出示例**：
```
创建编译输出目录: build
编译源文件: src/tcp_echo_server.c
链接目标文件...
编译完成！可执行文件：build/tcp_echo_server
```

### 步骤 3：查看编译结果

```bash
ls -l build/
```

应该能看到 `tcp_echo_server` 可执行文件。

### 步骤 4：运行程序

**方法一：直接运行**
```bash
./build/tcp_echo_server
```

**方法二：使用 make run**
```bash
make run
```

### 步骤 5：清理编译文件

```bash
make clean
```

这会删除 `build/` 目录下的所有文件。

### 步骤 6：重新编译

如果需要重新编译（先清理再编译）：

```bash
make rebuild
```

## 五、常用命令总结

| 命令 | 说明 |
|------|------|
| `make` | 编译项目（默认目标） |
| `make all` | 同 `make` |
| `make clean` | 清理编译文件 |
| `make rebuild` | 重新编译（先清理再编译） |
| `make run` | 编译并运行程序 |
| `make help` | 显示帮助信息 |

## 六、编译过程详解

### 6.1 完整的编译流程

1. **预处理**（Preprocessing）
   ```bash
   gcc -E tcp_echo_server.c -o tcp_echo_server.i
   ```
   - 处理 `#include`、`#define` 等预处理指令

2. **编译**（Compilation）
   ```bash
   gcc -c tcp_echo_server.c -o tcp_echo_server.o
   ```
   - 将 C 代码编译成汇编代码，再生成目标文件（.o）

3. **链接**（Linking）
   ```bash
   gcc tcp_echo_server.o -o tcp_echo_server
   ```
   - 将目标文件与库文件链接，生成可执行文件

### 6.2 Makefile 自动执行

Makefile 会自动执行上述步骤：
1. 检查依赖关系
2. 只编译修改过的文件
3. 链接生成最终可执行文件

## 七、常见问题解决

### 问题 1：找不到 common.h

**错误信息**：
```
fatal error: common.h: No such file or directory
```

**解决方法**：
- 确保 `common.h` 在 `src/` 目录下
- 检查 Makefile 中的 `HEADERS` 变量路径是否正确

### 问题 2：命令前缺少 Tab

**错误信息**：
```
Makefile:XX: *** missing separator. Stop.
```

**解决方法**：
- 确保命令前使用 Tab 键，不能用空格
- 检查编辑器是否将 Tab 转换为空格

### 问题 3：权限 denied

**错误信息**：
```
Permission denied
```

**解决方法**：
```bash
chmod +x build/tcp_echo_server
```

### 问题 4：端口被占用

**错误信息**：
```
bind: Address already in use
```

**解决方法**：
```bash
# 查找占用 8888 端口的进程
sudo lsof -i :8888
# 或
sudo netstat -tlnp | grep 8888

# 杀死进程
sudo kill -9 <PID>
```

## 八、Makefile 进阶技巧

### 8.1 添加多个源文件

如果项目有多个源文件：

```makefile
SRCS = $(SRC_DIR)/tcp_echo_server.c \
       $(SRC_DIR)/utils.c \
       $(SRC_DIR)/network.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
```

### 8.2 添加调试信息

```makefile
# 调试版本
debug: CFLAGS += -DDEBUG -g3
debug: $(TARGET)

# 发布版本
release: CFLAGS += -DNDEBUG -O3
release: $(TARGET)
```

### 8.3 添加库依赖

如果需要链接库（如 pthread）：

```makefile
LIBS = -lpthread
```

### 8.4 跨平台编译

```makefile
# 检测操作系统
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS += -lrt
endif
```

## 九、测试 Makefile

### 测试编译
```bash
make clean
make
```

### 测试运行
```bash
# 终端 1：运行服务器
./build/tcp_echo_server

# 终端 2：使用 telnet 测试
telnet localhost 8888
# 输入一些文字，应该会回显
```

### 测试清理
```bash
make clean
ls build/  # 应该为空或不存在
```

## 十、总结

1. **Makefile 的作用**：自动化编译过程，提高开发效率
2. **基本使用**：`make` 编译，`make clean` 清理
3. **依赖关系**：Makefile 会自动处理文件依赖
4. **增量编译**：只编译修改过的文件，节省时间

通过本教程，您应该能够：
- ✅ 理解 Makefile 的基本语法
- ✅ 使用 Makefile 编译 C 项目
- ✅ 解决常见的编译问题
- ✅ 根据需求修改 Makefile

## 参考资料

- [GNU Make 官方文档](https://www.gnu.org/software/make/manual/)
- [Makefile 教程](https://makefiletutorial.com/)
- GCC 编译选项：[GCC Manual](https://gcc.gnu.org/onlinedocs/)

