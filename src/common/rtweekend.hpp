#ifndef RTWEEKEND_HPP
#define RTWEEKEND_HPP

#include <cmath>
#include <cstdlib>
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

inline double random_double()
{
  // 0.0f <= n < 1.0f 사이의 난수 생성
  return std::rand() / (RAND_MAX + 1.0f);
};

inline double random_double(double min, double max)
{
  // min <= n < max 사이의 난수 생성
  return min + (max - min) * random_double();
};

inline int random_int(int min, int max)
{
  // min <= n < max 사이의 정수인 난수 생성
  return int(random_double(min, max + 1));
};

// common
#include "color.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"

#endif /* RTWEEKEND_HPP */
