#ifdef __MODUS_TARGET_GAMESHELL__

#include "GameShellControls.h"


GameShellControls::GameShellControls() : Controls() {
  // noop
}

GameShellControls::~GameShellControls() {
  // noop
}


TBool GameShellControls::Poll() {
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    TUint16 keys = bKeys;
    if (e.type == SDL_QUIT) {
      keys |= BUTTONQ;
    }

    if (e.type == SDL_KEYDOWN) {
//      printf("KEYDOWN %i\n", e.key.keysym.scancode);
      fflush(stdout);

      switch (e.key.keysym.scancode) {
        case SDL_SCANCODE_L:
          keys |= BUTTONR;
          break;
        case SDL_SCANCODE_H:
          keys |= BUTTONL;
          break;
        case SDL_SCANCODE_I:
          keys |= BUTTONX;
          break;
        case SDL_SCANCODE_U:
          keys |= BUTTONY;
          break;
        case SDL_SCANCODE_SPACE: // SELECT on ODROID
          keys |= BUTTON3;
          break;
        case SDL_SCANCODE_RETURN: // START on ODROID
          keys |= BUTTON4;
          break;
        case SDL_SCANCODE_K:
          keys |= BUTTONA;
          break;
        case SDL_SCANCODE_J:
          keys |= BUTTONB;
          break;
        case SDL_SCANCODE_UP:
          keys |= JOYUP;
          break;
        case SDL_SCANCODE_DOWN:
          keys |= JOYDOWN;
          break;
        case SDL_SCANCODE_LEFT:
          keys |= JOYLEFT;
          break;
        case SDL_SCANCODE_RIGHT:
          keys |= JOYRIGHT;
          break;
        case SDL_SCANCODE_ESCAPE:
          keys |= BUTTONQ;
          break;
        default:
          break;
      }
    }

    if (e.type == SDL_KEYUP) {
      switch (e.key.keysym.scancode) {
        // QUIT button, will never be set on target
        case SDL_SCANCODE_END:
          keys &= ~BUTTONR;
          break;
        case SDL_SCANCODE_HOME:
          keys &= ~BUTTONL;
          break;
        case SDL_SCANCODE_I:
          keys &= ~BUTTONX;
          break;
        case SDL_SCANCODE_U:
          keys &= ~BUTTONY;
          break;
        case SDL_SCANCODE_SPACE: // SELECT on ODROID
          keys &= ~BUTTON3;
          break;
        case SDL_SCANCODE_RETURN: // START on ODROID
          keys &= ~BUTTON4;
          break;
        case SDL_SCANCODE_K:
          keys &= ~BUTTONA;
          break;
        case SDL_SCANCODE_J:
          keys &= ~BUTTONB;
          break;
        case SDL_SCANCODE_UP:
          keys &= ~JOYUP;
          break;
        case SDL_SCANCODE_DOWN:
          keys &= ~JOYDOWN;
          break;
        case SDL_SCANCODE_LEFT:
          keys &= ~JOYLEFT;
          break;
        case SDL_SCANCODE_RIGHT:
          keys &= ~JOYRIGHT;
          break;
        case SDL_SCANCODE_ESCAPE:
          keys &= ~BUTTONQ;
          break;
        default:
          break;
      }
    }
    // cKeys are journaled if journaling is on!
    dKeys |= TUint16(TUint16(keys) ^ TUint16(TUint16(cKeys) & TUint16(keys)));
    cKeys = keys;
    bKeys = keys;
  }

  return false;
}

#endif
