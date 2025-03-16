/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include "esp_err.h"
#include <string.h> // Add this line for memcpy()
#include "driver/ledc.h"
#include "iot_servo.h"

static const char *TAG = "servo";

#define SERVO_CHECK(a, str, ret_val)                                  \
    do                                                                \
    {                                                                 \
        if (!(a))                                                     \
        {                                                             \
            ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str); \
            return (ret_val);                                         \
        }                                                             \
    } while (0)

/* 使用 13 位分辨率提高 PWM 控制精度 */
#define SERVO_LEDC_INIT_BITS LEDC_TIMER_13_BIT
#define SERVO_FREQ_MIN 50
#define SERVO_FREQ_MAX 400

/* Add MAX/MIN macros for frequency clamping */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static uint32_t g_full_duty = 0;
static servo_config_t g_cfg[LEDC_SPEED_MODE_MAX] = {0};

// 在文件顶部增加频率补偿结构体
static servo_freq_compensation_t g_freq_comp = {
    .base_freq = 50.0f,
    .adj_factor = 1.0f,
    .safe_min_freq = 45,
    .safe_max_freq = 55};

/**
 * @brief 根据角度计算 PWM duty 值
 */
static uint32_t calculate_duty(ledc_mode_t speed_mode, float angle)
{
    float angle_us = angle / g_cfg[speed_mode].max_angle *
                         (g_cfg[speed_mode].max_width_us - g_cfg[speed_mode].min_width_us) +
                     g_cfg[speed_mode].min_width_us;
    ESP_LOGD(TAG, "angle_us: %f", angle_us);
    uint32_t duty = (uint32_t)(((float)g_full_duty * angle_us * g_cfg[speed_mode].freq) / 1000000.0f);
    return duty;
}

/**
 * @brief 根据 PWM duty 计算舵机角度
 */
static float calculate_angle(ledc_mode_t speed_mode, uint32_t duty)
{
    float angle_us = ((float)duty * 1000000.0f) / ((float)g_full_duty * g_cfg[speed_mode].freq);
    angle_us -= g_cfg[speed_mode].min_width_us;
    if (angle_us < 0.0f)
    {
        angle_us = 0.0f;
    }
    float angle = angle_us * g_cfg[speed_mode].max_angle /
                  (g_cfg[speed_mode].max_width_us - g_cfg[speed_mode].min_width_us);
    return angle;
}

esp_err_t iot_servo_init(ledc_mode_t speed_mode, const servo_config_t *config)
{
    esp_err_t ret;
    SERVO_CHECK(config != NULL, "Pointer of config is invalid", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(config->channel_number > 0 && config->channel_number <= LEDC_CHANNEL_MAX,
                "Servo channel number out of range", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(config->freq <= SERVO_FREQ_MAX && config->freq >= SERVO_FREQ_MIN,
                "Servo PWM frequency out of range", ESP_ERR_INVALID_ARG);

    // 增加频率补偿初始化
    g_freq_comp.base_freq = config->freq;
    g_freq_comp.adj_factor = 1.0f; // 默认不调整

    uint64_t pin_mask = 0;
    uint32_t ch_mask = 0;
    for (size_t i = 0; i < config->channel_number; i++)
    {
        uint64_t _pin_mask = 1ULL << config->channels.servo_pin[i];
        uint32_t _ch_mask = 1UL << config->channels.ch[i];
        SERVO_CHECK(!(pin_mask & _pin_mask), "Servo gpio has a duplicate", ESP_ERR_INVALID_ARG);
        SERVO_CHECK(!(ch_mask & _ch_mask), "Servo channel has a duplicate", ESP_ERR_INVALID_ARG);
        SERVO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(config->channels.servo_pin[i]), "Servo gpio invalid", ESP_ERR_INVALID_ARG);
        pin_mask |= _pin_mask;
        ch_mask |= _ch_mask;
    }

    ledc_timer_config_t ledc_timer = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = SERVO_LEDC_INIT_BITS, /* 使用 13-bit 分辨率 */
        .freq_hz = config->freq,
        .speed_mode = speed_mode,
        .timer_num = config->timer_number,
        .deconfigure = false};

    ret = ledc_timer_config(&ledc_timer);
    SERVO_CHECK(ret == ESP_OK, "LEDC timer configuration failed", ESP_FAIL);

    for (size_t i = 0; i < config->channel_number; i++)
    {
        ledc_channel_config_t ledc_ch = {
            .intr_type = LEDC_INTR_DISABLE,
            .channel = config->channels.ch[i],
            .duty = 0, /* 初始化为 0° */
            .gpio_num = config->channels.servo_pin[i],
            .speed_mode = speed_mode,
            .timer_sel = config->timer_number,
            .hpoint = 0,
            .flags.output_invert = 0};
        ret = ledc_channel_config(&ledc_ch);
        SERVO_CHECK(ret == ESP_OK, "LEDC channel configuration failed", ESP_FAIL);
    }

    g_full_duty = (1 << SERVO_LEDC_INIT_BITS) - 1;
    g_cfg[speed_mode] = *config;
    return ESP_OK;
}

