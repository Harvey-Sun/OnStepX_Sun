// ---------------------------------------------------------------------------------------------------
// Button Panel - independent direction buttons and home/speed toggle button

#include "ButtonPanel.h"

#ifdef MOUNT_PRESENT

#if BUTTON_PANEL == ON

#include "../../../lib/tasks/OnTask.h"
#include "../../../lib/pushButton/PushButton.h"

#include "../../Telescope.h"
#include "../Mount.h"
#include "../guide/Guide.h"
#include "../home/Home.h"
#include "../goto/Goto.h"
#include "../status/Status.h"

// debounce time for direction buttons
#define BTN_DEBOUNCE_MS 5L

// For input-only pins (GPIO 34,35,36,39) that lack internal pull-up,
// external 10kΩ pull-up resistors to 3.3V are required.
// We still set INPUT_PULLUP for GPIO 25,32,33 which do support it.
#if BTN_NORTH_PIN == 25 || BTN_NORTH_PIN == 32 || BTN_NORTH_PIN == 33
  #define BTN_NORTH_INIT INPUT_PULLUP
#else
  #define BTN_NORTH_INIT INPUT
#endif

#if BTN_SOUTH_PIN == 25 || BTN_SOUTH_PIN == 32 || BTN_SOUTH_PIN == 33
  #define BTN_SOUTH_INIT INPUT_PULLUP
#else
  #define BTN_SOUTH_INIT INPUT
#endif

#if BTN_EAST_PIN == 25 || BTN_EAST_PIN == 32 || BTN_EAST_PIN == 33
  #define BTN_EAST_INIT INPUT_PULLUP
#else
  #define BTN_EAST_INIT INPUT
#endif

#if BTN_WEST_PIN == 25 || BTN_WEST_PIN == 32 || BTN_WEST_PIN == 33
  #define BTN_WEST_INIT INPUT_PULLUP
#else
  #define BTN_WEST_INIT INPUT
#endif

#if BTN_HOME_PIN == 25 || BTN_HOME_PIN == 32 || BTN_HOME_PIN == 33
  #define BTN_HOME_INIT INPUT_PULLUP
#else
  #define BTN_HOME_INIT INPUT
#endif

// Direction buttons: pressed = LOW (button connects pin to GND)
Button btnNorth(BTN_NORTH_PIN, BTN_NORTH_INIT, LOW | HYST(BTN_DEBOUNCE_MS));
Button btnSouth(BTN_SOUTH_PIN, BTN_SOUTH_INIT, LOW | HYST(BTN_DEBOUNCE_MS));
Button btnEast(BTN_EAST_PIN, BTN_EAST_INIT, LOW | HYST(BTN_DEBOUNCE_MS));
Button btnWest(BTN_WEST_PIN, BTN_WEST_INIT, LOW | HYST(BTN_DEBOUNCE_MS));

// Home/Speed toggle button
Button btnHome(BTN_HOME_PIN, BTN_HOME_INIT, LOW | HYST(BTN_DEBOUNCE_MS));

void buttonPanelWrapper() {
  buttonPanel.poll();
}

void ButtonPanel::init() {
  VF("MSG: Mount, ButtonPanel start monitor task (rate 5ms priority 1)... ");
  if (tasks.add(5, 0, true, 1, buttonPanelWrapper, "BtnPanl")) { VLF("success"); } else { VLF("FAILED!"); }
}

void ButtonPanel::poll() {
  // poll all buttons
  btnNorth.poll();
  btnSouth.poll();
  btnEast.poll();
  btnWest.poll();
  btnHome.poll();

  // determine current guide rate based on speed mode
  GuideRateSelect currentRate = isFastMode ? BUTTON_PANEL_FAST_RATE : BUTTON_PANEL_FINE_RATE;

  // --- Direction buttons: East/West (Axis1 RA) ---

  // safety: if both E and W pressed, stop axis1
  if (btnEast.isDown() && btnWest.isDown()) {
    guide.stopAxis1();
  }

  GuideAction btnGuideActionAxis1 = GA_BREAK;
  static GuideAction lastBtnGuideActionAxis1 = GA_BREAK;

  if (btnWest.isDown() && btnEast.isUp()) btnGuideActionAxis1 = GA_FORWARD;
  if (btnEast.isDown() && btnWest.isUp()) btnGuideActionAxis1 = GA_REVERSE;

  if (btnGuideActionAxis1 != lastBtnGuideActionAxis1) {
    lastBtnGuideActionAxis1 = btnGuideActionAxis1;
    if (btnGuideActionAxis1 != GA_BREAK) {
      #if GOTO_FEATURE == ON
        if (goTo.isHomePaused()) goTo.homeContinue(); else
        if (goTo.state == GS_GOTO) goTo.abort(); else
      #endif
      guide.startAxis1(btnGuideActionAxis1, currentRate, GUIDE_TIME_LIMIT*1000);
    } else {
      guide.stopAxis1();
    }
  }

  // --- Direction buttons: North/South (Axis2 Dec) ---

  // safety: if both N and S pressed, stop axis2
  if (btnNorth.isDown() && btnSouth.isDown()) {
    guide.stopAxis2();
  }

  GuideAction btnGuideActionAxis2 = GA_BREAK;
  static GuideAction lastBtnGuideActionAxis2 = GA_BREAK;

  if (btnNorth.isDown() && btnSouth.isUp()) btnGuideActionAxis2 = GA_FORWARD;
  if (btnSouth.isDown() && btnNorth.isUp()) btnGuideActionAxis2 = GA_REVERSE;

  if (btnGuideActionAxis2 != lastBtnGuideActionAxis2) {
    lastBtnGuideActionAxis2 = btnGuideActionAxis2;
    if (btnGuideActionAxis2 != GA_BREAK) {
      #if GOTO_FEATURE == ON
        if (goTo.isHomePaused()) goTo.homeContinue(); else
        if (goTo.state == GS_GOTO) goTo.abort(); else
      #endif
      guide.startAxis2(btnGuideActionAxis2, currentRate, GUIDE_TIME_LIMIT*1000);
    } else {
      guide.stopAxis2();
    }
  }

  // --- Home/Speed toggle button ---

  if (btnHome.isDown()) {
    // long press detection: trigger home command once when threshold reached
    if (!homeLongPressTriggered && btnHome.timeDown() > BUTTON_PANEL_LONG_PRESS_MS) {
      homeLongPressTriggered = true;
      // stop any active guides first
      guide.stop();
      // request return to home position
      VLF("MSG: ButtonPanel, long press - requesting home");
      CommandError e = home.request();
      if (e != CE_NONE) {
        VF("WRN: ButtonPanel, home request failed with code "); VL(e);
      }
      mountStatus.soundBeep();
    }
  }

  if (btnHome.isUp()) {
    // short press: toggle speed mode (only if we didn't trigger long press)
    if (btnHome.wasPressed() && !homeLongPressTriggered) {
      isFastMode = !isFastMode;
      if (isFastMode) {
        VLF("MSG: ButtonPanel, speed mode: FAST");
      } else {
        VLF("MSG: ButtonPanel, speed mode: FINE");
      }
      mountStatus.soundClick();
    }
    // reset long press flag when button is released
    homeLongPressTriggered = false;
  }
}

ButtonPanel buttonPanel;

#endif

#endif
