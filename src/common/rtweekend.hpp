#ifndef RTWEEKEND_HPP
#define RTWEEKEND_HPP

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <cstdio>
#include <string>
#include <fstream>

// constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// utils
inline double degrees_to_radians(double degrees)
{
  return degrees * pi / 180.0f;
};

// common
#include "color.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"

#endif /* RTWEEKEND_HPP */
