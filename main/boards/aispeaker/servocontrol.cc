#include "servocontrol.h"
#include "esp_log.h"

#define TAG "ServoControl"

ServoControl::ServoControl(ledc_mode_t ledc_mode,
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
                                   gpio_num_t servo_pins_4)
    : ledc_mode_(ledc_mode),
      ledc_timer_(ledc_timer),
      ledc_frequency_(ledc_frequency),
      ledc_timer_bit_(ledc_timer_bit),
      ledc_channel_1_(ledc_channel_1),
      ledc_channel_2_(ledc_channel_2),
      ledc_channel_3_(ledc_channel_3),
      ledc_channel_4_(ledc_channel_4),
      servo_pins_1_(servo_pins_1),
      servo_pins_2_(servo_pins_2),
      servo_pins_3_(servo_pins_3),
      servo_pins_4_(servo_pins_4)
{
    InitializeServo();
}

ServoControl::~ServoControl()
{
    // 析构函数：清理资源
    ESP_LOGI(TAG, "ServoControl 对象已销毁，执行清理操作");
    iot_servo_deinit(ledc_mode_);
}

void ServoControl::InitializeServo() {
    moveDelay_ = 80; // 移动延时 150 毫秒
    servo_config_t servo_cfg = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = ledc_frequency_,
        .timer_number = ledc_timer_,
        .channels = {
            .servo_pin = {
                servo_pins_1_,
                servo_pins_2_,
                servo_pins_3_,
                servo_pins_4_,
            },
            .ch = {
                ledc_channel_1_,
                ledc_channel_2_,
                ledc_channel_3_,
                ledc_channel_4_,
            },
        },
        .channel_number = 4,
        .duty_resolution = ledc_timer_bit_,
    };

    // Initialize the servo
    // 替换TEST_ASSERT为错误处理
    esp_err_t ret = iot_servo_init(ledc_mode_, &servo_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "舵机初始化失败: %s", esp_err_to_name(ret));
    }
}

// 小狗起立，将所有舵机设置为 90°（中间位置）
void ServoControl::standUp()
{
    ESP_LOGI(TAG, "小狗起立，将所有舵机设置为 90°（中间位置）");
    SetServos({90, 90, 90, 90});
}

// 坐下
void ServoControl::sitDown()
{
    ESP_LOGI(TAG, "小狗坐下，前腿弯曲 45°，后腿伸展 135°");
    // 前腿舵机
    iot_servo_write_angle_async(ledc_mode_, 0, 45);  // 左前腿弯曲
    iot_servo_write_angle_async(ledc_mode_, 1, 135); // 右前腿弯曲
    iot_servo_sync_update(ledc_mode_);
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

    // 后腿舵机
    iot_servo_write_angle_async(ledc_mode_, 2, 45);  // 左后腿伸展
    iot_servo_write_angle_async(ledc_mode_, 3, 135); // 右后腿伸展
    iot_servo_sync_update(ledc_mode_);
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
}

// 卧下睡觉
void ServoControl::sitDownWX()
{
    iot_servo_write_angle_async(ledc_mode_, 0, 180);
    iot_servo_write_angle_async(ledc_mode_, 1, 0);
    iot_servo_write_angle_async(ledc_mode_, 2, 0);
    iot_servo_write_angle_async(ledc_mode_, 3, 180);
    iot_servo_sync_update(ledc_mode_); // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
}

// 趴下睡觉
void ServoControl::lieDown()
{
    iot_servo_write_angle_async(ledc_mode_, 0, 0);
    iot_servo_write_angle_async(ledc_mode_, 1, 180);
    iot_servo_write_angle_async(ledc_mode_, 2, 180);
    iot_servo_write_angle_async(ledc_mode_, 3, 0);
    iot_servo_sync_update(ledc_mode_); // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
}

void ServoControl::turnLeft()
{
    ESP_LOGI(TAG, "小狗向左转");
    // 前腿舵机
    iot_servo_write_angle_async(ledc_mode_, 0, 135); // 左前腿抬起
    iot_servo_write_angle_async(ledc_mode_, 1, 45);  // 右前腿压低
    iot_servo_sync_update(ledc_mode_);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
    // 后腿舵机
    iot_servo_write_angle_async(ledc_mode_, 2, 45);  // 左后腿压低
    iot_servo_write_angle_async(ledc_mode_, 3, 135); // 右后腿抬起
    iot_servo_sync_update(ledc_mode_);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
}

void ServoControl::turnRight()
{
    ESP_LOGI(TAG, "小狗向右转");
    // 前腿舵机
    iot_servo_write_angle_async(ledc_mode_, 0, 45);  // 左前腿压低
    iot_servo_write_angle_async(ledc_mode_, 1, 135); // 右前腿抬起
    iot_servo_sync_update(ledc_mode_);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
    // 后腿舵机
    iot_servo_write_angle_async(ledc_mode_, 2, 135); // 左后腿抬起
    iot_servo_write_angle_async(ledc_mode_, 3, 45);  // 右后腿压低
    iot_servo_sync_update(ledc_mode_);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
}