esp_err_t iot_servo_deinit(ledc_mode_t speed_mode)
{
    SERVO_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "LEDC speed mode invalid", ESP_ERR_INVALID_ARG);
    for (size_t i = 0; i < g_cfg[speed_mode].channel_number; i++)
    {
        ledc_stop(speed_mode, g_cfg[speed_mode].channels.ch[i], 0);
    }
    ledc_timer_rst(speed_mode, g_cfg[speed_mode].timer_number);
    g_full_duty = 0;
    return ESP_OK;
}

/**
 * @brief 设置舵机角度
 *
 * 修改点：channel 参数为索引，通过 g_cfg[speed_mode].channels.ch[channel]
 *          获取实际的 LEDC 通道号。该函数仅设置 PWM duty，不进行 duty 更新，
 *          统一调用 iot_servo_sync_update 以实现同步更新。
 */
esp_err_t iot_servo_write_angle_async(ledc_mode_t speed_mode, uint8_t channel, float angle)
{
    SERVO_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "LEDC speed mode invalid", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(channel < g_cfg[speed_mode].channel_number, "Servo channel index out of range", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(angle >= 0.0f, "Angle can't be negative", ESP_ERR_INVALID_ARG);

    uint32_t duty = calculate_duty(speed_mode, angle);
    /* 通过索引获取实际 LEDC 通道号 */
    ledc_channel_t ledc_ch = g_cfg[speed_mode].channels.ch[channel];
    esp_err_t ret = ledc_set_duty(speed_mode, ledc_ch, duty);
    SERVO_CHECK(ret == ESP_OK, "Write servo angle failed", ESP_FAIL);

    /* 注意：不在此处调用 ledc_update_duty，统一由 iot_servo_sync_update 调用 */
    return ESP_OK;
}

esp_err_t iot_servo_write_angle(ledc_mode_t speed_mode, uint8_t channel, float angle)
{
    SERVO_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "LEDC speed mode invalid", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(channel < g_cfg[speed_mode].channel_number, "Servo channel index out of range", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(angle >= 0.0f, "Angle can't be negative", ESP_ERR_INVALID_ARG);

    uint32_t duty = calculate_duty(speed_mode, angle);
    /* 通过索引获取实际 LEDC 通道号 */
    ledc_channel_t ledc_ch = g_cfg[speed_mode].channels.ch[channel];
    esp_err_t ret = ledc_set_duty(speed_mode, ledc_ch, duty);
    ret |= ledc_update_duty(speed_mode, ledc_ch);
    SERVO_CHECK(ret == ESP_OK, "Write servo angle failed", ESP_FAIL);

    /* 注意：不在此处调用 ledc_update_duty，统一由 iot_servo_sync_update 调用 */
    return ESP_OK;
}

/**
 * @brief 同步更新所有配置的舵机通道，使多个舵机能够同时更新 PWM duty
 */
esp_err_t iot_servo_sync_update(ledc_mode_t speed_mode)
{
    SERVO_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "LEDC speed mode invalid", ESP_ERR_INVALID_ARG);
    esp_err_t ret = ESP_OK;
    for (size_t i = 0; i < g_cfg[speed_mode].channel_number; i++)
    {
        ret |= ledc_update_duty(speed_mode, g_cfg[speed_mode].channels.ch[i]);
    }
    return ret;
}

/**
 * @brief 读取舵机当前角度
 *
 * @param speed_mode LEDC 速度模式
 * @param channel 舵机通道索引（0 ~ channel_number-1）
 * @param angle 指向存储角度的变量指针
 */
esp_err_t iot_servo_read_angle(ledc_mode_t speed_mode, uint8_t channel, float *angle)
{
    SERVO_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "LEDC speed mode invalid", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(channel < g_cfg[speed_mode].channel_number, "Servo channel index out of range", ESP_ERR_INVALID_ARG);
    uint32_t duty = ledc_get_duty(speed_mode, g_cfg[speed_mode].channels.ch[channel]);
    float a = calculate_angle(speed_mode, duty);
    *angle = a;
    return ESP_OK;
}

// 新增频率补偿方法
void update_frequency_compensation(ledc_mode_t speed_mode)
{
    // 实现带保护机制的频率更新逻辑
    float new_freq = g_freq_comp.base_freq * g_freq_comp.adj_factor;
    new_freq = MAX(MIN(new_freq, g_freq_comp.safe_max_freq), g_freq_comp.safe_min_freq);

    // 记录调整前的配置
    servo_config_t old_cfg = g_cfg[speed_mode];

    ledc_timer_config_t timer_cfg = {
        .speed_mode = speed_mode,
        .duty_resolution = SERVO_LEDC_INIT_BITS,
        .timer_num = old_cfg.timer_number,
        .freq_hz = (uint32_t)new_freq,
        .clk_cfg = LEDC_AUTO_CLK};

    esp_err_t ret = ledc_timer_config(&timer_cfg);
    if (ret == ESP_OK)
    {
        g_cfg[speed_mode].freq = (uint32_t)new_freq; // 更新配置缓存
        ESP_LOGI(TAG, "Frequency adjusted: %.1fHz -> %" PRIu32 "Hz",
                 g_freq_comp.base_freq, g_cfg[speed_mode].freq);
    }
    else
    {
        ESP_LOGE(TAG, "Frequency adjust failed! Keep original %.1fHz",
                 g_freq_comp.base_freq);
    }
}
/**
 * @brief 设置频率补偿参数 / Set frequency compensation parameters
 *
 * @param comp 补偿配置结构体指针 / Pointer to compensation config struct
 * @return
 *     - ESP_OK 成功 / Success
 *     - ESP_ERR_INVALID_ARG 参数错误 / Invalid argument
 */
esp_err_t iot_servo_set_freq_compensation(const servo_freq_compensation_t *comp)
{
    SERVO_CHECK(comp != NULL, "Invalid comp pointer", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(comp->base_freq >= SERVO_FREQ_MIN && comp->base_freq <= SERVO_FREQ_MAX,
                "Base freq out of range", ESP_ERR_INVALID_ARG);
    SERVO_CHECK(comp->safe_min_freq <= comp->safe_max_freq,
                "Invalid freq range", ESP_ERR_INVALID_ARG);

    memcpy(&g_freq_comp, comp, sizeof(servo_freq_compensation_t));
    update_frequency_compensation(LEDC_LOW_SPEED_MODE); // 假设默认使用低速模式

    return ESP_OK;
}

/**
 * @brief 获取当前频率补偿配置 / Get current frequency compensation config
 *
 * @param comp 存储配置的结构体指针 / Pointer to store config
 * @return
 *     - ESP_OK 成功 / Success
 *     - ESP_ERR_INVALID_ARG 参数错误 / Invalid argument
 */
esp_err_t iot_servo_get_freq_compensation(servo_freq_compensation_t *comp)
{
    SERVO_CHECK(comp != NULL, "Invalid comp pointer", ESP_ERR_INVALID_ARG);

    memcpy(comp, &g_freq_comp, sizeof(servo_freq_compensation_t));
    return ESP_OK;
}

esp_err_t iot_servo_stop(ledc_mode_t speed_mode)
{
    SERVO_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "LEDC speed mode invalid", ESP_ERR_INVALID_ARG);
    for (size_t i = 0; i < g_cfg[speed_mode].channel_number; i++)
    {
        ledc_stop(speed_mode, g_cfg[speed_mode].channels.ch[i], 0);
    }
    return ESP_OK;
}