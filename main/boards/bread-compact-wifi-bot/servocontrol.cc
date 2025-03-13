#include "servocontrol.h"
#include "esp_log.h"

#define TAG "ServoControl"

ServoControl::ServoControl()
{
    InitializeServo();
}

ServoControl::~ServoControl()
{
    // 析构函数：清理资源
    ESP_LOGI(TAG, "ServoControl 对象已销毁，执行清理操作");
    iot_servo_deinit(LEDC_SPEED_MODE);
}

void ServoControl::InitializeServo()
{
    moveDelay_ = 80; // 移动延时 150 毫秒
    servo_config_t servo_cfg = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = LEDC_FREQUENCY,
        .timer_number = LEDC_TIMER,
        .channels = {
            .servo_pin = {
                SERVO1_PIN,
                SERVO2_PIN,
                SERVO3_PIN,
                SERVO4_PIN,
            },
            .ch = {
                LEDC_CHANNEL1,
                LEDC_CHANNEL2,
                LEDC_CHANNEL3,
                LEDC_CHANNEL4,
            },
        },
        .channel_number = 4,
    };

    // Initialize the servo
    // 替换TEST_ASSERT为错误处理
    esp_err_t ret = iot_servo_init(LEDC_SPEED_MODE, &servo_cfg);
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
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45);  // 左前腿弯曲
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 135); // 右前腿弯曲
    iot_servo_sync_update(LEDC_SPEED_MODE);
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

    // 后腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 45);  // 左后腿伸展
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135); // 右后腿伸展
    iot_servo_sync_update(LEDC_SPEED_MODE);
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
}

// 卧下睡觉
void ServoControl::sitDownWX()
{
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 180);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 0);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 0);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 180);
    iot_servo_sync_update(LEDC_SPEED_MODE); // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
}

// 趴下睡觉
void ServoControl::lieDown()
{
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 0);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 180);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 180);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 0);
    iot_servo_sync_update(LEDC_SPEED_MODE); // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
}

void ServoControl::turnLeft(int times = 1)
{
    ESP_LOGI(TAG, "小狗向左转");
    // 前腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 135); // 左前腿抬起
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 45);  // 右前腿压低
    iot_servo_sync_update(LEDC_SPEED_MODE);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
    // 后腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 45);  // 左后腿压低
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135); // 右后腿抬起
    iot_servo_sync_update(LEDC_SPEED_MODE);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
}

void ServoControl::turnRight(int times = 1)
{
    ESP_LOGI(TAG, "小狗向右转");
    // 前腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45);  // 左前腿压低
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 135); // 右前腿抬起
    iot_servo_sync_update(LEDC_SPEED_MODE);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
    // 后腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 135); // 左后腿抬起
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 45);  // 右后腿压低
    iot_servo_sync_update(LEDC_SPEED_MODE);               // 确保同步更新
    vTaskDelay(pdMS_TO_TICKS(moveDelay_));                // 延时 150 毫秒
}

void ServoControl::moveForward(int times = 1)
{
    ESP_LOGI(TAG, "小狗前进");
    for (int i = 0; i < times; i++)
    {
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 50);  // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 50);  // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 50); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90);  // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 130); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 50); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 50);  // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 130); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 50);  // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 130); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 50); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90);  // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 130); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    }
}

void ServoControl::moveBackward(int times = 1)
{
    ESP_LOGI(TAG, "小狗后退");
    for (int i = 0; i < times; i++)
    {
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 50); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90);  // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 130); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 50);  // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 130); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 50);  // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 130); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 130); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 50); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 50); // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90);  // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 50);  // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 130); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 50);  // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));

        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 130); // 右前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
        iot_servo_sync_update(LEDC_SPEED_MODE);              // 更新
        vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    }
}

void ServoControl::dance(int times = 1)
{
    // ESP_LOGI(TAG, "小狗前后摇摆");
    // for (int i = 0; i < times; i++)
    // {
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 135); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 135); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 45);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 45);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45);  // 左前腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 45);  // 左后腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 135); // 右前腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135); // 右后腿向前
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));
    // }

    // ESP_LOGI(TAG, "小狗左右摇摆");

    // for (int i = 0; i < times; i++)
    // {
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 135); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 135); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 135);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45);  // 左前腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 45);  // 左后腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 45); // 右前腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 45); // 右后腿向前
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));
    // }

    // ESP_LOGI(TAG, "小狗跳舞");

    // for (int i = 0; i < times; i++)
    // {
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 135); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 135); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 135);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 135);  // 左前腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 135);  // 左后腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 45); // 右前腿向前
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 45); // 右后腿向前
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 45); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 45);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 45);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 45); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 135);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));

    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90); // 左前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90); // 左后腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);  // 右前腿向后
    //     iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);  // 右后腿向后
    //     iot_servo_sync_update(LEDC_SPEED_MODE);               // 更新
    //     vTaskDelay(pdMS_TO_TICKS((int)150));
    // }

    ESP_LOGI(TAG, "小狗抬头");

    for (u_int8_t i = 0; i < 20; i++)
    {
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90 - i);
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90 + i);
        iot_servo_sync_update(LEDC_SPEED_MODE);
        vTaskDelay(pdMS_TO_TICKS((int)10));
    }

    for (u_int8_t i = 0; i < 65; i++)
    {
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90 - i);
        iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90 + i);
        iot_servo_sync_update(LEDC_SPEED_MODE);
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

// 建议使用 GPIO18、GPIO19、GPIO20、GPIO21
void ServoControl::testGpio(gpio_num_t gpio_num_)
{
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << gpio_num_);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK)
    {
        ESP_LOGI("GPIO_TEST", "GPIO4 可用");
    }
    else
    {
        ESP_LOGE("GPIO_TEST", "GPIO4 不可用，错误代码: %d", err);
    }
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
        iot_servo_write_angle_async(LEDC_SPEED_MODE, i, angles[i]);
    }
    iot_servo_sync_update(LEDC_SPEED_MODE);
}

bool ServoControl::IsValidAngle(float angle) const
{
    return (angle >= 0.0f) && (angle <= 180.0f);
}
