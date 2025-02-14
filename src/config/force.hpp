#ifndef __FORCE_HPP
#define __FORCE_HPP
#include <cmath>

inline double f(double x, double y, double z, double t)
{
  if (x < 0.3)
    return sin(x - 0.5) * cos(y - 0.5) * exp(-z * z);
  else
    return 0.0;
}

#endif
