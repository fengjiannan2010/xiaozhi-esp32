/**
 * 智能隐私玻璃控制器（基于问界 M9 功能逻辑）：
 * 支持单向隐私、四档调光、多方式控制（物理/语音/中控）、分区调节、一键遮光等功能。
 */

#ifndef SMART_GLASS_H
#define SMART_GLASS_H

#include <driver/uart.h>
#include <esp_log.h>
#include <cstring>
#include <string>
#include "mcp_server.h"

class SmartGlass {
public:
    enum GlassBrightness {
        BRIGHTNESS_FULL = 1,    // 最亮
        BRIGHTNESS_SOFT = 2,    // 柔和
        BRIGHTNESS_DIM  = 3,    // 偏暗
        BRIGHTNESS_DARK = 4     // 全暗
    };

    explicit SmartGlass(uart_port_t uart_port_num, int tx_io_num, int rx_io_num,
                        int rts_io_num, int cts_io_num,
                        int baud_rate, int buffer_size)
        : uart_port_num_(uart_port_num),
          tx_io_num_(tx_io_num),
          rx_io_num_(rx_io_num),
          rts_io_num_(rts_io_num),
          cts_io_num_(cts_io_num),
          baud_rate_(baud_rate),
          buffer_size_(buffer_size),
          left_window_level_(BRIGHTNESS_FULL),
          right_window_level_(BRIGHTNESS_FULL) {
        InitializeUart();

        auto& mcp_server = McpServer::GetInstance();

        // 注册属性
        mcp_server.AddTool("self.smart_glass.get_left_brightness",
            "获取左侧玻璃亮度等级（1~4）",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return left_window_level_;
            });

        mcp_server.AddTool("self.smart_glass.get_right_brightness",
            "获取右侧玻璃亮度等级（1~4）",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return right_window_level_;
            });

        // 注册设置亮度方法
        mcp_server.AddTool("self.smart_glass.set_brightness",
            "设置玻璃亮度",
            PropertyList({
                Property("zone", kPropertyTypeString),
                Property("level", kPropertyTypeInteger, 1, 4)
            }), [this](const PropertyList& properties) -> ReturnValue {
                std::string zone = properties["zone"].value<std::string>();
                int level = properties["level"].value<int>();
                if (!IsValidBrightness(level)) {
                    ESP_LOGW("SmartGlass", "Invalid brightness level: %d", level);
                    return ReturnValue::Error("Invalid brightness level");
                }
                if (zone == "left") {
                    left_window_level_ = level;
                    SetGlassLevel("left", level);
                } else if (zone == "right") {
                    right_window_level_ = level;
                    SetGlassLevel("right", level);
                } else if (zone == "all") {
                    left_window_level_ = level;
                    right_window_level_ = level;
                    SetGlassLevel("all", level);
                } else {
                    ESP_LOGW("SmartGlass", "Unknown zone: %s", zone.c_str());
                    return ReturnValue::Error("Unknown zone");
                }
                return true;
            });

        // 注册一键遮光
        mcp_server.AddTool("self.smart_glass.quick_darken",
            "一键遮光",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                left_window_level_ = BRIGHTNESS_DARK;
                right_window_level_ = BRIGHTNESS_DARK;
                SetGlassLevel("all", BRIGHTNESS_DARK);
                return true;
            });

        // 启动时同步状态
        SetGlassLevel("left", left_window_level_);
        SetGlassLevel("right", right_window_level_);
    }

private:
    int left_window_level_;
    int right_window_level_;

    uart_port_t uart_port_num_;
    int tx_io_num_;
    int rx_io_num_;
    int rts_io_num_;
    int cts_io_num_;
    int baud_rate_;
    int buffer_size_;

    void InitializeUart() {
        uart_config_t uart_config = {
            .baud_rate = baud_rate_,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
        ESP_ERROR_CHECK(uart_driver_install(uart_port_num_, buffer_size_, 0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(uart_port_num_, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(uart_port_num_, tx_io_num_, rx_io_num_, rts_io_num_, cts_io_num_));
        ESP_LOGI("SmartGlass", "UART initialized");
    }

    void SendUartMessage(const char* command_str) {
        uart_write_bytes(uart_port_num_, command_str, strlen(command_str));
        ESP_LOGI("SmartGlass", "Sent command: %s", command_str);
        ReadUartResponse();
    }

    void ReadUartResponse() {
        uint8_t data[128];
        int len = uart_read_bytes(uart_port_num_, data, sizeof(data) - 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            data[len] = 0;
            ESP_LOGI("SmartGlass", "Received UART response: %s", (char*)data);
        } else {
            ESP_LOGI("SmartGlass", "No UART response received.");
        }
    }

    void SetGlassLevel(const std::string& zone, int level) {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "ZONE:%s:LEVEL:%d", zone.c_str(), level);
        ESP_LOGI("SmartGlass", "Setting glass zone [%s] to level [%d]", zone.c_str(), level);
        SendUartMessage(cmd);
    }

    bool IsValidBrightness(int level) {
        return level >= BRIGHTNESS_FULL && level <= BRIGHTNESS_DARK;
    }
};

#endif // SMART_GLASS_H
