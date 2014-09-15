#pragma once

#include <cmath>

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

// угол между двумя точками
inline double GetAngle(int xFrom, int yFrom, int xTo, int yTo) {
    int dx = xTo - xFrom;
    int dy = yTo - yFrom;
    return atan2(dy, dx);
}

// минимальный угол между двумя углами
inline double GetAngle(double angelFrom, double angelTo) {
    return atan2(sin(angelTo - angelFrom), cos(angelTo - angelFrom));
}

template<typename T1, typename T2>
inline double Distance(T1 x1, T1 y1, T2 x2, T2 y2) {
    return sqrt(((double)x2 - (double)x1) * ((double)x2 - (double)x1) +
                ((double)y2 - (double)y1) * ((double)y2 - (double)y1));
}
