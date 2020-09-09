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
                if(lastPressedRepete < MAX_PRESSED_REPETE - 1)
                {
                    if(lastPressedRepete == 0)
                    {
                        lastPressedTime = millis();
                    }
                    lastPressedRepete++;
                }
                else
                {
                    lastPressedTime = millis() - lastPressedTime;
                    lastPressedRepete = 0;
                    if(lastPressedTime <= (500 * MAX_PRESSED_REPETE))
                    {
                        if(Timer + 10 < 5999)
                        {
                            Timer += 10;
                        }
                        else
                        {
                            Timer = 5999;
                        }
                    }
                }
                if(Timer < 5999)
                {
                    Timer++;
                }
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

void CURRENT_SENSOR_CTRL::calcAnalogRef()
{
    uint32_t SensRead = 0;
    for(int i = 0; i < N_SAMPLE; i++)
    {
        SensRead += analogRead(CURRENT_SENSOR);
    }
    analogReference = (SensRead / N_SAMPLE);
}

void CURRENT_SENSOR_CTRL::setup()
{
    calcAnalogRef();
}

void CURRENT_SENSOR_CTRL::calcCurrent(float &Current, float &CurrentAvg, bool SwitchStatus)
{
    float Millivolt = 0.0, AnalogCurrRms = 0.0;
    if(SwitchStatus == ON)
    {
        for(int i = 0;i < N_SAMPLE; i++)
        {
            int Sample = analogRead(CURRENT_SENSOR);
            Sample -= analogReference;
            AnalogCurrRms += (float)(Sample * Sample);
        }
        AnalogCurrRms /= N_SAMPLE;
        AnalogCurrRms = sqrt(AnalogCurrRms);
        Millivolt = (AnalogCurrRms * 5.0) / 1.024;    
        Current = (roundf((Millivolt / MVOLTS_TO_AMPS) * 100.0) / 100.0);
        Current -= 0.07;
        currentAvgAcc += Current;
        avgCnt++;
        if(CurrentAvgTimer.hasPassed(5, true))
        {
            CurrentAvg = (currentAvgAcc / (float)avgCnt);
            currentAvgAcc = 0.0;
            avgCnt = 0;
        }
    }
    else
    {
        calcAnalogRef();
        Current = 0.0;
        CurrentAvg = 0.0;
        CurrentAvgTimer.restart();
    }
}


void OLED_CTRL::setup()
{
    Oled.init(0x3c);
    Oled.clear();
    Oled.startScreen();
    Oled.cursorTo(20, 4);
    Oled.printString("Delayed Switch");
    Oled.cursorTo(40, 5);
    Oled.printString("starting...");
    delay(1500);
    Oled.clear();
}

void OLED_CTRL::showAllInfo(uint32_t Timer, bool Status, float Current, float CurrentAvg, bool &TimerSetting)
{
    char OledText1[20], OledText2[20];
    char hour[4], minute[4];
    bool IsTimer = false;
    if(Status == ON)
    {
        if(oldTimerSetting != TimerSetting)
        {
            oldTimerSetting = TimerSetting;
            Oled.clear();
        }
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
            if(ShowInfoTimer.hasPassed(15, true))
            {
                TimerSetting = false;
                infoRoll = STATUS;
            }
        }
    }
    else
    {
        infoRoll = TIMER;
        ShowInfoTimer.restart();
    }
    
    if(infoRoll == TIMER)
    {
        snprintf(hour, 4, "%02dh", Timer / 60);
        snprintf(minute, 4, "%02dm", Timer % 60);
        if(Status == ON && !TimerSetting)
        {
            snprintf(OledText1, 20, "Tempo mancante:");
        }
        else
        {
            snprintf(OledText1, 20, "Impostare timer:");
        }
        IsTimer = true;
    }
    else if(infoRoll == STATUS)
    {
        snprintf(OledText1, 20, "Stato switch:");
        snprintf(OledText2, 20, "%s", Status == ON ? "ACCESO" : "SPENTO");
    }
    // else if(infoRoll == CURRENT)
    // {
    //     int16_t CurrentDec = (int16_t)(Current * 1000.0);
    //     snprintf(OledText1, 20, "Corrente letta:"); 
    //     snprintf(OledText2, 20, "%dmA", CurrentDec); 
    // }
    else if(infoRoll == CURRENT_AVG)
    {
        int16_t CurrentDec = (int16_t)(CurrentAvg * 1000.0);
        snprintf(OledText1, 20, "Corrente media:");
        snprintf(OledText2, 20, "%dmA", CurrentDec);
    }
    Oled.cursorTo(strlen(OledText1), 3);
    Oled.printString(OledText1);
    if(!IsTimer)
    {
        Oled.cursorTo(55, 5);
        Oled.printString(OledText2);
    }
    else
    {
        Oled.cursorTo(50, 5);
        Oled.printString(hour);
        Oled.cursorTo(70, 5);
        Oled.printString(minute);
    }
    
}


void DELAYED_SWITCH::setup()
{
    Button.setup();
    Switch.setup();
    CurrentSensor.setup();
    OledDisplay.setup();
}

void DELAYED_SWITCH::runDelayedSwitch()
{
    Button.checkButton(switchTimer, status, timerSetting);
    Switch.switchRele(status);
    SwitchTimer.updateTimer(switchTimer, status);
    CurrentSensor.calcCurrent(current, currentAvg, status);
    OledDisplay.showAllInfo(switchTimer, status, current, currentAvg, timerSetting);
}