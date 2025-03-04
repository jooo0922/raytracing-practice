#ifndef COLOR_HPP
#define COLOR_HPP

#include "vec3.hpp"

// vec3 에 대한 별칭으로써 color 선언
using color = vec3;

// 출력 스트림(std::ofstream or std::ostream)에 .ppm 파일에 저장할 색상값을 출력하는 util 함수
void write_color(std::ostream &out, color pixel_color)
{
  // pixel_color 는 요소의 값이 0 ~ 1 사이로 정규화된 vec3 로 생성되어 전달되겠군.
  // pixel_color 내의 정규화된 요소의 값을 0 ~ 256 범위 내의 정수형으로 형변환하여 출력함.
  out << static_cast<int>(255.999 * pixel_color.x()) << ' '
      << static_cast<int>(255.999 * pixel_color.y()) << ' '
      << static_cast<int>(255.999 * pixel_color.z()) << '\n';
}

#endif /* COLOR_HPP */
