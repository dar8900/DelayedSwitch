#include "DelayedSwitch.h"

#define N_SAMPLE        50
#define MVOLTS_TO_AMPS  100.0

Chrono TimerSwitch(Chrono::SECONDS), ShowInfoTimer(Chrono::SECONDS);

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
        if(buttonPressed == LONG_PRESSED && Timer != 0)
        {
            Timer = 0;
            Status = OFF;
        }
        else if(buttonPressed == PRESSED)
        {
            if(Timer < 5999)
                Timer++;
            Status = ON;
        }
        delay(25);
    }
}


void CURRENT_SENSOR_CTRL::setup()
{
    uint32_t SensRead = 0;
    for(int i = 0; i < N_SAMPLE; i++)
    {
        SensRead += analogRead(CURRENT_SENSOR);
    }
    analogReference = (SensRead / N_SAMPLE);
}

void CURRENT_SENSOR_CTRL::calcCurrent(float &Current, float &CurrentAvg)
{
    float Millivolt = 0.0, AnalogCurrRms = 0.0;
    for(int i = 0;i < N_SAMPLE; i++)
    {
        int Sample = analogRead(CURRENT_SENSOR);
        Sample -= analogReference;
        AnalogCurrRms += (float)(Sample * Sample);
    }
    AnalogCurrRms /= N_SAMPLE;
    AnalogCurrRms = sqrt(AnalogCurrRms);
    Millivolt = (AnalogCurrRms * 5.0) / 1.024;    
    Current = roundf((Millivolt / MVOLTS_TO_AMPS) * 100.0);
}


void OLED_CTRL::setup()
{
    Oled.init(0x3c);
    Oled.clear();
    Oled.startScreen();
}

void OLED_CTRL::showAllInfo(uint32_t Timer, bool Status, float Current, float CurrentAvg)
{
    // 4 righe con: stato del timer, stato presa, corrente instantanea e corrente media
    uint8_t Col = 0;
    char OledText[50];
    // String OledText = "";
    if(ShowInfoTimer.hasPassed(2, true))
    {
        if(infoRoll < MAX_ROLL)
            infoRoll++;
        else
            infoRoll = TIMER;
        Oled.clear();
    }
    if(infoRoll == TIMER)
    {
        snprintf(OledText, 50, "Stato timer: %02dh %02dm", Timer / 60, Timer % 60);
    }
    // else if(infoRoll == STATUS)
    // {
    //     OledText = "Stato switch: ";
    //     if(Status == ON)
    //     {
    //         OledText += "ACCESO";
    //     }
    //     else
    //     {
    //         OledText += "SPENTO";
    //     }
    // }
    // else if(infoRoll == CURRENT)
    // {
    //     OledText = "Corrente letta: ";
    //     // OledText += String(Current, 2);
    //     OledText += "A";
    // }
    // else if(infoRoll == CURRENT_AVG)
    // {
    //     OledText = "Corrente media: ";
    //     // OledText += String(CurrentAvg, 2);
    //     OledText += "A";        
    // }
    Col = (128 - strlen(OledText)) / 2;
    Oled.cursorTo(35, Col);
    Oled.printString(OledText);
}


void DELAYED_SWITCH::setup()
{
    Button.setup();
    Switch.setup();
    SwitchTimer.setup(switchTimer);
    CurrentSensor.setup();
    OledDisplay.setup();
}

void DELAYED_SWITCH::runDelayedSwitch()
{
    Button.checkButton(switchTimer, status);
    Switch.switchRele(status);
    SwitchTimer.updateTimer(switchTimer, status);
    CurrentSensor.calcCurrent(current, currentAvg);
    OledDisplay.showAllInfo(switchTimer, status, current, currentAvg);
}