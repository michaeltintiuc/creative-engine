#ifdef __MODUS_TARGET_DESKTOP_CONTROLS__

#include "DesktopControls.h"

#if CONTROLLER_SUPPORT
#include "Gamepad.h"
#endif

void DesktopControls::Rumble(TFloat aStrength, TInt aTime) {
#if CONTROLLER_SUPPORT
  RumbleGamepad(aStrength, aTime);
#else
  (void)aStrength;
  (void)aTime;
#endif
}

TBool DesktopControls::Poll() {
#if CONTROLLER_SUPPORT
  const TUint16 gamepadKeys = ReadGamepad();
#else
  const TUint16 gamepadKeys = 0;
#endif
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    TUint16 keys = bKeys;
    if (e.type == SDL_QUIT) {
      keys |= BUTTONQ;
    }

    // Keyboard
    if (e.type == SDL_KEYDOWN) {
      switch (e.key.keysym.scancode) {
        // QUIT button, will never be set on target
#ifndef __EMSCRIPTEN__
        case SDL_SCANCODE_Q:
          keys |= BUTTONQ;
          break;
#endif
        case SDL_SCANCODE_1:
          keys |= BUTTON1;
          break;
        case SDL_SCANCODE_2:
          keys |= BUTTON2;
          break;
        case SDL_SCANCODE_3:
          keys |= BUTTON3;
          break;
        case SDL_SCANCODE_4:
        case SDL_SCANCODE_RETURN:
          keys |= BUTTON4;
          break;
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_X:
        case SDL_SCANCODE_F:
          keys |= BUTTONA;
          break;
//        case SDL_SCANCODE_SPACE:
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_Z:
        case SDL_SCANCODE_D:
          keys |= BUTTONB;
          break;
        case SDL_SCANCODE_S:
          keys |= BUTTONX;
          break;
        case SDL_SCANCODE_A:
          keys |= BUTTONY;
          break;
        case SDL_SCANCODE_K:
        case SDL_SCANCODE_UP:
          keys |= JOYUP;
          break;
        case SDL_SCANCODE_J:
        case SDL_SCANCODE_DOWN:
          keys |= JOYDOWN;
          break;
        case SDL_SCANCODE_H:
        case SDL_SCANCODE_LEFT:
          keys |= JOYLEFT;
          break;
        case SDL_SCANCODE_L:
        case SDL_SCANCODE_RIGHT:
          keys |= JOYRIGHT;
          break;
        case SDL_SCANCODE_TAB:
        case SDL_SCANCODE_COMMA:
          keys |= BUTTONL;
          break;
        case SDL_SCANCODE_BACKSPACE:
        case SDL_SCANCODE_RALT:
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_SLASH:
        case SDL_SCANCODE_PERIOD:
          keys |= BUTTONR;
          break;
        default:
          break;
      }
    }
    if (e.type == SDL_KEYUP) {
      switch (e.key.keysym.scancode) {
        // QUIT button, will never be set on target
#ifndef __EMSCRIPTEN__
        case SDL_SCANCODE_Q:
          keys &= TUint16(~BUTTONQ);
          break;
#endif
        case SDL_SCANCODE_1:
          keys &= TUint16(~BUTTON1);
          break;
        case SDL_SCANCODE_2:
          keys &= TUint16(~BUTTON2);
          break;
        case SDL_SCANCODE_3:
          keys &= TUint16(~BUTTON3);
          break;
        case SDL_SCANCODE_4:
        case SDL_SCANCODE_RETURN:
          keys &= TUint16(~BUTTON4);
          break;
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_X:
        case SDL_SCANCODE_F:
          keys &= TUint16(~BUTTONA);
          break;
//        case SDL_SCANCODE_SPACE:
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_Z:
        case SDL_SCANCODE_D:
          keys &= TUint16(~BUTTONB);
          break;
        case SDL_SCANCODE_S:
          keys &= TUint16(~BUTTONX);
          break;
        case SDL_SCANCODE_A:
          keys &= TUint16(~BUTTONY);
          break;
        case SDL_SCANCODE_K:
        case SDL_SCANCODE_UP:
          keys &= TUint16(~JOYUP);
          break;
        case SDL_SCANCODE_J:
        case SDL_SCANCODE_DOWN:
          keys &= TUint16(~JOYDOWN);
          break;
        case SDL_SCANCODE_H:
        case SDL_SCANCODE_LEFT:
          keys &= TUint16(~JOYLEFT);
          break;
        case SDL_SCANCODE_L:
        case SDL_SCANCODE_RIGHT:
          keys &= TUint16(~JOYRIGHT);
          break;
        case SDL_SCANCODE_TAB:
        case SDL_SCANCODE_COMMA:
          keys &= TUint16(~BUTTONL);
          break;
        case SDL_SCANCODE_BACKSPACE:
        case SDL_SCANCODE_RALT:
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_SLASH:
        case SDL_SCANCODE_PERIOD:
          keys &= TUint16(~BUTTONR);
          break;
        default:
          break;
      }
    }
    bKeys = keys;
    const TUint16 combined = keys | gamepadKeys;
    dKeys |= combined & ~cKeys;
    cKeys = combined;
  }

  const TUint16 combined = bKeys | gamepadKeys;
  dKeys |= combined & ~cKeys;
  cKeys = combined;
  return false;
}

#endif
