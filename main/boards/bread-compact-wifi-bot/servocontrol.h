#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "sdkconfig.h"
#include "board.h"
#include "freertos/FreeRTOS.h"  // FreeRTOS 核心头文件
#include "freertos/task.h"      // FreeRTOS 任务相关头文件
#include "iot_servo.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "driver/ledc.h"
#include "unity.h"
#include "boards/bread-compact-wifi-bot/config.h"


class ServoControl {
private:
    // Add these declarations
    void SetServos(const std::array<float,4>& angles);
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
    void turnLeft(int times);
    void turnRight(int times);
    void moveForward(int times);
    void moveBackward(int times);
    void dance(int times);
    void test0(u_int8_t channelIndex);
    void testGpio(gpio_num_t gpio_num_);
};

#endif // SERVO_CONTROL_H
