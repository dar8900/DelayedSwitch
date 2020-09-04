#include "DelayedSwitch.h"

#define N_SAMPLE        50
#define MVOLTS_TO_AMPS  100.0

Chrono TimerSwitch(Chrono::SECONDS), ShowInfoTimer(Chrono::SECONDS), CurrentAvgTimer(Chrono::SECONDS);

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

void BUTTON_CTRL::checkButton(uint32_t &Timer, bool &Status, bool &TimerSetting)
{
    uint32_t Elapsed = 0;
    bool ButtonStatus = false;
    longPress = false;
    ButtonStatus = (bool)digitalRead(BUTTON_PIN);
    if(ButtonStatus)
    {
        TimerSetting = true;
        ShowInfoTimer.restart();
        Elapsed = millis();
        while(ButtonStatus)
        {
            ButtonStatus = (bool)digitalRead(BUTTON_PIN);
            if(millis() - Elapsed > 1000)
            {
                longPress = true;
                break;
            }            
        }
        if(longPress)
        {
            Timer = 0;
            Status = OFF;
            wasLongPressed = true;
        }
        else
        {
            if(!wasLongPressed)
            {
                if(Timer < 5999)
                    Timer++;
                Status = ON;
            }
            else
            {
                wasLongPressed = false;
            }
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
    Current = roundf((Millivolt / MVOLTS_TO_AMPS) * 100.0) / 100.0;
    currentAvgAcc += Current;
    avgCnt++;
    if(CurrentAvgTimer.hasPassed(30, true))
    {
        CurrentAvg = (currentAvgAcc / (float)avgCnt);
        currentAvgAcc = 0.0;
        avgCnt = 0;
    }
}


void OLED_CTRL::setup()
{
    Oled.init(0x3c);
    Oled.clear();
    Oled.startScreen();
}

void OLED_CTRL::showAllInfo(uint32_t Timer, bool Status, float Current, float CurrentAvg, bool &TimerSetting)
{
    char OledText1[20], OledText2[20];
    if(!TimerSetting)
    {
        if(ShowInfoTimer.hasPassed(5, true))
        {
            if(infoRoll < MAX_ROLL - 1)
                infoRoll++;
            else
                infoRoll = TIMER;
            Oled.clear();
        }
    }
    else
    {
        infoRoll = TIMER;
        if(ShowInfoTimer.hasPassed(20, true))
        {
            TimerSetting = false;
        }
    }
    
    if(infoRoll == TIMER)
    {
        char hour[5], minute[5];
        snprintf(hour, 5, "%02dh", Timer / 60);
        snprintf(minute, 5, "%02dm", Timer % 60);
        strcat(OledText2, hour);
        strcat(OledText2, minute);
        snprintf(OledText1, 20, "Stato timer:");
        snprintf(OledText2, 20, "%s", OledText2);
    }
    else if(infoRoll == STATUS)
    {
        snprintf(OledText1, 20, "Stato switch:");
        snprintf(OledText2, 20, "%s", Status == ON ? "ACCESO" : "SPENTO");
    }
    else if(infoRoll == CURRENT)
    {
        int16_t CurrentDec = (int16_t)(Current * 1000.0);
        snprintf(OledText1, 20, "Corrente letta:"); 
        snprintf(OledText2, 20, "%dmA", CurrentDec); 
    }
    else if(infoRoll == CURRENT_AVG)
    {
        int16_t CurrentDec = (int16_t)(CurrentAvg * 1000.0);
        snprintf(OledText1, 20, "Corrente media:");
        snprintf(OledText2, 20, "%dmA", CurrentDec);
    }
    Oled.cursorTo(strlen(OledText1), 3);
    Oled.printString(OledText1);
    Oled.cursorTo(55, 5);
    Oled.printString(OledText2);
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
    Button.checkButton(switchTimer, status, timerSetting);
    Switch.switchRele(status);
    SwitchTimer.updateTimer(switchTimer, status);
    CurrentSensor.calcCurrent(current, currentAvg);
    OledDisplay.showAllInfo(switchTimer, status, current, currentAvg, timerSetting);
}