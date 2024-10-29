#include "Player.h"
#include "Game.h"

const int NUMLED = 5;

// An unconnected "floating" pin to give us some randomness for seeding
const int FLOATINGPIN = 17;
const int P1BUTTONPIN = 27;
const int P2BUTTONPIN = 38;
//                           B   R   W   G   Y
const int P1LEDS[NUMLED] = { 28, 29, 30, 31, 32 };
const int P2LEDS[NUMLED] = { 37, 36, 35, 34, 33 };

// Create Players and Game
Player p1(String("P1"), P1BUTTONPIN, P1LEDS, NUMLED, false);
Player p2(String("P2"), P2BUTTONPIN, P2LEDS, NUMLED, false);
Game state(p1, p2);

void setup() {
  randomSeed(analogRead(FLOATINGPIN));
}

void loop() {
  state.Update();
}
