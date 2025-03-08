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
    int32_t moveDelay_; // 移动延时
public:
    ServoControl();
    ~ServoControl();
    void InitializeServo();
    void sitDown();
    void sitDownW();
    void standUp();
    void turnLeft();
    void turnRight();
    void moveForward();
    void moveBackward();
    void dance();
    void test0(u_int8_t channelIndex);
    void testGpio(gpio_num_t gpio_num_);
};

#endif // SERVO_CONTROL_H
