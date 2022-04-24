#pragma once
#include "constants.h"

inline auto radians(double rDegrees) -> double {
    return rDegrees * gPI / 180.0;
}

inline auto degrees(double rRadians) -> double {
    return rRadians * 180.0 / gPI;
}
