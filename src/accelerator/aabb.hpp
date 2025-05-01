#ifndef AABB_HPP
#define AABB_HPP

#include "rtweekend.hpp"

/**
 * 축 정렬 경계 박스(Axis-Aligned Bounding Box, AABB)를 정의하는 클래스
 *
 * - 개별 오브젝트 또는 BVH(Bounding Volume Hierarchy) 가속 구조의 노드에 대해 경계 박스를 정의할 때 사용된다.
 * - 생성된 광선(ray)과 AABB 간의 교차 여부를 slab method를 기반으로 판단한다.
 */
class aabb
{
public:
  // x, y, z 축 방향의 슬랩(interval)들을 구성하는 멤버 변수 (slab 개념 설명 하단 필기 참고)
  interval x, y, z;

  // 빈 AABB를 생성 (기본값은 비어 있는 슬랩으로 간주)
  aabb() {};

  // 각 축별 슬랩(interval) 값을 직접 지정하여 AABB를 생성
  aabb(const interval &x, const interval &y, const interval &z)
      : x(x), y(y), z(z) {};

  // 두 점을 받아서, 각 축에 대해 최소/최대 범위를 자동으로 계산하여 AABB를 생성
  aabb(const point3 &a, const point3 &b)
  {
    // a와 b 중 누가 min/max인지는 상관없이, 자동으로 각 축별 슬랩(interval)을 생성
    x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
    x = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
    x = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
  };

  // 지정한 축 인덱스(0:x, 1:y, 2:z)에 해당하는 슬랩(interval)을 반환
  const interval &axis_interval(int n) const
  {
    if (n == 1)
      return y;
    if (n == 2)
      return z;
    return x;
  };
};

#endif /* AABB_HPP */
