#ifndef Game_h
#define Game_h
#include "Arduino.h"
#include "Player.h"

enum GameState {
  // System has just been powered on
  Boot,

  // Controller should send CONREADY to Display
  SendConReady,

  // Controller is waiting for DSPREADY from Display
  DisplayReadyWait,

  // Controller and Display are initialized
  Initialized,

  // Reset players and get ready for a new game
  NewGame,

  StartGame,

  // Show High Score
  ShowHighScore,

  // Show Splash
  ShowSplash,

  // Show Press Start
  ShowPressStart,

  // P1 Ready
  P1Ready,

  // P2 Ready
  P2Ready,

  // Start Countdown: Ready
  StartCountdownReady,

  // Start Countdown: Set
  StartCountdownSet,

  // Start Countdown: Go
  StartCountdownGo,

  // Game Play
  PlayGame,

  // Times up, Game End
  GameEnd,

  // Show winning player score
  ShowWinner,
};

class Game {
public:
  Game(Player &p1, Player &p2);
  void Update();

private:
  GameState _state;
  GameState _prev_state;
  long _gameTicker;
  long _currentStateTimeout;
  int _countdown_num;

  Player *_p1;
  Player *_p2;

  String _prev_message; // Use for not sending the same message over and over

  void printTick();
  void nextState(GameState state);
  bool stateTimedOut();
  void sendMessage(String s);
};
#endif
