#include "sdkconfig.h"
#include "iot/thing.h"
#include "board.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "servocontrol.h"

#include "boards/bread-compact-wifi-bot/config.h"

#define TAG "LightningDog"

namespace iot {

class LightningDog : public Thing {
private:
    light_mode_t light_mode_ = LIGHT_MODE_ALWAYS_ON;
    ServoControl servoControl_;
    void SendUartMessage(const char * command_str) {
        
    }

    void InitializeServo() {
        servoControl_.InitializeServo();
    }

public:
    LightningDog() : Thing("LightningDog", "可爱的小柴犬：有腿可以移动；可以调整灯光效果；支持彩虹灯，闪光灯，随机灯"), light_mode_(LIGHT_MODE_ALWAYS_ON) {
        InitializeServo();

        // 定义设备可以被远程执行的指令
        methods_.AddMethod("GoForward", "向前走", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.moveForward();
        });

        methods_.AddMethod("GoBack", "向后退", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.moveBackward();
        });

        methods_.AddMethod("TurnLeft", "向左转", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.turnLeft();
        });

        methods_.AddMethod("TurnRight", "向右转", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.turnRight();
        });

        methods_.AddMethod("StandUp", "立正", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.standUp();
        });

        methods_.AddMethod("SitDown", "坐下", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.sitDown();
        });

        methods_.AddMethod("Dance", "跳舞", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.dance();
        });

        methods_.AddMethod("GoForward", "打开彩虹灯", ParameterList(), [this](const ParameterList& parameters) {
            servoControl_.moveForward();
            light_mode_ = LIGHT_MODE_MAX;
        });
    }
};

} // namespace iot

DECLARE_THING(LightningDog);
