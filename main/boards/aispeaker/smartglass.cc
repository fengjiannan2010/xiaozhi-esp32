/**
 * 智能隐私玻璃控制器（基于问界 M9 功能逻辑）：
 * 支持单向隐私、四档调光、多方式控制（物理/语音/中控）、分区调节、一键遮光等功能。
 */

#include "sdkconfig.h"
#include "iot/thing.h"
#include "board.h"

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <cstring>
#include "config.h"

#define TAG "SmartGlass"

namespace iot {

    enum GlassBrightness {
        BRIGHTNESS_FULL = 1,    // 最亮
        BRIGHTNESS_SOFT = 2,    // 柔和
        BRIGHTNESS_DIM  = 3,    // 偏暗
        BRIGHTNESS_DARK = 4     // 全暗
    };

    class SmartGlass : public Thing {
        private:
            int left_window_level_ = BRIGHTNESS_FULL;
            int right_window_level_ = BRIGHTNESS_FULL;

            void SendUartMessage(const char * command_str) {
                uart_write_bytes(ECHO_UART_PORT_NUM, command_str, strlen(command_str));
                ESP_LOGI(TAG, "Sent command: %s", command_str);
            }

            void InitializeUart() {
                uart_config_t uart_config = {
                    .baud_rate = ECHO_UART_BAUD_RATE,
                    .data_bits = UART_DATA_8_BITS,
                    .parity    = UART_PARITY_DISABLE,
                    .stop_bits = UART_STOP_BITS_1,
                    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                    .source_clk = UART_SCLK_DEFAULT,
                };
                ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
                ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
                ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, UART_ECHO_TXD, UART_ECHO_RXD, UART_ECHO_RTS, UART_ECHO_CTS));
                ESP_LOGI(TAG, "UART initialized");
            }

            void SetGlassLevel(const std::string& zone, int level) {
                // 模拟发送命令，如："ZONE:L:LEVEL:2" 或 "ZONE:ALL:LEVEL:4"
                char cmd[32];
                snprintf(cmd, sizeof(cmd), "ZONE:%s:LEVEL:%d", zone.c_str(), level);
                SendUartMessage(cmd);
            }

        public:
            SmartGlass() : Thing("SmartGlass", "问界M9智能隐私玻璃控制器") {
                InitializeUart();

                // 属性：左右车窗当前亮度等级
                properties_.AddNumberProperty("left_brightness", "左侧玻璃亮度等级（1~4）", [this]() {
                    return left_window_level_;
                });

                properties_.AddNumberProperty("right_brightness", "右侧玻璃亮度等级（1~4）", [this]() {
                    return right_window_level_;
                });

                // 方法：设置某一侧玻璃亮度
                methods_.AddMethod("SetGlassLevel", "设置玻璃亮度", ParameterList({
                    Parameter("zone", "区域（left/right/all）", kValueTypeString, true),
                    Parameter("level", "亮度等级（1~4）", kValueTypeNumber, true)
                }), [this](const ParameterList& parameters) {
                    std::string zone = static_cast<std::string>(parameters["zone"].string());
                    uint8_t level = static_cast<uint8_t>(parameters["level"].number());

                    // 检查亮度等级是否在有效范围内
                    if (level < 1 || level > 4) {
                        ESP_LOGW(TAG, "Invalid brightness level: %d", level);
                        return;
                    }

                    if (zone == "left") {
                        left_window_level_ = level;
                    } else if (zone == "right") {
                        right_window_level_ = level;
                    } else if (zone == "all") {
                        left_window_level_ = level;
                        right_window_level_ = level;
                    } else {
                        ESP_LOGW(TAG, "Unknown zone: %s", zone.c_str());
                        return;
                    }

                    SetGlassLevel(zone, level);
                });

                // 方法：一键遮光（设为最暗）
                methods_.AddMethod("QuickDarken", "一键遮光", ParameterList(), [this](const ParameterList&) {
                    left_window_level_ = BRIGHTNESS_DARK;
                    right_window_level_ = BRIGHTNESS_DARK;
                    SetGlassLevel("all", BRIGHTNESS_DARK);
                });
            }
    };

} // namespace iot

DECLARE_THING(SmartGlass);
