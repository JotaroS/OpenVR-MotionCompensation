#pragma once
#include "MyGamepad.h"
int MyGamepad::GetPort()
{
    return cId + 1;
}
XINPUT_GAMEPAD* MyGamepad::GetState()
{
    return &state.Gamepad;
}
bool MyGamepad::CheckConnection()
{
    int controllerId = -1;
    for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
    {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
            controllerId = i;
    }
    cId = controllerId;
    return controllerId != -1;
}
// Returns false if the controller has been disconnected
bool MyGamepad::Refresh()
{
    if (cId == -1)
        CheckConnection();
    if (cId != -1)
    {
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        if (XInputGetState(cId, &state) != ERROR_SUCCESS)
        {
            cId = -1;
            return false;
        }
        float normLX = max(-1.0F, (float)state.Gamepad.sThumbLX / 32767);
        float normLY = max(-1.0F, (float)state.Gamepad.sThumbLY / 32767);
        leftStickX = (abs(normLX) < deadzoneX ? 0 : (abs(normLX) - deadzoneX) * (normLX / abs(normLX)));
        leftStickY = (abs(normLY) < deadzoneY ? 0 : (abs(normLY) - deadzoneY) * (normLY / abs(normLY)));
        if (deadzoneX > 0)
            leftStickX *= 1 / (1 - deadzoneX);
        if (deadzoneY > 0)
            leftStickY *= 1 / (1 - deadzoneY);
        float normRX = max(-1.0F, (float)state.Gamepad.sThumbRX / 32767);
        float normRY = max(-1.0F, (float)state.Gamepad.sThumbRY / 32767);
        rightStickX = (abs(normRX) < deadzoneX ? 0 : (abs(normRX) - deadzoneX) * (normRX / abs(normRX)));
        rightStickY = (abs(normRY) < deadzoneY ? 0 : (abs(normRY) - deadzoneY) * (normRY / abs(normRY)));
        if (deadzoneX > 0)
            rightStickX *= 1 / (1 - deadzoneX);
        if (deadzoneY > 0)
            rightStickY *= 1 / (1 - deadzoneY);
        leftTrigger = (float)state.Gamepad.bLeftTrigger / 255;
        rightTrigger = (float)state.Gamepad.bRightTrigger / 255;
        return true;
    }
    return false;
}
bool MyGamepad::IsPressed(WORD button)
{
    return (state.Gamepad.wButtons & button) != 0;
}