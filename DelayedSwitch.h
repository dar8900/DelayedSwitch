#ifndef DELAYED_SWIITCH_H
#define DELAYED_SWIITCH_H

#include <Arduino.h>
#include "SSD1306_minimal.h"
#include <Chrono.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define ON      true
#define OFF     false

#define TIMER_1     false
#define TIMER_10    true


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
        bool longPress = false, wasLongPressed = false;
        uint32_t lastPressedTime = 0;
        uint8_t lastPressedRepete = 0;
        const uint8_t MAX_PRESSED_REPETE = 3;
        bool timerIncrementModality = TIMER_1;
    public:
        void setup();
        void checkButton(uint32_t &Timer, bool &Status, bool &TimerSetting);
};

class OLED_CTRL
{
    private:
        enum
        {
            TIMER = 0,
            STATUS,
            // CURRENT,
            CURRENT_AVG,
            MAX_ROLL
        };
        SSD1306_Mini Oled;
        uint8_t infoRoll = TIMER;
        bool oldTimerSetting;
        bool oldSwitchStatus;
        // bool enablePageRool = true;
    public:
        void setup();     
        void showAllInfo(uint32_t Timer, bool Status, float Current, float CurrentAvg, bool &TimerSetting);
};

class CURRENT_SENSOR_CTRL
{
    private:
        int analogReference = 0;
        float currentAvgAcc;
        uint32_t avgCnt = 0;
        void calcAnalogRef();
    public:
        void setup();
        void calcCurrent(float &Current, float &CurrentAvg, bool SwitchStatus);
};

class DELAYED_SWITCH
{
    private:
        uint32_t switchTimer = 0;
        bool status;
        bool timerSetting;
        float current = 0.0;
        float currentAvg = 0.0;
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