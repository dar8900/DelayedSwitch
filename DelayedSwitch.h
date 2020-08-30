#ifndef DELAYED_SWIITCH_H
#define DELAYED_SWIITCH_H

#include <Arduino.h>
#include <SSD1306_minimal.h>
#include <Chrono.h>

class RELE_CTRL
{
    public:
        setup();


};

class BUTTON_CTRL
{
    public:
        setup();
};

class OLED_CTRL
{
    public:
        setup();
};

class CURRENT_SENSOR
{
    public:
        setup();
};

class DELAYED_SWITCH
{
    private:
        uint32_t switchTimer;
        
    public:
        setup();
};

#endif