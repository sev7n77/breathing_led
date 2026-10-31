#include <cstdint>
#include <cmath>
#include <unistd.h>
#include <cstdio>
#include <csignal>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>

#include "protocol.h"
#include "serial_port.h"

volatile sig_atomic_t keep_running = 1;
void handle_sigint(int) { keep_running = 0; }

int main(int argc, char** argv) {
    bool preview_mode = false;
    bool hex_mode = false;
    const char* dev = "/dev/ttyUSB0";   // 嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻嘻

    // 简单解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--preview") == 0) preview_mode = true;
        if (std::strcmp(argv[i], "--hex") == 0) hex_mode = true;
    }

    int fd = -1;
    if (!preview_mode && !hex_mode) {
        // 只有正常模式才打开串口
        fd = open_serial(dev);
        if (fd == -1) {
            perror("open serial");
            return 1;
        }
        send_frame(fd, 0, 0, 0); // 启动全0
    }

    std::signal(SIGINT, handle_sigint);
    std::signal(SIGTERM, handle_sigint);
    std::signal(SIGPIPE, SIG_IGN);

    constexpr float PI = 3.14159265f;
    constexpr float T = 2.0f;
    auto start_time = std::chrono::steady_clock::now();

    while (keep_running) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = now - start_time;
        float t = diff.count();

        int phase = static_cast<int>(t / T) % 3;
        float local = fmodf(t, T);

        uint8_t bright = static_cast<uint8_t>(
            127.5f * (1.0f - cosf(2.0f * PI * local / T))
        );

        uint8_t r = (phase == 0) ? bright : 0;
        uint8_t g = (phase == 1) ? bright : 0;
        uint8_t b = (phase == 2) ? bright : 0;

        // 根据参数决定输出方式
        if (preview_mode) {
            // 在终端画彩色色块 (ANSI转义字符)
            std::string color_code = (phase == 0) ? "\033[41m" : (phase == 1) ? "\033[42m" : "\033[44m";
            std::string bar(bright / 10, ' '); // 每10点亮度画一个空格
            std::cout << "\r" << color_code << bar << "\033[0m" << std::flush;
            usleep(50000); // 预览模式 20Hz 就够了
        }
        else if (hex_mode) {
            // 纯打印十六进制
            printf("TX: A5 %02X %02X %02X\n", r, g, b);
            usleep(200000); // 打慢点让你看清
        }
        else {
            // 正常串口发送
            send_frame(fd, r, g, b);
            usleep(20000); // 50Hz
        }
    }

    if (!preview_mode && !hex_mode) {
        printf("\n检测到退出信号，正在发送全 0 灭灯...\n");
        send_frame(fd, 0, 0, 0);
        tcdrain(fd);
        close(fd);
    }
    return 0;
}