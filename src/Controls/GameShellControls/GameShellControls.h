#ifdef __MODUS_TARGET_GAMESHELL__

#ifndef GAMESHELLCONTROLS_H
#define GAMESHELLCONTROLS_H


#include <SDL.h>
#include "Controls.h"

class GameShellControls : public Controls {
public:
  GameShellControls();
  ~GameShellControls();

  TBool Poll() final;
};

#endif // GAMESHELLCONTROLS_H

#endif //__MODUS_TARGET_GAMESHELL__
