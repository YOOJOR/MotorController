# MotorController 使用说明

本项目是基于 STM32F407 的电机控制固件工程，使用 CMake 构建，使用 OpenOCD 烧录。

## 1. 环境依赖

请先确保系统中已安装以下工具，并且在 PATH 中可直接调用：

- cmake
- make
- arm-none-eabi-gcc 工具链（包含 gcc/g++/objcopy/size）
- openocd

可用以下命令快速检查：

    cmake --version
    make --version
    arm-none-eabi-gcc --version
    openocd --version

## 2. 工程关键文件

- `CMakeLists.txt`：主构建入口
- `cmake/gcc-arm-none-eabi.cmake`：交叉编译工具链配置
- `CMakePresets.json`：简化后的默认构建参数
- `openocd.cfg`：烧录配置（当前使用 JLink + SWD + stm32f4x target）

## 3. 推荐构建方式（使用 preset）

在项目根目录执行：

    cmake --preset default
    cmake --build --preset default -j

说明：

- `default` preset 已固定以下参数：
  - Generator: Unix Makefiles
  - Build 目录: `build`
  - Toolchain: `cmake/gcc-arm-none-eabi.cmake`
  - Build type: Debug

## 4. 不使用 preset 的等价命令

如果你更习惯传统命令，可以执行：

    cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
    cmake --build build -j

## 5. 烧录命令

编译完成后执行：

    openocd -f openocd.cfg -c "program build/MotorController.elf verify reset exit"

说明：

- JLink/SWD 设置已经写在 `openocd.cfg` 中，无需在命令行重复指定。

## 6. 常见问题

### 6.1 切换过生成器后无法配置

如果之前用过 Ninja 或其他生成器，建议清理旧构建目录后再重新配置：

    rm -rf build
    cmake --preset default

### 6.2 找不到 arm-none-eabi-gcc

说明工具链未安装或未加入 PATH。请安装对应工具链并重新打开终端。

### 6.3 OpenOCD 连接失败

排查顺序：

1. 确认调试器连接正常（JLink）
2. 确认目标板已上电
3. 确认 `openocd.cfg` 中接口和 target 与硬件一致

## 7. 最简日常流程

每次开发可按下面三步：

    cmake --preset default
    cmake --build --preset default -j
    openocd -f openocd.cfg -c "program build/MotorController.elf verify reset exit"
