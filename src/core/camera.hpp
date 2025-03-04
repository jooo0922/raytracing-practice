#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../hittable/hittable.hpp"

class camera
{
public:
  double aspect_ratio = 1.0f; // .ppm 이미지 종횡비 (기본값 1:1)
  int image_width = 100;      // .ppm 이미지 너비 (기본값 100. 이미지 높이는 너비에 aspect_ratio 를 곱해서 계산.)

public:
  // 출력 스트림(std::ofstream or std::ostream)에 pixel 들을 순회하며 데이터 출력(= .ppm 이미지 렌더링)
  void render(std::ostream &output_stream, const hittable &world) {
    // ...
  };

private:
  // 카메라 및 viewport 파라미터 초기화
  void initialize() {
    // ...
  };

  // 주어진 반직선(ray)을 world 에 casting 하여 계산된 최종 색상값을 반환하는 함수
  color ray_color(const ray &r, const hittable &world) {
    // ...
  };
};

#endif /* CAMERA_HPP */
