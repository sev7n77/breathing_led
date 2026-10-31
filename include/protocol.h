#pragma once
#include <cstdint>
#include <unistd.h>

// 发送4字节定长帧：[0xA5][R][G][B]
inline void send_frame(int fd, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t buf[4];
    buf[0] = 0xA5;   // 帧头 (header / SOF)
    buf[1] = r;
    buf[2] = g;
    buf[3] = b;

    // write 返回实际写入的字节数，可能因为拔线等原因失败
    // 使用 (void) 强转，避免编译器产生 unused-result 警告
    ssize_t bytes_written = write(fd, buf, 4);
    (void)bytes_written;
}