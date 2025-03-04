#include "servocontrol.h"

#define TAG "ServoControl"

ServoControl::ServoControl() : servo_(nullptr) {
    InitializeServo();
}

ServoControl::~ServoControl() {
    // 析构函数：清理资源
    ESP_LOGI(TAG, "ServoControl 对象已销毁，执行清理操作");
    if (servo_) {
        setInitialPosition(); // 将舵机复位到初始位置
        delete servo_;        // 释放舵机控制器实例
        servo_ = nullptr;     // 设置指针为空
    }
}

void ServoControl::InitializeServo() {
    // 初始化舵机通道和 GPIO 引脚
    channels_[0] = LEDC_CHANNEL_0;
    channels_[1] = LEDC_CHANNEL_1;
    channels_[2] = LEDC_CHANNEL_2;
    channels_[3] = LEDC_CHANNEL_3;

    pins_[0] = SERVO1_PIN;
    pins_[1] = SERVO2_PIN;
    pins_[2] = SERVO3_PIN;
    pins_[3] = SERVO4_PIN;

    moveDelay_ = 150; // 移动延时 150 毫秒
    count_ = sizeof(channels_) / sizeof(channels_[0]);

    // 初始化舵机控制器实例
    servo_ = new Servo(LEDC_TIMER, LEDC_SPEED_MODE, LEDC_RESOLUTION, LEDC_FREQUENCY);
    if (servo_) {
        servo_->init(channels_, pins_, count_);
    } else {
        ESP_LOGE(TAG, "舵机控制器初始化失败！");
    }
}

void ServoControl::setInitialPosition() {
    // 将所有舵机设置为 90°（中间位置）
    for (uint8_t i = 0; i < count_; i++) {
        servo_->setAngle(servo_->getChannel(i), 90);
    }
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 500 毫秒
}

void ServoControl::sitDown() {
    printf("小狗坐下\n");
    // 前腿舵机
    servo_->setAngle(servo_->getChannel(0), 45); // 前腿弯曲
    servo_->setAngle(servo_->getChannel(1), 45); // 前腿弯曲
    // 后腿舵机
    servo_->setAngle(servo_->getChannel(2), 135); // 后腿伸展
    servo_->setAngle(servo_->getChannel(3), 135); // 后腿伸展
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
}

// 小狗起立，将所有舵机设置为 90°（中间位置）
void ServoControl::standUp() {
    ESP_LOGE(TAG, "小狗起立，将所有舵机设置为 90°（中间位置）");
    // 将所有舵机设置为 90°（中间位置）
    setInitialPosition();
}

void ServoControl::turnLeft() {
    printf("小狗向左转\n");
    // 前腿舵机
    servo_->setAngle(servo_->getChannel(0), 135); // 左前腿抬起
    servo_->setAngle(servo_->getChannel(1), 45);  // 右前腿压低
    // 后腿舵机
    servo_->setAngle(servo_->getChannel(2), 45);  // 左后腿压低
    servo_->setAngle(servo_->getChannel(3), 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
}

void ServoControl::turnRight() {
    printf("小狗向右转\n");
    // 前腿舵机
    servo_->setAngle(servo_->getChannel(0), 45);  // 左前腿压低
    servo_->setAngle(servo_->getChannel(1), 135); // 右前腿抬起
    // 后腿舵机
    servo_->setAngle(servo_->getChannel(2), 135); // 左后腿抬起
    servo_->setAngle(servo_->getChannel(3), 45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(1000)); // 延时 1 秒
}

void ServoControl::moveForward() {
    printf("小狗前进\n");
    // 前腿舵机
    servo_->setAngle(servo_->getChannel(0), 135); // 左前腿抬起
    servo_->setAngle(servo_->getChannel(1), 45);  // 右前腿压低
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    // 改变屏幕显示

    // 后腿舵机
    servo_->setAngle(servo_->getChannel(2), 45);  // 左后腿压低
    servo_->setAngle(servo_->getChannel(3), 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    // 改变屏幕显示

    // 交换动作
    servo_->setAngle(servo_->getChannel(0), 45);  // 左前腿压低
    servo_->setAngle(servo_->getChannel(1), 135); // 右前腿抬起
    servo_->setAngle(servo_->getChannel(2), 135); // 左后腿抬起
    servo_->setAngle(servo_->getChannel(3), 45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
}

void ServoControl::moveBackward() {
    printf("小狗后退\n");
    // 前腿舵机
    servo_->setAngle(servo_->getChannel(0), 45);  // 左前腿压低
    servo_->setAngle(servo_->getChannel(1), 135); // 右前腿抬起
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    // 后腿舵机
    servo_->setAngle(servo_->getChannel(2), 135); // 左后腿抬起
    servo_->setAngle(servo_->getChannel(3), 45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
    vTaskDelay(pdMS_TO_TICKS((int)moveDelay_));
    
    // 交换动作
    servo_->setAngle(servo_->getChannel(0), 135); // 左前腿抬起
    servo_->setAngle(servo_->getChannel(1), 45);  // 右前腿压低
    servo_->setAngle(servo_->getChannel(2), 45);  // 左后腿压低
    servo_->setAngle(servo_->getChannel(3), 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
}

void ServoControl::dance() {
    printf("小狗跳舞\n");
    // 前腿舵机
    servo_->setAngle(servo_->getChannel(0), 135); // 左前腿抬起
    servo_->setAngle(servo_->getChannel(1), 45);  // 右前腿压低
    // 后腿舵机
    servo_->setAngle(servo_->getChannel(2), 45);  // 左后腿压低
    servo_->setAngle(servo_->getChannel(3), 135); // 右后腿抬起
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒

    // 交换动作
    servo_->setAngle(servo_->getChannel(0), 45);  // 左前腿压低
    servo_->setAngle(servo_->getChannel(1), 135); // 右前腿抬起
    servo_->setAngle(servo_->getChannel(2), 135); // 左后腿抬起
    servo_->setAngle(servo_->getChannel(3), 45);  // 右后腿压低
    vTaskDelay(pdMS_TO_TICKS(500)); // 延时 0.5 秒
}   
