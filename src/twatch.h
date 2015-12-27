#include <pebble.h>
#pragma once

#define SECOND_LEN PBL_IF_ROUND_ELSE(77, 72)
#define SECOND_RADIUS 7
static const GPathInfo SECOND_HAND_POINTS = {
  4,
  (GPoint []) {
    {2, -70},
    {2, PBL_IF_ROUND_ELSE(-90, -85)},
    {-2, PBL_IF_ROUND_ELSE(-90, -85)},
    {-2, -70}
  }
};

#define MINUTE_LEN PBL_IF_ROUND_ELSE(-81, -78)
static const GPathInfo MINUTE_HAND_POINTS = {
  7,
  (GPoint []) {
    {4, 0},
    {4, MINUTE_LEN},
    {3, MINUTE_LEN - 2},
    {0, MINUTE_LEN - 3},
    {-3, MINUTE_LEN - 2},
    {-4, MINUTE_LEN},
    {-4, 0}
  }
};

#define HOUR_LEN PBL_IF_ROUND_ELSE(-55, -53)
static const GPathInfo HOUR_HAND_POINTS = {
  7,
  (GPoint []) {
    {5, 0},
    {5, HOUR_LEN},
    {4, HOUR_LEN -2},
    {0, HOUR_LEN - 3},
    {-4, HOUR_LEN - 2},
    {-5, HOUR_LEN},
    {-5, 0}
  }
};
