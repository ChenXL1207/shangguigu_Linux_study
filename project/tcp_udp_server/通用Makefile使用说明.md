# 通用 Makefile 使用说明

## 🎯 特点

这是一个**通用的 Makefile**，具有以下特点：

1. **自动发现**：自动查找 `src/` 目录下的所有 `.c` 文件
2. **自动编译**：为每个 `.c` 文件自动生成对应的可执行文件
3. **无需修改**：添加新的 `.c` 文件后，无需修改 Makefile
4. **灵活使用**：支持编译所有程序或单独编译某个程序

## 📋 使用方法

### 基本命令

```bash
# 编译所有程序
make

# 编译指定程序（例如：tcp_echo_server）
make tcp_echo_server

# 运行指定程序
make run-tcp_echo_server

# 列出所有可编译的程序
make list

# 清理编译文件
make clean

# 重新编译
make rebuild

# 显示帮助信息
make help
```

### 实际示例

假设 `src/` 目录下有：
- `tcp_echo_server.c`
- `tcp_echo_client.c`
- `my_program.c`  （新添加的文件）

```bash
# 1. 编译所有程序（包括新添加的 my_program.c）
make

# 2. 只编译 server
make tcp_echo_server

# 3. 只编译新添加的程序
make my_program

# 4. 运行新添加的程序
make run-my_program

# 5. 查看所有可编译的程序
make list
```

## 🔧 工作原理

### 自动发现机制

Makefile 使用以下函数自动发现源文件：

```makefile
# 获取所有 .c 文件
SRCS = $(wildcard $(SRC_DIR)/*.c)

# 生成目标文件名（去掉路径和扩展名）
TARGETS = $(patsubst $(SRC_DIR)/%.c,%,$(SRCS))

# 生成可执行文件路径
EXECUTABLES = $(patsubst %,$(BUILD_DIR)/%,$(TARGETS))
```

### 模式规则

使用模式规则自动处理所有文件：

```makefile
# 编译 .c 文件为 .o 文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 链接 .o 文件为可执行文件
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CC) $< -o $@ $(LIBS)
```

## 📁 项目结构

```
project/
├── Makefile          # 通用 Makefile（不需要修改）
├── src/              # 源代码目录
│   ├── common.h      # 公共头文件
│   ├── program1.c    # 程序1
│   ├── program2.c    # 程序2
│   └── program3.c    # 程序3（新添加，自动识别）
└── build/            # 编译输出目录（自动创建）
    ├── program1      # 可执行文件
    ├── program1.o    # 目标文件
    ├── program2      # 可执行文件
    ├── program2.o    # 目标文件
    ├── program3      # 可执行文件
    └── program3.o    # 目标文件
```

## 🚀 添加新程序的步骤

### 步骤 1：创建源文件

在 `src/` 目录下创建新的 `.c` 文件：

```bash
# 创建新文件
vim src/my_new_program.c
```

### 步骤 2：编写代码

```c
#include "common.h"

int main(int argc, char const *argv[]) {
    // 你的代码
    printf("Hello, World!\n");
    return 0;
}
```

### 步骤 3：编译和运行

```bash
# 编译所有程序（包括新添加的）
make

# 或只编译新程序
make my_new_program

# 运行新程序
make run-my_new_program
```

**就这么简单！不需要修改 Makefile！**

## ⚙️ 自定义配置

如果需要修改配置，可以编辑 Makefile 中的以下变量：

### 编译器配置

```makefile
CC = gcc                    # 编译器
CFLAGS = -Wall -Wextra -g -O2  # 编译选项
```

### 目录配置

```makefile
SRC_DIR = src              # 源代码目录
BUILD_DIR = build          # 编译输出目录
```

### 链接库

如果需要链接额外的库（如 pthread）：

```makefile
LIBS = -lpthread -lm
```

## 📊 命令对照表

| 命令 | 说明 | 示例 |
|------|------|------|
| `make` | 编译所有程序 | `make` |
| `make <程序名>` | 编译指定程序 | `make tcp_echo_server` |
| `make run-<程序名>` | 运行指定程序 | `make run-tcp_echo_server` |
| `make list` | 列出所有程序 | `make list` |
| `make clean` | 清理编译文件 | `make clean` |
| `make rebuild` | 重新编译 | `make rebuild` |
| `make help` | 显示帮助 | `make help` |

## 🔍 高级用法

### 1. 只编译修改过的文件

Makefile 会自动检测文件修改，只重新编译修改过的文件：

```bash
# 修改了 tcp_echo_server.c 后
make  # 只会重新编译 tcp_echo_server，不会重新编译其他文件
```

### 2. 并行编译（加速编译）

使用 `-j` 选项可以并行编译多个文件：

```bash
make -j4  # 使用4个并行任务编译
```

### 3. 查看编译命令

使用 `-n` 选项可以查看将要执行的命令，而不实际执行：

```bash
make -n  # 显示将要执行的命令
```

## ❓ 常见问题

### Q1: 如何排除某些文件不被编译？

如果某些 `.c` 文件不想被编译，可以：

1. **重命名文件**：将文件扩展名改为 `.c.bak` 或其他
2. **移动到其他目录**：将文件移到 `src/` 目录外
3. **修改 Makefile**：添加排除规则（需要修改 Makefile）

### Q2: 如何为不同程序使用不同的编译选项？

如果需要为不同程序使用不同的编译选项，可以在源文件中使用条件编译，或者修改 Makefile 添加特定规则。

### Q3: 如何添加子目录支持？

当前 Makefile 只支持 `src/` 目录下的文件。如果需要支持子目录，需要修改 `wildcard` 函数：

```makefile
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c)
```

### Q4: 编译后的文件在哪里？

所有编译生成的文件都在 `build/` 目录下：
- 可执行文件：`build/<程序名>`
- 目标文件：`build/<程序名>.o`

## 📚 总结

这个通用 Makefile 的优势：

✅ **自动化**：自动发现和编译所有 `.c` 文件  
✅ **灵活性**：支持单独编译或编译所有程序  
✅ **易用性**：添加新文件无需修改 Makefile  
✅ **高效性**：只编译修改过的文件  
✅ **可维护性**：清晰的代码结构和注释  

**现在，你只需要专注于编写 C 代码，编译的事情交给 Makefile！**

