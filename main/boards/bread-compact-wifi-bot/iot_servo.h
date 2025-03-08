/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #ifndef _IOT_SERVO_H_
 #define _IOT_SERVO_H_
 
 #include "esp_err.h"
 #include "driver/ledc.h"
 #include "driver/gpio.h"
 
 /**
  * @brief Configuration of servo motor channel
  *
  */
 typedef struct {
     gpio_num_t servo_pin[LEDC_CHANNEL_MAX];     /**< Pin number of PWM output */
     ledc_channel_t ch[LEDC_CHANNEL_MAX];          /**< The LEDC channels used */
 } servo_channel_t;
 
 /**
  * @brief Configuration of servo motor
  *
  */
 typedef struct {
     uint16_t max_angle;        /**< Servo max angle, e.g., 180 */
     uint16_t min_width_us;     /**< Pulse width corresponding to minimum angle, typically 500us */
     uint16_t max_width_us;     /**< Pulse width corresponding to maximum angle, typically 2500us */
     uint32_t freq;             /**< PWM frequency */
     ledc_timer_t timer_number; /**< Timer number of LEDC */
     servo_channel_t channels;  /**< Channels to use */
     uint8_t channel_number;    /**< Total number of channels */
 } servo_config_t;
 

 typedef struct {
    float base_freq;          // 基准频率
    float adj_factor;         // 动态调整系数（根据负载算法计算）
    uint32_t safe_min_freq;   // 安全下限频率（例45Hz）
    uint32_t safe_max_freq;   // 安全上限频率（例55Hz） 
} servo_freq_compensation_t;


 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Initialize LEDC to control the servo.
  *
  * @param speed_mode Select the LEDC channel group with specified speed mode.
  * @param config Pointer to servo configuration structure.
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  *     - ESP_FAIL Configure LEDC failed
  */
 esp_err_t iot_servo_init(ledc_mode_t speed_mode, const servo_config_t *config);
 
 /**
  * @brief Deinitialize LEDC for servo.
  *
  * @param speed_mode Select the LEDC channel group with specified speed mode.
  *
  * @return
  *     - ESP_OK Success
  */
 esp_err_t iot_servo_deinit(ledc_mode_t speed_mode);
 
 /**
  * @brief Set the servo motor to a certain angle.
  *
  * @note This API is not thread-safe.
  *
  * @param speed_mode Select the LEDC channel group with specified speed mode.
  * @param channel Servo channel index (0 ~ channel_number-1) as configured in servo_config_t.
  * @param angle The target angle (in degrees).
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */

 esp_err_t iot_servo_write_angle(ledc_mode_t speed_mode, uint8_t channel, float angle);
 
 esp_err_t iot_servo_write_angle_async(ledc_mode_t speed_mode, uint8_t channel, float angle);
 /**
  * @brief Update the PWM duty for all configured servo channels simultaneously.
  *
  * This function should be called after setting the desired angle for each channel,
  * so that all channels update their PWM registers at the same time.
  *
  * @param speed_mode Select the LEDC channel group with specified speed mode.
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
 esp_err_t iot_servo_sync_update(ledc_mode_t speed_mode);
 
 /**
  * @brief Read the current angle of a servo channel.
  *
  * @param speed_mode Select the LEDC channel group with specified speed mode.
  * @param channel Servo channel index (0 ~ channel_number-1) as configured in servo_config_t.
  * @param angle Pointer to store the current angle.
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
 esp_err_t iot_servo_read_angle(ledc_mode_t speed_mode, uint8_t channel, float *angle);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* _IOT_SERVO_H_ */
 