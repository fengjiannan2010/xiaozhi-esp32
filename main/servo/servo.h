#ifndef SERVO_H
#define SERVO_H

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

class Servo {
private:
    ledc_timer_t timer_;       // LEDC 定时器
    ledc_mode_t mode_;         // LEDC 模式 (高速或低速)
    ledc_timer_bit_t resolution_; // 占空比分辨率
    uint32_t frequency_;       // PWM 频率

    struct ServoChannel {
        ledc_channel_t channel_; // LEDC 通道
        int32_t pin_;       // GPIO 引脚
    };

    ServoChannel *servo_channels_;   // 舵机通道数组
    uint8_t channel_count_;    // 舵机数量

public:
    // 构造函数
    Servo(ledc_timer_t timer = LEDC_TIMER_0,
                    ledc_mode_t mode = LEDC_LOW_SPEED_MODE,
                    ledc_timer_bit_t duty_res = LEDC_TIMER_13_BIT,
                    uint32_t frequency = 50);

    // 析构函数
    ~Servo();

    // 初始化舵机控制器
    void init(const ledc_channel_t *channel_list, const int32_t *pin_list, uint8_t count);
    // 设置舵机角度
    void setAngle(ledc_channel_t channel, uint8_t angle);

    // 获取舵机通道数量
    uint8_t getChannelCount() const;

    uint32_t angleToDuty(uint32_t angle);

    // 获取指定通道的通道对象
    ledc_channel_t getChannel(uint8_t index) const;
};

#endif // SERVO_H
