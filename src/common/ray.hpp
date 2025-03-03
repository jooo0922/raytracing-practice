#ifndef RAY_HPP
#define RAY_HPP

#include "vec3.hpp"

// ray (반직선) 클래스 정의
class ray
{
public:
  ray() {}                                                                           // 매개변수 없는 기본 생성자
  ray(const point3 &origin, const vec3 &direction) : orig(origin), dir(direction) {} // 반직선의 출발점, 방향벡터를 매개변수로 받는 생성자 오버로딩 > 반직선 출발점 및 방향 멤버변수 초기화

  // 각 캡슐화된 멤버변수를 반환하는 getter 메서드를 상수함수로 정의함.
  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }

  // 반직선 상의 특정 점의 좌표를 반환하는 상수함수 (ray 객체의 데이터를 변경하지 않음.)
  point3 at(double t) const
  {
    // 반직선 상에서 임의의 비율값 t 에 대응되는 점의 좌표를 계산하여 반환
    return orig + t * dir;
  }

private:
  point3 orig; // 반직선의 출발점 멤버변수
  vec3 dir;    // 반직선의 방향 멤버변수
};

#endif /* RAY_HPP */
