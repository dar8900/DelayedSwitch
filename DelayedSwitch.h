#ifndef DELAYED_SWIITCH_H
#define DELAYED_SWIITCH_H

#include <Arduino.h>
#include <SSD1306_minimal.h>
#include <Chrono.h>
#include <EEPROM.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ON      true
#define OFF     false


#define RELE_PIN            1
#define BUTTON_PIN          3
#define CURRENT_SENSOR      A2

class RELE_CTRL
{
    private:
        bool oldStatus;
    public:
        void setup();
        void switchRele(bool Status);

};

class TIMER_CTRL
{
    public:
        void setup(uint32_t &Timer);
        void updateTimer(uint32_t &Timer, bool &Status);
};

class BUTTON_CTRL
{
    private:
        enum
        {
            PRESSED = 0,
            LONG_PRESSED,
            NO_PRESS
        };
        uint8_t buttonPressed = NO_PRESS;
        uint8_t oldButtonStatus = NO_PRESS;

    public:
        void setup();
        void checkButton(uint32_t &Timer, bool &Status);
};

class OLED_CTRL
{
    private:
        enum
        {
            TIMER = 0,
            STATUS,
            CURRENT,
            CURRENT_AVG,
            MAX_ROLL
        };
        SSD1306_Mini Oled;
        uint8_t infoRoll;
    public:
        void setup();
        void showAllInfo(uint32_t Timer, bool Status, float Current, float CurrentAvg);
};

class CURRENT_SENSOR_CTRL
{
    private:
        int analogReference = 0;
        float currentAvgAcc;
        uint32_t avgCnt = 0;
    public:
        void setup();
        void calcCurrent(float &Current, float &CurrentAvg);
};

class DELAYED_SWITCH
{
    private:
        uint32_t switchTimer;
        bool status;
        float current;
        float currentAvg;
        BUTTON_CTRL Button;
        RELE_CTRL Switch;
        TIMER_CTRL SwitchTimer;
        CURRENT_SENSOR_CTRL CurrentSensor;
        OLED_CTRL OledDisplay;
        
    public:
        void setup();
        void runDelayedSwitch();
};

#endif