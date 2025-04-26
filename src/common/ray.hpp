#ifndef RAY_HPP
#define RAY_HPP

#include "vec3.hpp"

// ray (반직선) 클래스 정의
class ray
{
public:
  ray() {};                                                                                                  // 매개변수 없는 기본 생성자
  ray(const point3 &origin, const vec3 &direction, double time) : orig(origin), dir(direction), tm(time) {}; // 반직선의 출발점, 방향벡터, 생성 시점을 매개변수로 받는 생성자 오버로딩
  ray(const point3 &origin, const vec3 &direction) : ray(origin, direction, 0.0f) {};                        // 반직선의 출발점, 방향벡터를 매개변수로 받는 생성자 오버로딩

  // 각 캡슐화된 멤버변수를 반환하는 getter 메서드를 상수함수로 정의함.
  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }

  // 광선 생성 시점 반환하는 getter
  double time() const { return tm; }

  // 반직선 상의 특정 점의 좌표를 반환하는 상수함수 (ray 객체의 데이터를 변경하지 않음.)
  point3 at(double t) const
  {
    // 반직선 상에서 임의의 비율값 t 에 대응되는 점의 좌표를 계산하여 반환
    return orig + t * dir;
  }

private:
  point3 orig; // 반직선의 출발점 멤버변수
  vec3 dir;    // 반직선의 방향 멤버변수
  double tm;   // 광선이 생성된 시점
};

/**
 * Motion Blur 와 ray::time
 *
 *
 * 기존의 광선(ray)은 시작점(origin)과 방향(direction)만 저장했지만,
 * Motion Blur 를 구현하려면 광선이 "언제" 생성되었는지의 정보(time)가 추가로 필요하다.
 *
 * 시간(time) 정보를 추가하면,
 * - 광선이 발사된 시점에 따라 움직이는 물체들의 위치를 정확히 계산할 수 있고,
 * - 결과적으로 시간에 따라 흐릿해지는(motion blur) 효과를 자연스럽게 표현할 수 있다.
 *
 * Motion Blur 를 defocus blur와 마찬가지로 여러 샘플들을 적분하는 방식이며,
 * 차이점은 '공간'이 아닌 '시간' 축(time axis) 상에서 여러 시점을 샘플링하여 적분한다는 점이다.
 *
 * 레이 트레이서 내부 로직은 크게 변하지 않고,
 * 각 ray가 가지고 있는 time 값을 기반으로 교차(intersection) 판정만 시간에 맞게 처리해주면 된다.
 */

#endif /* RAY_HPP */
