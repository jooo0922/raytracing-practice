#ifndef COLOR_HPP
#define COLOR_HPP

#include "interval.hpp"
#include "vec3.hpp"

// vec3 에 대한 별칭으로써 color 선언
using color = vec3;

// 출력 스트림(std::ofstream or std::ostream)에 .ppm 파일에 저장할 색상값을 출력하는 util 함수
void write_color(std::ostream &out, const color &pixel_color)
{
  /**
   * pixel_color 에는 antialiasing 을 위해
   * 현재 pixel 주변 sample 들을 통과하는 ray 들로부터 얻어진 색상들을 적분한 색상값이 전달됨.
   *
   * 그러나, 적분 과정에서 [0.0, 1.0] 범위를 살짝 벗어나는 색상값이 계산될 가능성이 있으므로,
   * 적분된 색상값의 각 컴포넌트들을 해당 범위로 안전하게 clamping 한 뒤에 출력 스트림에 기록함.
   */
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // 적분된 색상값을 256개의 정수형 범위([0, 255])로 맵핑할 수 있도록 [0.0, 0.999] 사이로 clamping
  static const interval intensity(0.000f, 0.999f);

  /**
   * int casting 시 소수점 내림(truncation) 연산을 수행하므로,
   * 최대 정수값을 255 로 캐스팅되도록 하려면 [0.0, 255.~~~] 범위로 맵핑해야 함.
   * 따라서, [0.0, 0.999] 범위로 clamping 된 색상값에 256 을 곱한 뒤 casting 함.
   */
  int rbyte = int(256 * intensity.clamp(r));
  int gbyte = int(256 * intensity.clamp(g));
  int bbyte = int(256 * intensity.clamp(b));

  // 출력 스트림에 [0, 255] 정수형 범위로 맵핑된 색상값 쓰기
  out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif /* COLOR_HPP */