void ServoControl::moveForward(int times)
{
    ESP_LOGI(TAG, "小狗前进");
    for (int i = 0; i < times; i++)
    {
        iot_servo_write_angle_async(ledc_mode_, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 50);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 50);  // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 50); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90);  // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 130); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 50); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 50);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 130); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 50);  // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 130); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 50); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90);  // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 130); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    }
}

void ServoControl::moveBackward(int times)
{
    ESP_LOGI(TAG, "小狗后退");
    for (int i = 0; i < times; i++)
    {
        iot_servo_write_angle_async(ledc_mode_, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 50); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90);  // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 130); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 50);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 130); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 50);  // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 130); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 130); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 50); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 50); // 右后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90);  // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 50);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 50);  // 左后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(ledc_mode_, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_sync_update(ledc_mode_);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    }
}

void ServoControl::swingBackAndForth(int times)
{
    ESP_LOGI(TAG, "小狗前后摇摆");
    for (int i = 0; i < times; i++)
    {
        iot_servo_write_angle_async(ledc_mode_, 0, 135); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 135); // 左后腿向后
        iot_servo_write_angle_async(ledc_mode_, 1, 45);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 45);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));

        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));

        iot_servo_write_angle_async(ledc_mode_, 0, 45);  // 左前腿向前
        iot_servo_write_angle_async(ledc_mode_, 2, 45);  // 左后腿向前
        iot_servo_write_angle_async(ledc_mode_, 1, 135); // 右前腿向前
        iot_servo_write_angle_async(ledc_mode_, 3, 135); // 右后腿向前
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));

        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));
    }
}

void ServoControl::swingLeftAndRight(int times){
    ESP_LOGI(TAG, "小狗左右摇摆");
    for (int i = 0; i < times; i++)
    {
        iot_servo_write_angle_async(ledc_mode_, 0, 135); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 135); // 左后腿向后
        iot_servo_write_angle_async(ledc_mode_, 1, 135);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 135);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));

        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));

        iot_servo_write_angle_async(ledc_mode_, 0, 45);  // 左前腿向前
        iot_servo_write_angle_async(ledc_mode_, 2, 45);  // 左后腿向前
        iot_servo_write_angle_async(ledc_mode_, 1, 45); // 右前腿向前
        iot_servo_write_angle_async(ledc_mode_, 3, 45); // 右后腿向前
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));

        iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
        iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
        iot_servo_sync_update(ledc_mode_);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)150));
    }

}

void ServoControl::dance(int times)
{
    // ESP_LOGI(TAG, "小狗跳舞");
    // for (int i = 0; i < times; i++)
    // {
    //     iot_servo_write_angle_async(ledc_mode_, 0, 135); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 135); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 135);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 135);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 135);  // 左前腿向前
    //     iot_servo_write_angle_async(ledc_mode_, 2, 135);  // 左后腿向前
    //     iot_servo_write_angle_async(ledc_mode_, 1, 45); // 右前腿向前
    //     iot_servo_write_angle_async(ledc_mode_, 3, 45); // 右后腿向前
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 45); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 45); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 45);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 45);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 45); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 45); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 135);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 135);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(ledc_mode_, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(ledc_mode_, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(ledc_mode_);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));
    // }

    ESP_LOGI(TAG, "小狗抬头");
    for (u_int8_t i = 0; i < 20; i++)
    {
        iot_servo_write_angle_async(ledc_mode_, 0, 90 - i);
        iot_servo_write_angle_async(ledc_mode_, 1, 90 + i);
        iot_servo_sync_update(ledc_mode_);
        vTaskDelay(pdMS_TO_TICKS((int)10));
    }

    for (u_int8_t i = 0; i < 65; i++)
    {
        iot_servo_write_angle_async(ledc_mode_, 3, 90 - i);
        iot_servo_write_angle_async(ledc_mode_, 2, 90 + i);
        iot_servo_sync_update(ledc_mode_);
        vTaskDelay(pdMS_TO_TICKS((int)10));
    }
}

void ServoControl::test0(u_int8_t channelIndex)
{
    ESP_LOGI(TAG, "测试代码");
    standUp();
    vTaskDelay(pdMS_TO_TICKS(1000));
    dance(1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    standUp();
}

void ServoControl::SetServos(const std::array<float, 4> &angles)
{
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (!IsValidAngle(angles[i]))
        {
            ESP_LOGE(TAG, "非法角度: %.1f°", angles[i]);
            abort();
        }
        iot_servo_write_angle_async(ledc_mode_, i, angles[i]);
    }
    iot_servo_sync_update(ledc_mode_);
}

bool ServoControl::IsValidAngle(float angle) const
{
    return (angle >= 0.0f) && (angle <= 180.0f);
}
