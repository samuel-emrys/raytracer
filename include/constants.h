#pragma once

#include <cmath>
#include <limits>
#include <memory>

const double gINFINITY = std::numeric_limits<double>::infinity();
const double gPI = 3.1415926535897932385;

inline auto radians(double rDegrees) -> double {
    return rDegrees * gPI / 180.0;
}

inline auto degrees(double rRadians) -> double {
    return rRadians * 180.0 / gPI;
}
