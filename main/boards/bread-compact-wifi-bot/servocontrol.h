#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "sdkconfig.h"
#include "board.h"
#include "servo/servo.h"
#include "freertos/FreeRTOS.h"  // FreeRTOS 核心头文件
#include "freertos/task.h"      // FreeRTOS 任务相关头文件

#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "driver/ledc.h"
#include "boards/bread-compact-wifi-bot/config.h"

class ServoControl {
private:
    light_mode_t light_mode_ = LIGHT_MODE_ALWAYS_ON;
    ledc_channel_t channels_[4]; // 舵机通道
    int32_t pins_[4];            // GPIO 引脚
    uint8_t count_;              // 舵机数量
    Servo* servo_; // 舵机控制器实例指针
    int32_t moveDelay_; // 移动延时

public:
    ServoControl();
    ~ServoControl();
    void InitializeServo();
    void setInitialPosition();
    void sitDown();
    void standUp();
    void turnLeft();
    void turnRight();
    void moveForward();
    void moveBackward();
    void dance();
};

#endif // SERVO_CONTROL_H
