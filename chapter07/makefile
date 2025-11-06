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
$(if $(or $(findstring pthread,$(1)),$(findstring thread,$(1))),-lpthread )$(if $(or $(findstring shared_memory,$(1)),$(findstring mq,$(1)),$(findstring product,$(1)),$(findstring consumer,$(1)),$(findstring process,$(1))),-lrt )
endef

# 检查文件内容是否包含pthread函数调用或信号量函数
define check_pthread_in_file
$(shell grep -q "pthread_create\|pthread_join\|pthread_exit\|pthread_cancel\|pthread_detach\|pthread_testcancel\|sem_init\|sem_post\|sem_wait\|sem_destroy" $(1) && echo "-lpthread")
endef

# 检查文件内容是否包含GLib函数调用
define check_glib_in_file
$(shell grep -q "g_thread_pool\|g_malloc\|g_free\|g_print\|g_error\|g_warning\|g_message\|g_debug\|g_log\|g_assert\|g_return_if_fail\|g_return_val_if_fail" $(1) && echo "-lglib-2.0")
endef

# # 检查文件内容是否包含rt库函数调用（进程间通信相关）
# define check_rt_in_file
# $(shell grep -q "shmget\|shmat\|shmdt\|mq_open\|mq_send\|mq_receive\|shm_open\|shm_unlink\|mmap\|munmap\|ftruncate" $(1) && echo "-lrt")
# endef

# 通用规则：为每个源文件生成独立的可执行文件，自动检测并添加链接库
$(BUILD_DIR)/%.out: $(SRC_DIR)/%.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	@NAME_LIBS=$$(echo "$(call get_libs,$*)"); \
	CONTENT_LIBS=$$(echo "$(call check_pthread_in_file,$<) $(call check_rt_in_file,$<) $(call check_glib_in_file,$<)"); \
	ALL_LIBS=$$(echo "$$NAME_LIBS $$CONTENT_LIBS" | tr -s ' ' | tr ' ' '\n' | sort -u | grep -v '^$$' | tr '\n' ' ' | sed 's/ $$//'); \
	GLIB_CFLAGS=$$(pkg-config --cflags glib-2.0 2>/dev/null || echo ""); \
	if [ -n "$$ALL_LIBS" ]; then \
		echo "  linking with: $$ALL_LIBS"; \
		$(CC) $(CFLAGS) $$GLIB_CFLAGS -o $@ $< $$ALL_LIBS; \
	else \
		$(CC) $(CFLAGS) $$GLIB_CFLAGS -o $@ $<; \
	fi

# 清理 build 目录:
clean:
	@echo "clean..."
	rm -rf $(BUILD_DIR)

# 引入所有 .d 文件:
-include $(DEPS)