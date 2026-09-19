#include "Player.h"

Player::Player(const char* name, ButtonInput& input, Lights& lights)
    : _name(name), _input(&input), _lights(&lights) {}

void Player::Begin() {
  _input->Begin();
  _lights->Begin();
  _lights->Clear();
}

void Player::Reset() {
  _total_score = 0;
  _mole_index = -1;
  _lamp_on = false;
  _is_ready = false;
  _lights->Clear();
  _input->ClearPresses();
}

void Player::Ready(uint32_t now_ms) {
  if (_is_ready) {
    return;
  }
  _is_ready = true;
  _last_blink_ms = now_ms;
}

void Player::NotReady() {
  _is_ready = false;
  _mole_index = -1;
  _lamp_on = false;
  _lights->Clear();
}

bool Player::TakeStartPress() { return _input->TakePress(config::kStartButtonIndex); }

int Player::CurrentPoints(uint32_t now_ms) const {
  const uint32_t elapsed = now_ms - _mole_started_ms;
  if (elapsed >= config::kMoleLifetimeMs) {
    return 0;
  }
  const uint32_t decayed =
      ((uint32_t) config::kMoleStartingPoints * elapsed) / config::kMoleLifetimeMs;
  return config::kMoleStartingPoints - (int) decayed;
}

void Player::NewMole(uint32_t now_ms) {
  if (!_is_ready) {
    return;
  }

  // Turn off whatever was lit. Guarded, because on the first mole of a game
  // there is no previous index -- the original code indexed the pin array with
  // -1 here.
  if (_mole_index >= 0) {
    _lights->Set((uint8_t) _mole_index, colors::kOff);
  }

  // Never pick the same cap twice in a row. Without this a player can lean on
  // one button and keep scoring whenever random() repeats.
  int8_t next = _mole_index;
  while (next == _mole_index) {
    next = (int8_t) random(0, config::kButtonsPerPlayer);
  }
  _mole_index = next;

  _lights->Set((uint8_t) _mole_index, colors::kMole);
  _lights->Show();
  _lamp_on = true;

  _mole_started_ms = now_ms;
  _last_blink_ms = now_ms;

  // A press that landed while the previous mole was expiring must not be
  // credited against the new one.
  _input->ClearPresses();
}

void Player::UpdateBlink(uint32_t now_ms) {
  if (_mole_index < 0) {
    return;
  }
  if (now_ms - _last_blink_ms < config::kLedBlinkMs) {
    return;
  }

  _last_blink_ms = now_ms;
  _lamp_on = !_lamp_on;
  _lights->Set((uint8_t) _mole_index, _lamp_on ? colors::kMole : colors::kOff);
  _lights->Show();
}

void Player::ApplyPenalty() {
  _total_score -= config::kWrongButtonPenalty;
  // Floor at zero. The original guard skipped the penalty entirely whenever
  // the score was below the penalty amount, so a losing player was immune.
  if (_total_score < 0) {
    _total_score = 0;
  }
}

void Player::Update(uint32_t now_ms) {
  if (!_is_ready) {
    return;
  }

  UpdateBlink(now_ms);

  // Snapshot the target before consuming presses, so a hit that spawns a new
  // mole cannot make a same-tick press on another cap look like a miss against
  // the mole that has not been shown yet.
  const int8_t target = _mole_index;
  bool scored = false;

  for (uint8_t i = 0; i < config::kButtonsPerPlayer; i++) {
    if (!_input->TakePress(i)) {
      continue;
    }

    if ((int8_t) i == target && !scored) {
      scored = true;
      _total_score += CurrentPoints(now_ms);
    } else {
      ApplyPenalty();
    }
  }

  if (scored) {
    NewMole(now_ms);
    return;
  }

  // Nobody got there in time; move the mole on.
  if (now_ms - _mole_started_ms >= config::kMoleLifetimeMs) {
    NewMole(now_ms);
  }
}
