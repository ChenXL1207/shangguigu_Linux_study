SRC_DIR = ./src
BUILD_DIR = ./build

CC = cc
CFLAGS = -Wall

# ./src/*.c
SRCS = $(shell find $(SRC_DIR) -name '*.c')
# ./src/*.c => ./build/*.o
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
# ./src/*.c => ./build/*.d
DEPS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRCS))
# ./src/*.c => ./build/*.out (可执行文件)
TARGETS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.out,$(SRCS))

# 默认目标: 编译所有可执行文件
all: $(TARGETS)

# build/xyz.d 的规则由 src/xyz.c 生成:
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@); \
	rm -f $@; \
	$(CC) -MM $< >$@.tmp; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.o $@ : ,g' < $@.tmp > $@; \
	rm -f $@.tmp

# build/xyz.o 的规则由 src/xyz.c 生成:
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# 智能链接库检测函数
# 检查文件名是否包含特定关键词，并返回相应的链接库
define get_libs
$(if $(or $(findstring pthread,$(1)),$(findstring thread,$(1))),-lpthread)$(if $(or $(findstring shared_memory,$(1)),$(findstring mq,$(1)),$(findstring product,$(1)),$(findstring consumer,$(1))),-lrt)
endef

# 检查文件内容是否包含pthread函数调用
define check_pthread_in_file
$(shell grep -q "pthread_create\|pthread_join\|pthread_exit\|pthread_cancel\|pthread_detach\|pthread_testcancel" $(1) && echo "-lpthread")
endef

# 检查文件内容是否包含rt库函数调用
define check_rt_in_file
$(shell grep -q "shmget\|shmat\|shmdt\|mq_open\|mq_send\|mq_receive" $(1) && echo "-lrt")
endef

# 通用规则：为每个源文件生成独立的可执行文件，自动检测并添加链接库
$(BUILD_DIR)/%.out: $(SRC_DIR)/%.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	@NAME_LIBS=$$(echo "$(call get_libs,$*)" | tr -d ' '); \
	CONTENT_LIBS=$$(echo "$(call check_pthread_in_file,$<) $(call check_rt_in_file,$<)" | tr -d ' '); \
	ALL_LIBS=$$(echo "$$NAME_LIBS $$CONTENT_LIBS" | tr -s ' ' | tr ' ' '\n' | sort -u | tr '\n' ' ' | sed 's/ $$//'); \
	if [ -n "$$ALL_LIBS" ]; then \
		echo "  linking with: $$ALL_LIBS"; \
		$(CC) $(CFLAGS) -o $@ $< $$ALL_LIBS; \
	else \
		$(CC) $(CFLAGS) -o $@ $<; \
	fi

# 清理 build 目录:
clean:
	@echo "clean..."
	rm -rf $(BUILD_DIR)

# 引入所有 .d 文件:
-include $(DEPS)