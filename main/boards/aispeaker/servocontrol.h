#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "sdkconfig.h"
#include "board.h"
#include "freertos/FreeRTOS.h" // FreeRTOS 核心头文件
#include "freertos/task.h"     // FreeRTOS 任务相关头文件
#include "servo.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "driver/ledc.h"
#include "unity.h"
#include "config.h"

class ServoControl
{
private:
    ledc_mode_t ledc_mode_ = LEDC_LOW_SPEED_MODE; // LEDC 低速模式
    ledc_timer_t ledc_timer_ = LEDC_TIMER_3;      // LEDC 定时器
    uint32_t ledc_frequency_ = 50; // LEDC 频率
    ledc_timer_bit_t ledc_timer_bit_ = LEDC_TIMER_13_BIT; // LEDC 定时器位数
    ledc_channel_t ledc_channel_1_ = LEDC_CHANNEL_1; // LEDC 通道 1
    ledc_channel_t ledc_channel_2_ = LEDC_CHANNEL_2; // LEDC 通道 2
    ledc_channel_t ledc_channel_3_ = LEDC_CHANNEL_3; // LEDC 通道 3
    ledc_channel_t ledc_channel_4_ = LEDC_CHANNEL_4; // LEDC 通道 4

    gpio_num_t servo_pins_1_ = GPIO_NUM_NC; // 舵机 1 引脚
    gpio_num_t servo_pins_2_ = GPIO_NUM_NC; // 舵机 2 引脚
    gpio_num_t servo_pins_3_ = GPIO_NUM_NC; // 舵机 3 引脚
    gpio_num_t servo_pins_4_ = GPIO_NUM_NC; // 舵机 4 引脚

    // Add these declarations
    void SetServos(const std::array<float, 4> &angles);
    bool IsValidAngle(float angle) const;
    int32_t moveDelay_; // 移动延时
public:
    ServoControl(ledc_mode_t ledc_mode,
                                   ledc_timer_t ledc_timer,
                                   uint32_t ledc_frequency,
                                   ledc_timer_bit_t ledc_timer_bit,
                                   ledc_channel_t ledc_channel_1,
                                   ledc_channel_t ledc_channel_2,
                                   ledc_channel_t ledc_channel_3,
                                   ledc_channel_t ledc_channel_4,
                                   gpio_num_t servo_pins_1,
                                   gpio_num_t servo_pins_2,
                                   gpio_num_t servo_pins_3,
                                   gpio_num_t servo_pins_4);
    ~ServoControl();
    void InitializeServo();
    void sitDown();
    void lieDown();
    void sitDownWX();
    void standUp();
    void swingBackAndForth(int times);
    void swingLeftAndRight(int times);
    void turnLeft();
    void turnRight();
    void moveForward(int times);
    void moveBackward(int times);
    void dance(int times);
    void test0(u_int8_t channelIndex);
};

#endif // SERVO_CONTROL_H
