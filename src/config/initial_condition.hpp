#ifndef __COND_INI_HPP
#define __COND_INI_HPP
#include <cmath>

inline double g(double x, double y, double z)
{
  x -= 0.5;
  y -= 0.5;
  z -= 0.5;
  if (x * x + y * y + z * z < 0.1)
    return 1.0;
  else
    return 0.0;
}

#endif

