//--------------------------------------------------------------------------------------------------
// telescope mount control - button panel (direction + home/speed buttons)
#pragma once

#include "../../../Common.h"

#ifdef MOUNT_PRESENT

#if BUTTON_PANEL == ON

class ButtonPanel {
  public:
    void init();

    // monitor button panel for directional guiding and home/speed control
    void poll();

  private:
    bool isFastMode = false;                    // false = fine rate, true = fast rate
    bool homeLongPressTriggered = false;         // prevents repeated home triggers while held
};

extern ButtonPanel buttonPanel;

#endif

#endif
