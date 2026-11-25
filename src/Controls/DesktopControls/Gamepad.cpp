#include "Gamepad.h"

#if defined(__MODUS_TARGET_DESKTOP_CONTROLS__) && CONTROLLER_SUPPORT
#ifdef __EMSCRIPTEN__

#include <cstring>
#include <emscripten.h>
#include <emscripten/html5.h>

TUint16 ReadGamepad() {
  if (emscripten_sample_gamepad_data() != EMSCRIPTEN_RESULT_SUCCESS) {
    return 0;
  }

  constexpr TUint16 buttons[] = {
    BUTTONB, BUTTONA, BUTTONX, BUTTONY, BUTTONL, BUTTONR, 0, 0,
    BUTTON_SELECT, BUTTON_START, 0, 0, JOYUP, JOYDOWN, JOYLEFT, JOYRIGHT
  };
  constexpr double deadzone = 0.73;
  const int count = emscripten_get_num_gamepads();
  for (int index = 0; index < count; ++index) {
    EmscriptenGamepadEvent pad{};
    if (emscripten_get_gamepad_status(index, &pad) != EMSCRIPTEN_RESULT_SUCCESS ||
        !pad.connected || std::strcmp(pad.mapping, "standard") != 0) {
      continue;
    }

    TUint16 keys = 0;
    for (int button = 0; button < pad.numButtons && button < 16; ++button) {
      if (pad.digitalButton[button]) {
        keys |= buttons[button];
      }
    }
    if (pad.numAxes >= 2) {
      if (pad.axis[0] < -deadzone) keys |= JOYLEFT;
      if (pad.axis[0] > deadzone) keys |= JOYRIGHT;
      if (pad.axis[1] < -deadzone) keys |= JOYUP;
      if (pad.axis[1] > deadzone) keys |= JOYDOWN;
    }
    return keys;
  }
  return 0;
}

EM_JS(void, RumbleWebGamepad, (float strength, int duration), {
  void (async () => {
    const gamepads = navigator.getGamepads?.();
    const gamepad = gamepads && Array.from(gamepads).find(
      pad => pad?.connected && pad.mapping === "standard"
    );
    if (!gamepad) return;

    const actuator = gamepad.vibrationActuator;
    if (actuator?.playEffect) {
      try {
        const result = await actuator.playEffect("dual-rumble", {
          duration,
          startDelay: 0,
          strongMagnitude: strength,
          weakMagnitude: strength
        });
        if (result === "complete" || result === "preempted") return;
      } catch {
        // Some controllers expose a pulse actuator even when dual rumble fails.
      }
    }

    const pulseActuator = actuator?.pulse ? actuator :
      Array.from(gamepad.hapticActuators ?? []).find(haptic => haptic?.pulse);
    await pulseActuator?.pulse(strength, duration);
  })().catch(() => {
    // Haptics are optional, and access can be blocked by the browser or embed.
  });
});

void RumbleGamepad(TFloat strength, TInt duration) {
  RumbleWebGamepad(strength, duration);
}

#else

#include <SDL.h>
#include <algorithm>
#include <memory>

namespace {
using Controller = std::unique_ptr<SDL_GameController, decltype(&SDL_GameControllerClose)>;

Controller &ActiveController() {
  static Controller controller(nullptr, SDL_GameControllerClose);
  return controller;
}
}

TUint16 ReadGamepad() {
  static const bool initialized = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0;
  if (!initialized) {
    return 0;
  }

  auto &controller = ActiveController();
  SDL_PumpEvents();
  if (controller && !SDL_GameControllerGetAttached(controller.get())) {
    controller.reset();
  }

  if (!controller) {
    for (int index = 0; index < SDL_NumJoysticks(); ++index) {
      if (SDL_IsGameController(index)) {
        controller.reset(SDL_GameControllerOpen(index));
        if (controller) {
          break;
        }
      }
    }
  }

  if (!controller) {
    return 0;
  }

  constexpr TUint16 buttons[] = {
    BUTTONB, BUTTONA, BUTTONX, BUTTONY, BUTTON_SELECT, BUTTONQ, BUTTON_START,
    0, 0, BUTTONL, BUTTONR, JOYUP, JOYDOWN, JOYLEFT, JOYRIGHT
  };
  static_assert(SDL_CONTROLLER_BUTTON_DPAD_RIGHT + 1 == sizeof(buttons) / sizeof(buttons[0]),
                "Unexpected SDL controller button order");

  TUint16 keys = 0;
  for (int index = 0; index <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT; ++index) {
    if (SDL_GameControllerGetButton(controller.get(), static_cast<SDL_GameControllerButton>(index))) {
      keys |= buttons[index];
    }
  }

  constexpr Sint16 deadzone = 24000;
  const Sint16 x = SDL_GameControllerGetAxis(controller.get(), SDL_CONTROLLER_AXIS_LEFTX);
  const Sint16 y = SDL_GameControllerGetAxis(controller.get(), SDL_CONTROLLER_AXIS_LEFTY);
  if (x < -deadzone) keys |= JOYLEFT;
  if (x > deadzone) keys |= JOYRIGHT;
  if (y < -deadzone) keys |= JOYUP;
  if (y > deadzone) keys |= JOYDOWN;
  return keys;
}

void RumbleGamepad(TFloat strength, TInt duration) {
#if SDL_VERSION_ATLEAST(2, 0, 9)
  auto &controller = ActiveController();
  if (!controller || !SDL_GameControllerGetAttached(controller.get())) {
    return;
  }

  const float normalized = std::max(0.0f, std::min(strength, 1.0f));
  const Uint16 magnitude = static_cast<Uint16>(normalized * 65535.0f);
  SDL_GameControllerRumble(controller.get(), magnitude, magnitude, std::max(0, duration));
#else
  (void)strength;
  (void)duration;
#endif
}

#endif
#endif
