#ifdef __MODUS_TARGET_DESKTOP_CONTROLS__

#ifndef DESKTOPCONTROLS_H
#define DESKTOPCONTROLS_H

#include <SDL.h>
#include "Controls.h"

class DesktopControls : public Controls {
public:
  TBool Poll() final;

  void Rumble(TFloat aStrength, TInt aTime) override;
};

#endif // DESKTOPCONTROLS_H

#endif //__MODUS_TARGET_DESKTOP__
