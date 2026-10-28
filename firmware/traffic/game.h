// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <stdint.h>

// Red light, green light timing logic.
//
// The game is deliberately dumb: it alternates a coloured phase with a short
// dark gap. A dark phase either starts a green or red phase, or (one time in
// three) repeats the gap so the next colour arrives a little later.
//
// Nothing here touches the Arduino API, so the whole schedule is exercised by
// host-side tests.

namespace traffic {

enum class Phase : uint8_t { kDark, kGreen, kRed };

// The pause between coloured phases, in milliseconds.
constexpr uint16_t kDarkMs = 500;

// A green or red phase lasts a random time in [kColorMinMs, kColorMaxMs).
constexpr uint32_t kColorMinMs = 3000;
constexpr uint32_t kColorMaxMs = 10000;

class Game {
 public:
  // A zero seed would make the xorshift generator stick at zero.
  explicit Game(uint32_t seed = 1) : rng_(seed == 0 ? 1 : seed) {}

  void begin(uint32_t nowMs) {
    phase_ = Phase::kDark;
    durationMs_ = kDarkMs;
    endsAtMs_ = nowMs + durationMs_;
  }

  // True once the current phase has run for its full duration. The signed
  // subtraction keeps this correct when millis() wraps.
  bool due(uint32_t nowMs) const {
    return static_cast<int32_t>(nowMs - endsAtMs_) >= 0;
  }

  // Moves to the next phase, starting from nowMs, and returns it.
  Phase advance(uint32_t nowMs) {
    if (phase_ == Phase::kDark) {
      switch (nextRandom(3)) {
        case 1:
          phase_ = Phase::kGreen;
          durationMs_ = kColorMinMs + nextRandom(kColorMaxMs - kColorMinMs);
          break;
        case 2:
          phase_ = Phase::kRed;
          durationMs_ = kColorMinMs + nextRandom(kColorMaxMs - kColorMinMs);
          break;
        default:
          // Roll again after another gap: same behaviour as the original
          // sketch's random(3) == 0 branch.
          phase_ = Phase::kDark;
          durationMs_ = kDarkMs;
          break;
      }
    } else {
      phase_ = Phase::kDark;
      durationMs_ = kDarkMs;
    }

    endsAtMs_ = nowMs + durationMs_;
    return phase_;
  }

  Phase phase() const { return phase_; }
  uint32_t durationMs() const { return durationMs_; }
  uint32_t endsAtMs() const { return endsAtMs_; }

 private:
  // xorshift32: cheap on an ATtiny85 and good enough for a game.
  uint32_t nextRandom(uint32_t bound) {
    rng_ ^= rng_ << 13;
    rng_ ^= rng_ >> 17;
    rng_ ^= rng_ << 5;
    return rng_ % bound;
  }

  uint32_t rng_;
  Phase phase_ = Phase::kDark;
  uint32_t durationMs_ = kDarkMs;
  uint32_t endsAtMs_ = 0;
};

}  // namespace traffic
