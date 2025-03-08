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
    ServoControl servoControl_;
    
    void InitializeServo() {
        servoControl_.InitializeServo();
    }

    static void servo_test_task(void *arg)
    {
        auto* dog = static_cast<LightningDog*>(arg);
        dog->servoControl_.test0(0);
    }

public:
    LightningDog() : Thing("LightningDog", "萌萌小柴犬：可以做有趣的动作；可以向前走，向后退，向左转，向右转，立正，坐下，跳舞等动作") {
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
        servoControl_.test0(3);
        // xTaskCreate(servo_test_task, "servo_test_task", 2048, this, 5, NULL);
    }
};

} // namespace iot

DECLARE_THING(LightningDog);
