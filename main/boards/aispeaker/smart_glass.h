/**
 * 智能隐私玻璃控制器（基于问界 M9 功能逻辑）：
 * 支持单向隐私、四档调光、多方式控制（物理/语音/中控）、分区调节、一键遮光等功能。
 */

#ifndef SMART_GLASS_H
#define SMART_GLASS_H

#include "iot/thing.h"
#include <driver/uart.h>
#include <string>

using namespace iot;

class SmartGlass : public Thing {
public:
    SmartGlass(uart_port_t uart_port_num ,int tx_io_num, int rx_io_num,
                int rts_io_num, int cts_io_num,
                int baud_rate, int buffer_size);

private:
    enum GlassBrightness {
        BRIGHTNESS_FULL = 1,    // 最亮
        BRIGHTNESS_SOFT = 2,    // 柔和
        BRIGHTNESS_DIM  = 3,    // 偏暗
        BRIGHTNESS_DARK = 4     // 全暗
    };
    int left_window_level_ = BRIGHTNESS_FULL;
    int right_window_level_ = BRIGHTNESS_FULL;

    uart_port_t uart_port_num_ = UART_NUM_1;
    int tx_io_num_;
    int rx_io_num_;
    int rts_io_num_;
    int cts_io_num_;
    int baud_rate_;
    int buffer_size_;

    void InitializeUart();
    void SendUartMessage(const char *command_str);
    void ReadUartResponse();
    void SetGlassLevel(const std::string &zone, int level);
    bool IsValidBrightness(int level);
};

#endif // SMART_GLASS_H
