
#include "servocontrol.h"

#define TAG "ServoControl"

ServoControl::ServoControl() {
    InitializeServo();
}

ServoControl::~ServoControl() {
    // 析构函数：清理资源
    ESP_LOGI(TAG, "ServoControl 对象已销毁，执行清理操作");
    iot_servo_deinit(LEDC_SPEED_MODE);
}

void ServoControl::InitializeServo() {
    moveDelay_ = 150; // 移动延时 150 毫秒
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
    TEST_ASSERT(ESP_OK == iot_servo_init(LEDC_SPEED_MODE, &servo_cfg));
}

// 小狗起立，将所有舵机设置为 90°（中间位置）
void ServoControl::standUp() {
    ESP_LOGI(TAG, "小狗起立，将所有舵机设置为 90°（中间位置）");
    // 将所有舵机设置为 90°（中间位置）
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 90);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 90);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 90);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 90);
    iot_servo_sync_update(LEDC_SPEED_MODE);
}

void ServoControl::sitDown() {
    ESP_LOGI(TAG, "小狗坐下，前腿弯曲 45°，后腿伸展 135°");
    // 前腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 45);  // 前腿弯曲
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 45);  // 前腿弯曲
    iot_servo_sync_update(LEDC_SPEED_MODE);
    // 后腿舵机
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 135); // 后腿伸展
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 135); // 后腿伸展
    iot_servo_sync_update(LEDC_SPEED_MODE);
}

void ServoControl::sitDownW() {
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 0, 180);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 1, 0);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 2, 0);
    iot_servo_write_angle_async(LEDC_SPEED_MODE, 3, 180);
    iot_servo_sync_update(LEDC_SPEED_MODE); // 确保同步更新
}


void ServoControl::turnLeft() {
    ESP_LOGI(TAG, "小狗向左转");
    // 前腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE, 0, 135); // 左前腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE, 1, 45);  // 右前腿压低
    // 后腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE, 2, 45);  // 左后腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE, 3, 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS(moveDelay_)); // 延时 150 毫秒
}

void ServoControl::turnRight() {
    ESP_LOGI(TAG, "小狗向右转");
    // 前腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE, 0,  45);  // 左前腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE, 1, 135); // 右前腿抬起
    // 后腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE, 2,  135); // 左后腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE, 3,  45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(moveDelay_)); // 延时 150 毫秒
}

void ServoControl::moveForward() {
    ESP_LOGI(TAG,"小狗前进");
    // 前腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE, 0, 135); // 左前腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE, 1, 45);  // 右前腿压低
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    // 改变屏幕显示

    // 后腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE, 2, 45);  // 左后腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE, 3, 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    // 改变屏幕显示

    // 交换动作
    iot_servo_write_angle(LEDC_SPEED_MODE, 0,  45); // 左前腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE, 1, 135); // 右前腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE, 2,  135); // 左后腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE, 3,  45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(moveDelay_)); // 延时 150 毫秒
}

void ServoControl::moveBackward() {
    ESP_LOGI(TAG,"小狗后退");
    // 前腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE,(0), 45);  // 左前腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE,(1), 135); // 右前腿抬起
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    // 后腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE,(2), 135); // 左后腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE,(3), 45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    
    // 交换动作
    iot_servo_write_angle(LEDC_SPEED_MODE,(0), 135); // 左前腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE,(1), 45);  // 右前腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE,(2), 45);  // 左后腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE,(3), 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
}

void ServoControl::dance() {
    ESP_LOGI(TAG,"小狗跳舞");
    // 前腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE,(0), 135); // 左前腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE,(1), 45);  // 右前腿压低
    // 后腿舵机
    iot_servo_write_angle(LEDC_SPEED_MODE,(2), 45);  // 左后腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE,(3), 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS(moveDelay_)); // 延时 150 毫秒

    // 交换动作
    iot_servo_write_angle(LEDC_SPEED_MODE,(0), 45);  // 左前腿压低
    iot_servo_write_angle(LEDC_SPEED_MODE,(1), 135); // 右前腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE,(2), 135); // 左后腿抬起
    iot_servo_write_angle(LEDC_SPEED_MODE,(3), 45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(moveDelay_)); // 延时 150 毫秒
}   

void ServoControl::test0(u_int8_t channelIndex) {
    ESP_LOGI(TAG, "测试代码");
    // for (size_t i = 0; i < 10; i++)
    // {   
    //     ESP_LOGI(TAG, "Channel %d , 0 °",channelIndex);
    //     iot_servo_write_angle(LEDC_SPEED_MODE, channelIndex,  0);
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    //     ESP_LOGI(TAG, "Channel %d , 90 °",channelIndex);
    //     iot_servo_write_angle(LEDC_SPEED_MODE, channelIndex,  90);
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }

    standUp();
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    sitDownW();
}

//建议使用 GPIO18、GPIO19、GPIO20、GPIO21
void ServoControl:: testGpio(gpio_num_t gpio_num_){
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << gpio_num_);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        ESP_LOGI("GPIO_TEST", "GPIO4 可用");
    } else {
        ESP_LOGE("GPIO_TEST", "GPIO4 不可用，错误代码: %d", err);
    }
}

