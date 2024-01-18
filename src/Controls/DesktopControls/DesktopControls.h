#ifdef __MODUS_TARGET_DESKTOP_CONTROLS__

#ifndef DESKTOPCONTROLS_H
#define DESKTOPCONTROLS_H

#include <SDL.h>
#include "Controls.h"

#ifdef CONTROLLER_SUPPORT
#define CONTROLLER_SUPPORT
#define CONTROLLER_AXIS_MIN 24000
#endif

class DesktopControls : public Controls {
public:
  DesktopControls();
  ~DesktopControls();

  TBool Poll() final;

#ifdef CONTROLLER_SUPPORT
  void Rumble(TFloat aStrength, TInt aTime);

  SDL_Haptic         *haptic;
  SDL_GameController *ctrl;
#endif
};

#endif // DESKTOPCONTROLS_H

#endif //__MODUS_TARGET_DESKTOP__
