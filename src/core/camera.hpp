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
  color ray_color(const ray &r, const hittable &world)
  {
    // world 에 추가된 hittable objects 들을 순회하며 현재 ray 와 교차 검사 수행
    hit_record rec;
    if (world.hit(r, interval(0, infinity), rec))
    {
      // 하나라도 충돌한 hittable object 가 존재한다면, rec 변수에는 현재 ray 방향에서 카메라로부터 가장 가까운 교차점의 충돌 정보가 기록됨.
      // -> 카메라에서 가장 가까운 교차점의 노멀벡터([-1.0, 1.0] 범위)를 [0.0, 1.0] 범위의 색상값으로 맵핑
      return 0.5f * (rec.normal + color(1.0f, 1.0f, 1.0f));
    }

    // 반직선을 길이가 1인 단위 벡터로 정규화
    vec3 unit_direction = unit_vector(r.direction());
    // [-1.0, 1.0] 범위로 정규화된 단위 벡터의 y 값을 [0.0, 1.0] 범위로 맵핑
    auto a = 0.5f * (unit_direction.y() + 1.0f);

    // 흰색과 파란색을 [0.0, 1.0] 범위의 a값에 따라 혼합(선형보간)하여 .ppm 에 출력할 색상 계산
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
  };
};

#endif /* CAMERA_HPP */
