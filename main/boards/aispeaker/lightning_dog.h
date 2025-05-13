#ifndef LIGHTNING_DOG_H
#define LIGHTNING_DOG_H

#include "iot/thing.h"
#include "servocontrol.h"

class LightningDog : public iot::Thing {
public:
    explicit LightningDog(ServoControl* servoControl);
private:
    ServoControl* servoControl_;
};

#endif // LIGHTNING_DOG_H
