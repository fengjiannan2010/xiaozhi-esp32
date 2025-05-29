

#include "iot/thing.h"
#include "servocontrol.h"
#include <esp_log.h>

using namespace iot;

class LightningDog : public iot::Thing {
public:
    explicit LightningDog(ServoControl* servoControl):
        Thing("LightningDog", "萌萌小柴犬：可以做有趣的动作；可以向前走，向后退，向左转，向右转，立正，坐下，跳舞等动作"),
        servoControl_(servoControl) {

        methods_.AddMethod("GoForward", "向前走", ParameterList(), [this](const ParameterList&) {
            servoControl_->moveForward(5);
        });

        methods_.AddMethod("GoBack", "向后退", ParameterList(), [this](const ParameterList&) {
            servoControl_->moveBackward(5);
        });

        methods_.AddMethod("TurnLeft", "向左转", ParameterList(), [this](const ParameterList&) {
            servoControl_->turnLeft();
        });

        methods_.AddMethod("TurnRight", "向右转", ParameterList(), [this](const ParameterList&) {
            servoControl_->turnRight();
        });

        methods_.AddMethod("StandUp", "立正", ParameterList(), [this](const ParameterList&) {
            servoControl_->standUp();
        });

        methods_.AddMethod("SitDown", "坐下", ParameterList(), [this](const ParameterList&) {
            servoControl_->sitDown();
        });

        methods_.AddMethod("LieDown", "趴下", ParameterList(), [this](const ParameterList&) {
            servoControl_->lieDown();
        });

        methods_.AddMethod("Dance", "跳舞", ParameterList(), [this](const ParameterList&) {
            servoControl_->dance(5);
        });

        servoControl_->test0(0);
    }
private:
    ServoControl* servoControl_;
};