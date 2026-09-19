#ifndef Player_h
#define Player_h

#include "ButtonInput.h"
#include "Config.h"
#include "Lights.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// One player's score, current mole, and lamp state.
//
// The Player no longer talks to pins directly; it drives a ButtonInput and a
// Lights, which is what lets the resistor ladder and the dome lamps be swapped
// for GPIOs and pixels without touching any of this.
// ---------------------------------------------------------------------------

class Player {
public:
  Player(const char* name, ButtonInput& input, Lights& lights);

  // Configure hardware. Called from setup(), not from the constructor.
  void Begin();

  // Advance one game tick. No-op unless the player is in the game.
  void Update(uint32_t now_ms);

  // Clear score and lamps and drop out of the game.
  void Reset();

  void Ready(uint32_t now_ms);
  void NotReady();
  bool IsReady() const { return _is_ready; }

  // True exactly once per press of this player's start button.
  bool TakeStartPress();

  // Light a fresh mole and restart its point decay.
  void NewMole(uint32_t now_ms);

  int Score() const { return _total_score; }
  const char* Name() const { return _name; }
  int8_t MoleIndex() const { return _mole_index; }

  // Exposed for the debug screens, which drive the hardware directly.
  ButtonInput& Input() { return *_input; }
  Lights& Lamps() { return *_lights; }

private:
  // Points a hit is worth right now: decays linearly from
  // kMoleStartingPoints to zero across kMoleLifetimeMs.
  int CurrentPoints(uint32_t now_ms) const;

  void UpdateBlink(uint32_t now_ms);
  void ApplyPenalty();

  const char* _name;
  ButtonInput* _input;
  Lights* _lights;

  int _total_score = 0;
  int8_t _mole_index = -1;
  bool _is_ready = false;

  uint32_t _mole_started_ms = 0;
  uint32_t _last_blink_ms = 0;
  bool _lamp_on = false;
};

#endif
