#include "DelayedSwitch.h"

DELAYED_SWITCH DelayedSwitch;

void setup()
{
    DelayedSwitch.setup();
}

void loop()
{
    DelayedSwitch.runDelayedSwitch();
}