#ifndef LIGHTNING_DOG_H
#define LIGHTNING_DOG_H

#include "servocontrol.h"
#include "mcp_server.h"
#include <esp_log.h>

class LightningDog {
private:
    ServoControl* servoControl_;

public:
    explicit LightningDog(ServoControl* servoControl)
        : servoControl_(servoControl) {
        auto& mcp_server = McpServer::GetInstance();

        mcp_server.AddTool("self.lightning_dog.go_forward",
            "让小柴犬向前走",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->moveForward(5);
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.go_back",
            "让小柴犬向后退",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->moveBackward(5);
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.turn_left",
            "让小柴犬向左转",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->turnLeft();
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.turn_right",
            "让小柴犬向右转",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->turnRight();
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.stand_up",
            "让小柴犬立正",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->standUp();
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.sit_down",
            "让小柴犬坐下",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->sitDown();
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.lie_down",
            "让小柴犬趴下",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->lieDown();
                return true;
            });

        mcp_server.AddTool("self.lightning_dog.dance",
            "让小柴犬跳舞",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                servoControl_->dance(5);
                return true;
            });

        // 可选：初始化测试
        servoControl_->test0(0);
    }
};

#endif // LIGHTNING_DOG_H
