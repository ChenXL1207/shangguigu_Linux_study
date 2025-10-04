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

# 为每个源文件生成独立的可执行文件:
$(BUILD_DIR)/%.out: $(SRC_DIR)/%.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

# 特殊规则：shared_memory 需要链接 -lrt
$(BUILD_DIR)/shared_memory.out: $(SRC_DIR)/shared_memory.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $< -lrt

# 特殊规则：father_son_mq_test 需要链接 -lrt
$(BUILD_DIR)/father_son_mq_test.out: $(SRC_DIR)/father_son_mq_test.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $< -lrt

# 特殊规则：product 需要链接 -lrt
$(BUILD_DIR)/product.out: $(SRC_DIR)/product.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $< -lrt

# 特殊规则：consumer 需要链接 -lrt
$(BUILD_DIR)/consumer.out: $(SRC_DIR)/consumer.c
	@echo "building $@..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $< -lrt

# 清理 build 目录:
clean:
	@echo "clean..."
	rm -rf $(BUILD_DIR)

# 引入所有 .d 文件:
-include $(DEPS)
