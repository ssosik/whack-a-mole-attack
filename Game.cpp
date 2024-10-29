#include "Game.h"

#define HWSERIAL Serial2

const int BAUD = 115200;
const int LOOPDELAY = 10; // Milliseconds to delay on each Update

typedef struct {
  uint8_t lang;
  String name;
  uint32_t timeout;
} state;

// These values are the timeouts for various states in milliseconds, and will
// be divided by the above LOOPDELAY in order to calculate how many ticks the
// state should run for
const state States[] {
    {0, "BOOT", 200},
    {1, "SENDCONREADY", 1000},
    {2, "DISPLAYREADYWAIT", 10000},
    {3, "INITIALIZED", 200},
    {4, "NEWGAME", 200},
    {5, "STARTGAME", 1000},
    {6, "SHOWHIGHSCORE", 10000},
    {7, "SHOWSPLASH", 8000},
    {8, "SHOWPRESSSTART", 10000},
    {9, "P1READY", 10000},
    {10, "P2READY", 10000},
    {11, "STARTREADY", 2000},
    {12, "STARTSET", 2000},
    {13, "STARTGO", 1000},
    {14, "PLAYGAME", 60000}, // How long to play the game in milliseconds
    {15, "GAMEEND", 200},
    {16, "SHOWWINNER", 10000},
};

// Constructor, create a game instance and initialize values
Game::Game(Player &p1, Player &p2) {
  Serial.print("Game Boot");
  _gameTicker = 0;
  _currentStateTimeout = 0;
  _state = GameState::Boot;
  _p1 = &p1;
  _p2 = &p2;
  _prev_message = "";
  HWSERIAL.begin(BAUD);
}

bool DisplayReady() {
  // Try to read a message from the Display
  int incomingByte;
  String msg = "";
  // Read from the serial connection looking for "DSPREADY"
  while (HWSERIAL.available() > 0) {
    incomingByte = HWSERIAL.read();
    Serial.print(char(incomingByte));
    msg = msg + String(char(incomingByte));
  };
  return msg == "DSPREADY";
}

// Advance to the next state, setting the timeout
void Game::nextState(GameState state) {
  _currentStateTimeout = _gameTicker + (States[state].timeout / LOOPDELAY);
  _state = state;
  Serial.print("Next State: ");
  Serial.println(States[state].name);
}

// Has the current game ticker value has exceeded the previously-set timeout
bool Game::stateTimedOut() {
  return _gameTicker >= _currentStateTimeout;
}

