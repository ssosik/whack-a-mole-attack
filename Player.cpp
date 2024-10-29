#include "Player.h"

//                                B    R    W    G    Y    P
const int BUTTON_THRESHOLDS[] = { 440, 307, 223, 142, 687, 1023 };
const int BUTTON_TOLERANCE = 20; // 200;
const int BUTTONDEBOUNCETICKS = 20; // 1000;

const int LEDBLINKTICKS = 20;

const int BUTTON_STARTING_POINTS = 1000;
const int BUTTON_POINT_DECREMENT = 1;
const int WRONG_BUTTON_POINT_DECREMENT = 500;
const int POINT_DECREMENT_TICK_INTERVAL = 2;

// Player Class Implementation
Player::Player(String name, int pin, const int button_leds[], const int numled, bool test_mode) {
  Serial.print(" New Player ");
  Serial.println(name);

  // Initialize LED pins and Turn off all LEDs
  for (int i = 0; _numled > i; i++) {
    pinMode(button_leds[i], OUTPUT);
    digitalWrite(button_leds[i], LOW);
  }

  // Set button input pin
  pinMode(pin, INPUT);

  // Set initial state
  _name = name;
  _input_pin = pin;
  _button_leds = button_leds;
  _numled = numled;
  _led_index = -1;
  _total_score = 0;
  _current_points = 0;
  _button_debounce_tick = 0;
  _last_increment_tick = 0;
  _last_led_tick = 0;
  _led_on = false;
  _is_ready = false;
  _test_mode = test_mode;
}

void Player::Update(long current_tick) {
  // If this player is not playing don't do any updates
  if (!_is_ready) {
    return;
  }

  BlinkLight(current_tick);

  if (current_tick >= POINT_DECREMENT_TICK_INTERVAL + _last_increment_tick) {
    // Decrement current points value; players get less points the
    // longer they take to press the button
    _current_points -= BUTTON_POINT_DECREMENT;
    _last_increment_tick = current_tick;
  }

  // Current points ran out, Light up a new Light
  if (_current_points <= 0) {
    Serial.print(" Player ");
    Serial.print(_name);
    Serial.print(" timeout; ");
    NewLight(current_tick);
    return;
  }

  if (CorrectButtonPressed(current_tick)) {
    // Update points
    AddPoints(_current_points);

    Serial.print(" Player ");
    Serial.print(_name);
    Serial.print(" scored ");
    Serial.print(_current_points);
    Serial.print(", total ");
    Serial.println(_total_score);

    if (!_test_mode) {
      // Light new LED
      NewLight(current_tick);
    }

    return;
  }
}

void Player::AddPoints(int points) {
  _total_score += points;
}

void Player::Reset() {
  _total_score = 0;
  // Turn off all LEDs
  for (int i = 0; _numled > i; i++) {
    digitalWrite(_button_leds[i], LOW);
  }
  _is_ready = false;
}

void Player::Ready() {
  _is_ready = true;
}

void Player::NotReady() {
  _is_ready = false;
}

void Player::Print() {
  Serial.print(" Player ");
  Serial.print(_name);
  Serial.print(" Score: ");
  Serial.println(_total_score);
}

// Handle turning on a new light
void Player::NewLight(long current_tick) {
  if (!_is_ready) {
    return;
  }

  _last_increment_tick = current_tick;
  // Turn off previous light
  digitalWrite(_button_leds[_led_index], LOW);

  // Turn on new light
  if (_test_mode) {
    // Cycle through the LEDs
    _led_index += 1;
    if (_led_index >= _numled) {
      _led_index = 0;
    }
  } else {
    _led_index = random(0, _numled);
  }
  digitalWrite(_button_leds[_led_index], HIGH);
  _led_on = true;

  Serial.print(" Player ");
  Serial.print(_name);
  Serial.print(" new light index: ");
  Serial.print(_led_index);
  Serial.print(" pin: ");
  Serial.println(_button_leds[_led_index]);

  // Handle state updates
  _current_points = BUTTON_STARTING_POINTS;
}

// Handle turning on a new light
void Player::BlinkLight(long current_tick) {
  if (_test_mode) {
    return;
  }

  if (_last_led_tick + LEDBLINKTICKS <= current_tick) {
    if (_led_on) {
      digitalWrite(_button_leds[_led_index], LOW);
      _led_on = false;
      _last_led_tick = current_tick;
    } else {
      digitalWrite(_button_leds[_led_index], HIGH);
      _led_on = true;
      _last_led_tick = current_tick;
    }
  }
}

int Player::Score() {
  return _total_score;
}

bool Player::CheckIfReady() {
  // Read the value from the input button pin
  int pinValue = analogRead(_input_pin);
  bool zero = (0 <= pinValue and pinValue <= BUTTON_TOLERANCE);
  if (zero) {
    return _is_ready;
  }

  int lower = BUTTON_THRESHOLDS[5] - BUTTON_TOLERANCE;
  int upper = BUTTON_THRESHOLDS[5] + BUTTON_TOLERANCE;

  return (lower <= pinValue and pinValue <= upper);
}

bool Player::CorrectButtonPressed(long tick) {
  // Read the value from the input button pin
  int pinValue = analogRead(_input_pin);
  bool zero = (0 <= pinValue and pinValue <= BUTTON_TOLERANCE);

  if (zero) {
    return false;
  }

  if (!Debounced(tick)) {
    return false;
  }

  // Determine if the pin value indicates the correct button has been pressed
  int lower = BUTTON_THRESHOLDS[_led_index] - BUTTON_TOLERANCE;
  int upper = BUTTON_THRESHOLDS[_led_index] + BUTTON_TOLERANCE;
  bool correct = (lower <= pinValue and pinValue <= upper);

  // if the WRONG button was pressed, decrement total score
  if (!correct) {
    if ((_total_score - WRONG_BUTTON_POINT_DECREMENT) > 0) {
      _total_score -= WRONG_BUTTON_POINT_DECREMENT;
    }
  }

  //Serial.print("Player ");
  //Serial.print(_name);
  //Serial.print(" had button press pin value ");
  //Serial.print(pinValue);
  //Serial.print("; is Correct? ");
  //Serial.print(correct);
  //Serial.print("\n");
  
  return correct;
}

// Return true if we've waited long enough to block out any extra/noisy clicks
bool Player::Debounced(long tick) {
  if ((_button_debounce_tick + BUTTONDEBOUNCETICKS) < tick) {;
    _button_debounce_tick = tick;
    //Serial.println("CLICK");
    return true;
  }
  //Serial.println("bounced");
  return false;
}


