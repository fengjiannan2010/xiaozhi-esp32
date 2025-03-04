#include "servo.h"

const char *TAG = "SERVO";

// 构造函数
Servo::Servo(ledc_timer_t timer, ledc_mode_t mode, ledc_timer_bit_t duty_res, uint32_t frequency)
    : timer_(timer), mode_(mode), resolution_(duty_res), frequency_(frequency),
    servo_channels_(nullptr), channel_count_(0) {}

// 析构函数
Servo::~Servo() {
    if (servo_channels_ != nullptr) {
        delete[] servo_channels_;
    }
}

// 初始化舵机控制器
void Servo::init(const ledc_channel_t *channel_list, const int32_t *pin_list, uint8_t count) {
    channel_count_ = count;
    servo_channels_ = new ServoChannel[channel_count_];

    // 配置 LEDC 定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = resolution_,
        .timer_num = timer_,
        .freq_hz = frequency_,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 配置 LEDC 通道
    for (uint8_t i = 0; i < channel_count_; i++) {
        servo_channels_[i].channel_ = channel_list[i];
        servo_channels_[i].pin_ = pin_list[i];

        // 配置 LEDC 通道
        ledc_channel_config_t ledc_channel = {
            .gpio_num = servo_channels_[i].pin_,
            .speed_mode = mode_,
            .channel = servo_channels_[i].channel_ ,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = timer_,
            .duty = 0, // 初始占空比为 0
            .hpoint = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        ESP_LOGI(TAG, "Servo initialized on pin %d, channel %d, frequency %d Hz", (int)servo_channels_[i].pin_, (int)(i+1) , (int)frequency_);
    }
}

// 设置舵机角度
void Servo::setAngle(ledc_channel_t channel, uint8_t angle) {
    if (angle > 180) {
        angle = 180; // 限制角度范围为 0° 到 180°
    }

    // 将角度转换为占空比
    uint32_t duty = angleToDuty(angle);
    // 设置占空比
    ESP_ERROR_CHECK(ledc_set_duty(mode_, channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(mode_, channel));

    ESP_LOGI(TAG, "Servo channel %d set to %d degrees (duty: %d)", channel, (int)angle, (int)duty);    
}

// 将角度转换为占空比
uint32_t Servo::angleToDuty(uint32_t angle) {
    // 舵机 PWM 脉宽范围：500us (0°) 到 2500us (180°)
    const uint32_t min_pulse_us = 500;
    const uint32_t max_pulse_us = 2500;

    // 计算脉宽
    uint32_t pulse_us = min_pulse_us + (max_pulse_us - min_pulse_us) * angle / 180;

    // 将脉宽转换为占空比
    const uint32_t max_duty = (1 << resolution_) - 1;
    const uint32_t period_us = 1000000 / frequency_;
    uint32_t duty = (pulse_us * max_duty) / period_us;
    return duty;
}

// 获取舵机通道数量
uint8_t Servo::getChannelCount() const {
    return channel_count_;
}

// 获取指定通道的通道对象
ledc_channel_t Servo::getChannel(uint8_t index) const {
    if (index < channel_count_) {
        return servo_channels_[index].channel_;
    }
    return LEDC_CHANNEL_MAX; // 返回无效值
}