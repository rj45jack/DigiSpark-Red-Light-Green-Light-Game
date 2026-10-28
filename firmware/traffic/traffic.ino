// SPDX-License-Identifier: Apache-2.0
//
// Red light, green light.
//
// Original sketch: rj45jack, 2017.
// Modern rewrite: 2026.
//
// Target: DigiSpark (ATtiny85 at 16.5 MHz) with one WS2812B / NeoPixel on P0.
// Power it from a USB battery bank, hold it up, and play the game.

#include <Adafruit_NeoPixel.h>

#include "game.h"

// Data pin for the LED. P0 is the DigiSpark's leftmost pin.
constexpr uint8_t LED_PIN = 0;

// A single WS2812B at full brightness is painfully bright and draws ~60 mA.
// 64 keeps it clearly visible without blinding anyone.
constexpr uint8_t BRIGHTNESS = 64;

Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
traffic::Game game;

// The ATtiny85 has no entropy source, so mix the free-running microsecond
// counter with noise from an unconnected ADC pin. P2 is otherwise unused.
uint32_t makeSeed() {
  uint32_t seed = micros();
  for (uint8_t i = 0; i < 8; ++i) {
    seed = seed * 33u + static_cast<uint16_t>(analogRead(A1));
  }
  return seed;
}

void showPhase(traffic::Phase phase) {
  switch (phase) {
    case traffic::Phase::kGreen:
      pixel.setPixelColor(0, 0, 255, 0);
      break;
    case traffic::Phase::kRed:
      pixel.setPixelColor(0, 255, 0, 0);
      break;
    case traffic::Phase::kDark:
    default:
      pixel.setPixelColor(0, 0, 0, 0);
      break;
  }
  pixel.show();
}

void setup() {
  pixel.begin();
  pixel.setBrightness(BRIGHTNESS);
  pixel.setPixelColor(0, 0, 0, 0);
  pixel.show();

  game = traffic::Game(makeSeed());
  game.begin(millis());
}

void loop() {
  const uint32_t now = millis();
  if (game.due(now)) {
    showPhase(game.advance(now));
  }
}
