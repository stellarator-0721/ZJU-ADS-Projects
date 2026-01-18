# =====================================
# Recover Design Project
# Windows MinGW 兼容版
# =====================================

CC = gcc
CFLAGS = -O2 -std=c11 -Wall -Iinclude

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
DATA_DIR = data
RESULT_DIR = result

TARGET = $(BIN_DIR)\recover_design.exe
SRC = $(SRC_DIR)\recover_design.c
OBJ = $(BUILD_DIR)\recover_design.o
CFLAGS = -O2 -std=c11 -Wall -Wl,--stack,8388608

# Windows mkdir 语法
MKDIR_P = if not exist $(1) mkdir $(1)

# =====================================
# 主规则
# =====================================

all: dirs $(TARGET)
	@echo.
	@echo ====== Running All Test Cases ======
	@$(TARGET) < $(DATA_DIR)\sample1_in.txt > $(RESULT_DIR)\sample1_out.txt
	@$(TARGET) < $(DATA_DIR)\sample2_in.txt > $(RESULT_DIR)\sample2_out.txt
	@$(TARGET) < $(DATA_DIR)\sample3_in.txt > $(RESULT_DIR)\sample3_out.txt
	@$(TARGET) < $(DATA_DIR)\minimum1_in.txt > $(RESULT_DIR)\minimum1_out.txt
	@$(TARGET) < $(DATA_DIR)\minimum2_in.txt > $(RESULT_DIR)\minimum2_out.txt
	@$(TARGET) < $(DATA_DIR)\maximum1_in.txt > $(RESULT_DIR)\maximum1_out.txt
	@$(TARGET) < $(DATA_DIR)\maximum2_in.txt > $(RESULT_DIR)\maximum2_out.txt
	@$(TARGET) < $(DATA_DIR)\comprehensive_in.txt > $(RESULT_DIR)\comprehensive_out.txt
	@echo ====== All Test Results Saved in 'result\' ======

# 编译和链接
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

$(BUILD_DIR)\%.o: $(SRC_DIR)\%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 目录创建
dirs:
	$(call MKDIR_P,$(BUILD_DIR))
	$(call MKDIR_P,$(BIN_DIR))
	$(call MKDIR_P,$(RESULT_DIR))

# 清理
clean:
	-del /Q $(BUILD_DIR)\*.o 2>nul || true
	-del /Q $(BIN_DIR)\recover_design.exe 2>nul || true
	-del /Q $(RESULT_DIR)\*.txt 2>nul || true

.PHONY: all clean dirs
