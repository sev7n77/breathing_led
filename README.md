# RM 呼吸灯·视觉端 (Breathing LED Vision)

RoboMaster 算法组与电控组联调作业。视觉端通过 C++ 计算正弦呼吸曲线，并通过串口（UART）发送 RGB 颜色值给电控端，实现红、绿、蓝三色全周期交替呼吸效果。
## 🖥️ 运行环境与硬件设备

本项目主要在以下软硬件环境中开发与测试通过：

### 软件环境
- **宿主操作系统**：Windows 11
- **虚拟化环境**：WSL 2 (Ubuntu 22.04 LTS )
- **开发工具**：CLion + CMake
- **编程语言**：C++ 17
- **通信依赖**：Linux `termios` 库、`usbipd-win` (用于 WSL 2 USB透传)


## ✨ 功能特性
- **平滑呼吸**：使用绝对时间 `std::chrono` 和正弦公式 `127.5 * (1 - cos(2πt/T))` 计算亮度，无时间累积漂移。
- **全周期交替**：红、绿、蓝三色依次完整走完一个“暗→亮→暗”的 2 秒周期。
- **安全防护**：
    - 启动和按 `Ctrl+C` / 强杀退出时，自动补发全 0（灭灯）帧。
    - 忽略 `SIGPIPE` 信号，防止联调拔线时程序崩溃。
- **调试模式（命令行参数）**：
    - `--preview`：在终端直接打印彩色色块，无硬件预览呼吸效果。
    - `--hex`：在终端直接打印十六进制数据包，无需串口即可验证协议。

## 🔌 通信协议 (固定 4 字节)
- **物理层**：115200 8N1（8数据位，无校验位，1停止位），无流控。
- **发送频率**：50 Hz（每 20ms 一帧）。
- **帧格式**：

| 偏移 | 字段 | 类型 | 说明 |
| :--- | :--- | :--- | :--- |
| 0 | header | uint8_t | 固定 `0xA5` |
| 1 | r | uint8_t | 红色 0~255 |
| 2 | g | uint8_t | 绿色 0~255 |
| 3 | b | uint8_t | 蓝色 0~255 |

*注：本协议为单向通信，无回包，无应用层校验码。*

## 🛠️ 如何编译与运行
本项目使用 CMake 构建。

```bash
# 编译项目
mkdir build && cd build
cmake ..
make

# 正常运行（连接真实串口，默认路径 /dev/ttyUSB0）
./breathing_led

# 终端预览呼吸色块
./breathing_led --preview

# 终端打印十六进制协议数据
./breathing_led --hex
```

## 🧪 本地自测详细方法（无实体硬件）
### **方法一**：终端彩色色块预览（验证呼吸算法）
在终端执行：
```bash
./cmake-build-debug/breathing_led --preview
```
或者 (这个最方便）：
```bash
直接在clion右上角运行调试/配置箭头处点击,选择编辑配置,程序实参 (R) 输入--preview
```
你会看到终端里出现红、绿、蓝色的长条色块，依次平滑地变长变短（明暗交替）。



### **方法二**：十六进制数据打印（验证通信协议）
在终端执行：
```bash
./cmake-build-debug/breathing_led --hex
```
或者另一种(与上者同理)
你会在屏幕上看到类似 TX: A5 00 00 00、TX: A5 1F 00 00 的十六进制数据疯狂刷屏。



### **方法三**：使用虚拟串口对
如果你连实体串口线都没有，可以用 socat 创建一对虚拟串口来模拟硬件通信：

打开一个终端，创建虚拟串口：

```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
 假设输出显示分配了  /dev/pts/3 和 /dev/pts/4

修改 main.cpp,把 dev 改为 /dev/pts/3,重新编译并运行
````

新建个终端监听接收端：

```bash
hexdump -C /dev/pts/4
```

你可以清晰地在第二个终端里看到以 a5 开头的 4 字节数据包刷屏，退出程序时，最后会刷出一个 a5 00 00 00，证明安全防呆逻辑生效。


**如果看不到最后的 `A5 00 00 00`**，
在终端使用 `xxd` 监听虚拟串口：
```bash
xxd -c 4 /dev/pts/4
```
## 🛠️ 实际联调 （ WSL 2 挂载 USB 串口设备流程 ）

在 Windows 下使用 WSL 2 开发时，默认无法直接访问 USB 设备。需要通过 `usbipd-win` 将 Windows 上的串口设备“透传”给 WSL。

请严格按照以下步骤操作：

### 步骤零：前置准备（安装 usbipd-win）
在 Windows 宿主机上，必须安装 `usbipd-win` 才能使用透传功能。
打开 **Windows PowerShell（以管理员身份运行）**，执行以下命令安装：
```powershell
winget install usbipd
```
### 步骤一：Windows 端识别与绑定（需管理员权限）
列出所有 USB 设备，找到串口设备的 BUSID：

```powershell
usbipd list
```
绑定：
```powershell
usbipd bind --busid <你的ID>
```
连接到WSL：
```powershell
usbipd attach --wsl --busid <你的ID>
```
### 步骤二：WSL 端确认设备节点
切回 WSL/Linux 终端：

查看透传后的串口设备节点：

```
ls /dev/ttyUSB* /dev/ttyACM*
```
如果输出包含 /dev/ttyUSB0 或 /dev/ttyACM0，说明硬件已成功透传进 WSL。

**如遇权限问题（Permission denied）**：
默认情况下普通用户无权读写串口，需将当前用户加入 dialout 组：

```
sudo usermod -aG dialout $USER
```
⚠️ 执行完毕后，必须彻底关闭并重新打开 WSL 终端才能生效。
### 步骤三：修改代码中的串口路径
打开 src/main.cpp，找到定义串口路径的代码（约在第 20 行）：
```cpp
// 将此处修改为你在步骤二中查到的实际设备路径
const char* dev = "/dev/ttyUSB0"; // 也可能是 /dev/ttyACM0
```
保存并重新编译，即可正常通信。


## 🚑 USB 透传常见坑点排查 (FAQ)
- **/dev/ttyUSB0 出现又瞬间消失？**  
这是 Ubuntu 22.04 的 brltty（盲文显示器驱动）服务误抢了 CH340 串口。直接在 WSL 终端执行 sudo apt remove brltty 卸载该服务，然后重新执行 attach。

- **usbipd: error: The device is being used by another process？**  
说明 Windows 上其他软件（如串口助手、Keil等）占用了该 COM 口，请关闭相关软件后重新 attach 透传。

- **拔插线后 WSL 找不到设备了？**  
因为物理断开导致 USB/IP 连接失效，重新在 PowerShell 里敲一次 usbipd attach --wsl --busid <你的ID> 即可。
