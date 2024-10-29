#ifndef Player_h
#define Player_h
#include "Arduino.h"
class Player {
public:
  Player(String name, int pin, const int _button_leds[], const int numled, bool test_mode);
  void Update(long tick);
  void AddPoints(int points);
  void Reset();
  void Ready();
  void NotReady();
  void Print();
  void NewLight(long tick);
  void BlinkLight(long tick);
  bool CorrectButtonPressed(long tick);
  bool CheckIfReady();
  bool Debounced(long tick);
  int  Score();
private:
  String _name;
  int _input_pin;
  const int *_button_leds;
  int _numled;
  int _led_index;
  int _total_score;
  int _current_points;
  bool _is_ready;
  bool _test_mode;
  long _last_increment_tick;
  long _last_led_tick;
  bool _led_on;
  long _button_debounce_tick;
  unsigned long _button_debounce_time;
};
#endif