// Handle the state actions for this iteration
void Game::Update() {
  int p1score;
  int p2score;
  String msg;

  switch(_state) {
    case Boot:
      nextState(GameState::SendConReady);
      break;

    case SendConReady:
      _prev_message = "";
      // Send CONREADY message over serial line for 2-way handshake with Display
      sendMessage("#CONREADY");

      // Go to the next state
      nextState(GameState::DisplayReadyWait);
      break;

    case DisplayReadyWait:
      // Wait for the LED display to confirm it's ready
      if (DisplayReady()) {
        // It's ready, we're all initialized
        nextState(GameState::Initialized);
      }

      if (stateTimedOut()) {
        // Retry, we waited for a while and did not get DSPREADY, send CONREADY again
        nextState(GameState::SendConReady);
      }
      break;

    case Initialized:
      nextState(GameState::NewGame);
      break;

    case NewGame:
      _p1->Reset();
      _p2->Reset();

      nextState(GameState::ShowSplash);
      sendMessage("#NEWGAME");
      break;

    case ShowSplash:
      sendMessage("#SHOWSPLASH");

      if (_p1->CheckIfReady()) {
        nextState(GameState::P1Ready);
      }

      if (_p2->CheckIfReady()) {
        nextState(GameState::P2Ready);
      }


      if (stateTimedOut()) {
        nextState(GameState::ShowHighScore);
      }
      break;

    case ShowHighScore:
      sendMessage("#SHOWHIGHSCORE");

      if (_p1->CheckIfReady()) {
        nextState(GameState::P1Ready);
      }

      if (_p2->CheckIfReady()) {
        nextState(GameState::P2Ready);
      }

      if (stateTimedOut()) {
        nextState(GameState::ShowPressStart);
      }
      break;

    case ShowPressStart:
      sendMessage("#SHOWPRESSSTART");

      if (_p1->CheckIfReady()) {
        nextState(GameState::P1Ready);
      }

      if (_p2->CheckIfReady()) {
        nextState(GameState::P2Ready);
      }


      if (stateTimedOut()) {
        nextState(GameState::ShowSplash);
      }
      break;

    case P1Ready:
      _p1->Ready();

      // Wait for the other player to hit the start button
      if (_p2->CheckIfReady()) {
        _p2->Ready();
        nextState(GameState::StartCountdownReady);
      }

      if (stateTimedOut()) {
        nextState(GameState::StartCountdownReady);
      }

      //msg = "#ISP2READY:";
      //msg += _currentStateTimeout - _gameTicker;
      //sendMessage(msg);
      sendMessage("#ISP2READY");

      break;

    case P2Ready:
      _p2->Ready();

      // Wait for the other player to hit the start button
      if (_p1->CheckIfReady()) {
        _p1->Ready();
        nextState(GameState::StartCountdownReady);
      }

      if (stateTimedOut()) {
        nextState(GameState::StartCountdownReady);
      }

      // msg = "#ISP1READY:";
      // msg += _currentStateTimeout - _gameTicker;
      // sendMessage(msg);
      sendMessage("#ISP1READY");

      break;

    case StartCountdownReady:
      sendMessage("#GAMEREADY:READY");
      if (stateTimedOut()) {
        nextState(GameState::StartCountdownSet);
      }
      break;

    case StartCountdownSet:
      sendMessage("#GAMEREADY:SET");
      if (stateTimedOut()) {
        nextState(GameState::StartCountdownGo);
      }
      break;

    case StartCountdownGo:
      sendMessage("#GAMEREADY:GO");
      if (stateTimedOut()) {
        nextState(GameState::StartGame);
      }
      break;

    case StartGame:
      _p1->NewLight(_gameTicker);
      _p2->NewLight(_gameTicker);
      nextState(GameState::PlayGame);
      break;

    case PlayGame:
      _p1->Update(_gameTicker);
      _p2->Update(_gameTicker);
  
      // Send the current score to the display
      msg = "#p1:";
      msg += _p1->Score();
      msg += "#p2:";
      msg += _p2->Score();
      if (_prev_message != msg) {
        _prev_message = msg;
        msg += "#loop:";
        msg += _gameTicker;
        HWSERIAL.print(msg);
        //Serial.println(msg);
      }

      if (stateTimedOut()) {
        nextState(GameState::GameEnd);
      }
      break;

    case GameEnd:
      _p1->NotReady();
      _p2->NotReady();
      if (stateTimedOut()) {
        nextState(GameState::ShowWinner);
      }
      break;

    case ShowWinner:
      p1score = _p1->Score();
      p2score = _p2->Score();
      if (p1score == p2score) {
        msg = "#TIEGAME:";
        msg += p1score;
        sendMessage(msg);
      } else if (p1score > p2score) {
        msg = "#PLAYER1WIN:";
        msg += p1score;
        sendMessage(msg);
      } else {
        msg = "#PLAYER2WIN:";
        msg += p2score;
        sendMessage(msg);
      }

      if (_p1->CheckIfReady()) {
        _p1->Reset();
        _p2->Reset();
        nextState(GameState::P1Ready);
      }

      if (_p2->CheckIfReady()) {
        _p1->Reset();
        _p2->Reset();
        nextState(GameState::P2Ready);
      }

      if (stateTimedOut()) {
        nextState(GameState::NewGame);
      }
      break;
  };

  _gameTicker += 1;
  delay(LOOPDELAY);
}

void Game::sendMessage(String s) {
  if (_prev_message != s) {
    HWSERIAL.println(s);
    Serial.println(s);
    _prev_message = s;
  }
}
