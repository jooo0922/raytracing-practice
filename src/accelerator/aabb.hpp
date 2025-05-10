#ifndef AABB_HPP
#define AABB_HPP

#include "common/rtweekend.hpp"

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
    y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
    z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
  };

  // 두 AABB 를 감싸는(= 합친) 가장 작은 AABB를 생성
  aabb(const aabb &box0, const aabb &box1)
  {
    // 두 AABB 의 각 축별 슬랩을 합친다.
    x = interval(box0.x, box1.x);
    y = interval(box0.y, box1.y);
    z = interval(box0.z, box1.z);
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

  // 주어진 광선(ray)이 AABB를 통과하는지 여부를 검사 (slab method 기반 -> 하단 필기 참고)
  bool hit(const ray &r, interval ray_t) const
  {
    // 주어진 광선의 출발점과 방향벡터
    const point3 &ray_origin = r.origin();
    const vec3 &ray_dir = r.direction();

    // AABB 를 구성하는 각 축의 slab 을 순회하며 주어진 광선과 교차 여부 검사
    for (int axis = 0; axis < 3; axis++)
    {
      // 현재 축의 슬랩(interval) 가져오기
      const interval &ax = axis_interval(axis);
      /**
       * 슬랩 경계면과 주어진 광선 상의 교차 지점 t0, t1 계산 공식 상의 denominator(분모)인
       * dx, dy, dz 를 미리 역수로 계산해두는 것.
       *
       * https://raytracing.github.io/books/RayTracingTheNextWeek.html#boundingvolumehierarchies/rayintersectionwithanaabb 참고
       */
      const double adinv = 1.0f / ray_dir[axis];

      // 슬랩 경계면과 광선의 교차 시점 t0, t1 계산
      auto t0 = (ax.min - ray_origin[axis]) * adinv;
      auto t1 = (ax.max - ray_origin[axis]) * adinv;

      // t0, t1의 순서가 반대일 수 있으므로 정렬
      if (t0 < t1)
      {
        // 광선이 각 축의 슬랩에 진입할 때, 가장 늦은 진입 시점을 ray_t.min에 반영
        if (t0 > ray_t.min)
          ray_t.min = t0;
        // 광선이 각 축의 슬랩에서 탈출할 때, 가장 빠른 탈출 시점을 ray_t.max에 반영
        if (t1 < ray_t.max)
          ray_t.max = t1;
      }
      else
      {
        if (t1 > ray_t.min)
          ray_t.min = t1;
        if (t0 < ray_t.max)
          ray_t.max = t0;
      }

      /**
       * 광선의 slab 진입 시점보다 탈출 시점이 빠르다는 건 논리적으로 맞지 않음
       * -> 즉, ray 가 모든 slab 과 교차 불가하다는 뜻이므로, false 를 반환
       */
      if (ray_t.max <= ray_t.min)
      {
        return false;
      }
    }
    return true;
  };

  // 가장 긴 축의 슬랩 인덱스를 반환하는 함수 (0: x, 1: y, 2: z)
  // -> BVH 분할 시, 가장 긴 축을 기준으로 정렬하여 공간 분할 품질을 높이기 위함
  int longest_axis() const
  {
    // x, y, z 축 슬랩 길이 중 가장 큰 축의 인덱스를 반환
    if (x.size() > y.size())
    {
      return x.size() > z.size() ? 0 : 2;
    }
    else
    {
      return y.size() > z.size() ? 1 : 2;
    }
  };

  // AABB 의 특수 상수 객체 선언
  static const aabb empty;    // 아무것도 감싸지 않는 최소 AABB
  static const aabb universe; // 모든 공간을 감싸는 무한한 AABB
};

// 비어 있는 AABB 정의 -> 누적 bounding box 계산 시 초기값으로 사용
const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);

// 무한한 AABB 정의 -> 어떤 AABB와 비교해도 포함되도록 만들기 위해 사용
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

/**
 * aabb::hit()
 *
 *
 * Slab method를 기반으로 광선(ray)과 AABB의 교차 여부를 검사한다.
 *
 * AABB는 x, y, z 세 축 각각에 대해 [min, max] 구간을 가지며,
 * 이는 두 평면으로 이루어진 "슬랩(slab)"이라 부른다.
 * 따라서 AABB는 총 세 개의 슬랩(x, y, z)의 **교집합**으로 구성된다.
 *
 * Slab method란, 광선이 각 슬랩에 들어가고 나가는 시점(t값)을 계산한 뒤,
 * 세 슬랩에서 공통으로 광선이 내부에 머무르는 구간이 존재하는지를 검사하는 방식이다.
 *
 * 각 축별로:
 *   - 슬랩 진입 시점: t0 = (slab.min - ray.origin) / ray.direction
 *   - 슬랩 탈출 시점: t1 = (slab.max - ray.origin) / ray.direction
 *   - 단, 방향이 음수일 수 있으므로 항상 min(t0, t1), max(t0, t1)로 정리함.
 *
 * 그 후, 전체 AABB에 대해 진입 가능한 가장 늦은 시점 t_min과,
 * 탈출 가능한 가장 이른 시점 t_max를 누적하여 계산한다.
 *
 * 이 두 값이 겹치는지 여부(t_min < t_max)를 통해 AABB 교차 여부를 판단한다.
 *
 * 이 개념은 친구들 약속 시간 잡기 비유로도 이해할 수 있다:
 *   - 각 축의 슬랩은 친구 1명,
 *   - 슬랩이 허용하는 t 구간은 그 친구가 약속 가능한 시간대,
 *   - 슬랩 진입 시점은 각 친구마다 모임 장소에 도착하는 시간,
 *   - 슬랩 탈출 시점은 각 친구마다 모임 장소에서 떠나는 시간,
 *   - 가장 늦은 진입 시점은 가장 늦게 모임 장소에 도착한 친구의 시간,
 *   - 가장 빠른 탈출 시점은 가장 빨리 모임 장소에서 떠나는 친구의 시간,
 *   - 세 슬랩(친구) 모두가 공통으로 만날 수 있는 시간이 존재할 때만(hit),
 *     광선이 AABB를 통과했다고 본다.
 */

#endif/* AABB_HPP */
