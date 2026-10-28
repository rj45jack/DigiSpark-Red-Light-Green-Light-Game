// SPDX-License-Identifier: Apache-2.0
#include <unity.h>

#include "game.h"

namespace {

using traffic::Game;
using traffic::Phase;

constexpr uint16_t kSteps = 2000;

uint8_t raw(Phase phase) { return static_cast<uint8_t>(phase); }

}  // namespace

void test_starts_with_a_dark_gap(void) {
  Game game(1234);
  game.begin(1000);

  TEST_ASSERT_EQUAL_UINT8(raw(Phase::kDark), raw(game.phase()));
  TEST_ASSERT_EQUAL_UINT32(traffic::kDarkMs, game.durationMs());
  TEST_ASSERT_FALSE(game.due(1000));
  TEST_ASSERT_FALSE(game.due(1499));
  TEST_ASSERT_TRUE(game.due(1500));
  TEST_ASSERT_TRUE(game.due(5000));
}

void test_due_survives_millis_wrap(void) {
  Game game(1);
  game.begin(0xFFFFFF00u);

  TEST_ASSERT_FALSE(game.due(0xFFFFFF10u));
  TEST_ASSERT_FALSE(game.due(0x00000000u));
  TEST_ASSERT_FALSE(game.due(0x000000F3u));
  TEST_ASSERT_TRUE(game.due(0x000000F4u));
}

void test_advance_reschedules_from_now(void) {
  Game game(5);
  game.begin(0);

  TEST_ASSERT_TRUE(game.due(traffic::kDarkMs));
  game.advance(1000);

  TEST_ASSERT_FALSE(game.due(1000));
  TEST_ASSERT_TRUE(game.due(1000 + game.durationMs()));
}

void test_durations_stay_in_range(void) {
  Game game(42);
  game.begin(0);

  uint32_t now = 0;
  for (uint16_t i = 0; i < kSteps; ++i) {
    now = game.endsAtMs();
    const Phase next = game.advance(now);

    if (next == Phase::kDark) {
      TEST_ASSERT_EQUAL_UINT32(traffic::kDarkMs, game.durationMs());
    } else {
      TEST_ASSERT_TRUE(game.durationMs() >= traffic::kColorMinMs);
      TEST_ASSERT_TRUE(game.durationMs() < traffic::kColorMaxMs);
    }
    TEST_ASSERT_EQUAL_UINT32(now + game.durationMs(), game.endsAtMs());
  }
}

void test_a_colour_always_ends_in_the_dark(void) {
  Game game(7);
  game.begin(0);

  Phase previous = game.phase();
  for (uint16_t i = 0; i < kSteps; ++i) {
    const Phase next = game.advance(i);
    if (previous != Phase::kDark) {
      TEST_ASSERT_EQUAL_UINT8(raw(Phase::kDark), raw(next));
    }
    previous = next;
  }
}

void test_schedule_reaches_both_colours_and_extra_gaps(void) {
  Game game(2026);
  game.begin(0);

  bool sawGreen = false;
  bool sawRed = false;
  bool sawExtraGap = false;
  Phase previous = game.phase();

  for (uint16_t i = 0; i < kSteps; ++i) {
    const Phase next = game.advance(i);
    if (next == Phase::kGreen) {
      sawGreen = true;
    }
    if (next == Phase::kRed) {
      sawRed = true;
    }
    if (previous == Phase::kDark && next == Phase::kDark) {
      sawExtraGap = true;
    }
    previous = next;
  }

  TEST_ASSERT_TRUE(sawGreen);
  TEST_ASSERT_TRUE(sawRed);
  TEST_ASSERT_TRUE(sawExtraGap);
}

void test_same_seed_gives_the_same_schedule(void) {
  Game a(123456);
  Game b(123456);
  a.begin(0);
  b.begin(0);

  for (uint16_t i = 0; i < 200; ++i) {
    const Phase phaseA = a.advance(i);
    const Phase phaseB = b.advance(i);
    TEST_ASSERT_EQUAL_UINT8(raw(phaseA), raw(phaseB));
    TEST_ASSERT_EQUAL_UINT32(a.durationMs(), b.durationMs());
  }
}

void test_different_seeds_diverge(void) {
  Game a(1);
  Game b(2);
  a.begin(0);
  b.begin(0);

  bool diverged = false;
  for (uint16_t i = 0; i < 50 && !diverged; ++i) {
    const Phase phaseA = a.advance(i);
    const Phase phaseB = b.advance(i);
    if (phaseA != phaseB || a.durationMs() != b.durationMs()) {
      diverged = true;
    }
  }

  TEST_ASSERT_TRUE(diverged);
}

void test_zero_seed_still_runs(void) {
  Game game(0);
  game.begin(0);

  bool sawColour = false;
  for (uint16_t i = 0; i < 50; ++i) {
    if (game.advance(i) != Phase::kDark) {
      sawColour = true;
    }
  }

  TEST_ASSERT_TRUE(sawColour);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_starts_with_a_dark_gap);
  RUN_TEST(test_due_survives_millis_wrap);
  RUN_TEST(test_advance_reschedules_from_now);
  RUN_TEST(test_durations_stay_in_range);
  RUN_TEST(test_a_colour_always_ends_in_the_dark);
  RUN_TEST(test_schedule_reaches_both_colours_and_extra_gaps);
  RUN_TEST(test_same_seed_gives_the_same_schedule);
  RUN_TEST(test_different_seeds_diverge);
  RUN_TEST(test_zero_seed_still_runs);
  return UNITY_END();
}
