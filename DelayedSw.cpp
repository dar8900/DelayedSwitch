#include "DelayedSwitch.h"

Chrono TimerSwitch(Chrono::SECONDS);

void RELE_CTRL::setup()
{
    pinMode(RELE_PIN, OUTPUT);
}

void RELE_CTRL::switchRele(bool Status)
{
    if(oldStatus != Status)
    {
        if(Status == ON)
        {
            digitalWrite(RELE_PIN, HIGH);
        }
        else
        {
            digitalWrite(RELE_PIN, LOW);
        }
        oldStatus = Status;        
    }
}


void TIMER_CTRL::setup(uint32_t &Timer)
{
    Timer = 0;
}

void TIMER_CTRL::updateTimer(uint32_t &Timer, bool &Status)
{
    if(Status == ON)
    {
        if(TimerSwitch.hasPassed(60, true))
        {
            if(Timer > 0)
            {
                Timer--;
            }
        }
        if(Timer == 0)
        {
            Status = OFF;
            TimerSwitch.restart();
        }
    }
}

void BUTTON_CTRL::setup()
{
    pinMode(BUTTON_PIN, INPUT);
}

void BUTTON_CTRL::checkButton(uint32_t &Timer, bool &Status)
{
    uint32_t Elapsed = 0;
    bool ButtonStatus = false;
    buttonPressed = NO_PRESS;
    ButtonStatus = (bool)digitalRead(BUTTON_PIN);
    if(ButtonStatus)
    {
        Elapsed = millis();
        while(ButtonStatus)
        {
            ButtonStatus = (bool)digitalRead(BUTTON_PIN);
            if(millis() - Elapsed > 1000)
            {
                buttonPressed = LONG_PRESSED;
                break;
            }
            else
            {
                buttonPressed = PRESSED;
            }
            
        }
        if(buttonPressed == LONG_PRESSED)
        {
            Timer = 0;
            Status = OFF;
        }
        else if(buttonPressed == PRESSED)
        {
            Timer++;
            Status = ON;
        }
        delay(25);
    }
}


void CURRENT_SENSOR::setup()
{

}

void CURRENT_SENSOR::calcCurrent(float &Current)
{
    
}