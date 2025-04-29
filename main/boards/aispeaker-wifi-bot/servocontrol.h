#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "sdkconfig.h"
#include "board.h"
#include "freertos/FreeRTOS.h" // FreeRTOS 核心头文件
#include "freertos/task.h"     // FreeRTOS 任务相关头文件
#include "iot_servo.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "driver/ledc.h"
#include "unity.h"
#include "boards/bread-compact-wifi-bot/config.h"

class ServoControl
{
private:
    // Add these declarations
    void SetServos(const std::array<float, 4> &angles);
    bool IsValidAngle(float angle) const;
    int32_t moveDelay_; // 移动延时
public:
    ServoControl();
    ~ServoControl();
    void InitializeServo();
    void sitDown();
    void lieDown();
    void sitDownWX();
    void standUp();
    void swingBackAndForth(int times = 1);
    void swingLeftAndRight(int times = 1);
    void turnLeft(int times = 1);
    void turnRight(int times = 1);
    void moveForward(int times = 1);
    void moveBackward(int times = 1);
    void dance(int times);
    void test0(u_int8_t channelIndex);
};

#endif // SERVO_CONTROL_H
