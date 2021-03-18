#pragma once
#ifndef MYGAMEPAD_H_
#define MYGAMEPAD_H_

#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <Xinput.h> //add "/DYNAMICBASE "xinput.lib"  in linker command line option.

using std::max;
class MyGamepad
{
private:
    int cId;
    XINPUT_STATE state;
    float deadzoneX;
    float deadzoneY;
public:
    MyGamepad() : deadzoneX(0.05f), deadzoneY(0.02f) {}
    MyGamepad(float dzX, float dzY) : deadzoneX(dzX), deadzoneY(dzY) {}
    float leftStickX;
    float leftStickY;
    float rightStickX;
    float rightStickY;
    float leftTrigger;
    float rightTrigger;
    float vaai;
    int GetPort();
    XINPUT_GAMEPAD* GetState();
    bool CheckConnection();
    bool Refresh();
    bool IsPressed(WORD);
};
#endif