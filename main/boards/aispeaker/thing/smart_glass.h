/**
 * 智能隐私玻璃控制器（基于问界 M9 功能逻辑）：
 * 支持单向隐私、四档调光、多方式控制（物理/语音/中控）、分区调节、一键遮光等功能。
 */

#include "iot/thing.h"
#include <driver/uart.h>
#include <esp_log.h>
#include <cstring>
#include <string>

#define BRIGHTNESS_FULL 1
#define BRIGHTNESS_SOFT 2
#define BRIGHTNESS_DIM  3
#define BRIGHTNESS_DARK 4

using namespace iot;

class SmartGlass : public Thing {
public:
    explicit SmartGlass(uart_port_t uart_port_num, int tx_io_num, int rx_io_num,
               int rts_io_num, int cts_io_num,
               int baud_rate, int buffer_size)
        : Thing("SmartGlass", "问界M9智能隐私玻璃控制器"),
          uart_port_num_(uart_port_num),
          tx_io_num_(tx_io_num),
          rx_io_num_(rx_io_num),
          rts_io_num_(rts_io_num),
          cts_io_num_(cts_io_num),
          baud_rate_(baud_rate),
          buffer_size_(buffer_size),
          left_window_level_(BRIGHTNESS_FULL),
          right_window_level_(BRIGHTNESS_FULL) {

        InitializeUart();

        properties_.AddNumberProperty("left_brightness", "左侧玻璃亮度等级（1~4）", [this]() {
            return left_window_level_;
        });

        properties_.AddNumberProperty("right_brightness", "右侧玻璃亮度等级（1~4）", [this]() {
            return right_window_level_;
        });

        methods_.AddMethod("SetGlassLevel", "设置玻璃亮度", ParameterList({
            Parameter("zone", "区域（left/right/all）", kValueTypeString, true),
            Parameter("level", "亮度等级（1~4）", kValueTypeNumber, true)
        }), [this](const ParameterList &parameters) {
            std::string zone = static_cast<std::string>(parameters["zone"].string());
            uint8_t level = static_cast<uint8_t>(parameters["level"].number());

            if (!IsValidBrightness(level)) {
                ESP_LOGW("SmartGlass", "Invalid brightness level: %d", level);
                return;
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
            }
        });

        methods_.AddMethod("QuickDarken", "一键遮光", ParameterList(), [this](const ParameterList &) {
            left_window_level_ = BRIGHTNESS_DARK;
            right_window_level_ = BRIGHTNESS_DARK;
            SetGlassLevel("all", BRIGHTNESS_DARK);
        });

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

    void SendUartMessage(const char *command_str) {
        uart_write_bytes(uart_port_num_, command_str, strlen(command_str));
        ESP_LOGI("SmartGlass", "Sent command: %s", command_str);
        ReadUartResponse();
    }

    void ReadUartResponse() {
        uint8_t data[128];
        int len = uart_read_bytes(uart_port_num_, data, sizeof(data) - 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            data[len] = 0;
            ESP_LOGI("SmartGlass", "Received UART response: %s", (char *)data);
        } else {
            ESP_LOGI("SmartGlass", "No UART response received.");
        }
    }

    void SetGlassLevel(const std::string &zone, int level) {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "ZONE:%s:LEVEL:%d", zone.c_str(), level);
        ESP_LOGI("SmartGlass", "Setting glass zone [%s] to level [%d]", zone.c_str(), level);
        SendUartMessage(cmd);
    }

    bool IsValidBrightness(int level) {
        return level >= BRIGHTNESS_FULL && level <= BRIGHTNESS_DARK;
    }
};
