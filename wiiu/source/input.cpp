#include "input.hpp"

void Input::initialize()
{
    WPADInit();
    KPADInit();

    // TODO: Adjust deadzone?

    getInstance().kpads.resize(4);
}

void Input::finalize()
{
    WPADShutdown();
}

void Input::update()
{
    VPADStatus vstatus;
    VPADReadError verror;
    VPADRead(VPADChan::VPAD_CHAN_0, &vstatus, 1, &verror);
    if (verror == VPADReadError::VPAD_READ_SUCCESS)
    {
        getInstance().vpad = vstatus;
    }

    // TODO: kpads
}

touchPosition Input::getTouch()
{
    VPADTouchData rawTouch = getInstance().vpad.tpNormal;
    VPADTouchData touch;
    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &touch, &rawTouch);

    return touch;
}

uint32_t Input::getHeld()
{
    // TODO: kpads
    return getInstance().vpad.hold;
}

uint32_t Input::getDown()
{
    // TODO: kpads
    return getInstance().vpad.trigger;
}