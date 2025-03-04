#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "hittable.hpp"

// sphere(구체) 클래스를 hittable(피충돌 물체) 추상 클래스로부터 상속받아 정의
class sphere : public hittable
{
public:
  // 구체의 중점 좌표와 반지름을 매개변수로 받는 생성자 정의
  sphere(point3 _center, double _radius) : center(_center), radius(_radius) {}

  // 순수 가상 함수 hit 재정의(override)
  bool hit(const ray &r, interval ray_t, hit_record &rec) const override
  {
    // 반직선-구체 교차 여부를 검증하는 판별식 구현 (main.cpp > '근의 공식과 판별식 관련 필기 참고')
    vec3 oc = r.origin() - center;                  // 반직선 출발점 ~ 구체의 중점까지의 벡터 (본문 공식에서 (A-C) 에 해당)
    auto a = r.direction().length_squared();        // 반직선 방향벡터 자신과의 내적 (본문 공식에서 b⋅b 에 해당) > 벡터 자신과의 내적을 벡터 길이 제곱으로 리팩터링
    auto half_b = dot(oc, r.direction());           // 2 * 반직선 방향벡터와 (A-C) 벡터와의 내적 (본문 공식에서 2tb⋅(A−C) 에 해당) > half_b 로 변경
    auto c = oc.length_squared() - radius * radius; // (A-C) 벡터 자신과의 내적 - 반직선 제곱 (본문 공식에서 (A−C)⋅(A−C)−r^2 에 해당) > 벡터 자신과의 내적을 벡터 길이 제곱으로 리팩터링

    auto discriminant = half_b * half_b - a * c; // 근의 공식 판별식 계산

    // 판별식이 0보다 작아 이차방정식의 해 t 가 존재하지 않을 경우, 교차점이 없는 것으로 판단하고 함수 종료
    if (discriminant < 0)
      return false;

    // 계산상의 편의를 위해 근의 공식에서 제곱근에 해당하는 항을 미리 구해놓음.
    auto sqrtd = sqrt(discriminant);

    // 반직선과 구체의 교차점들 중에서 더 가까운 교차점에 해당하는 반직선 상의 비율값 t 를 먼저 계산
    auto root = (-half_b - sqrtd) / a;

    if (!ray_t.surrounds(root))
    {
      // 만약, 더 가까운 교차점에 해당하는 반직선 상의 비율값 t 가 반직선의 유효범위를 벗어난다면, 나머지 교차점에 대한 반직선 상의 비율값 t 를 다시 계산함.
      root = (-half_b + sqrtd) / a;

      if (!ray_t.surrounds(root))
      {
        // 만약, 나머지 교차점에 대한 반직선 상의 비율값 t 조차 반직선 유효범위를 벗어난다면, 교차점이 없는 것으로 판단해서 false 반환하고 함수 종료
        return false;
      }
    }

    // 반직선 유효범위 내의 비율값 t 가 존재한다면, 이 비율값으로 충돌 정보를 출력 매개변수 rec 에 저장함.
    // (https://raytracing.github.io/books/RayTracingInOneWeekend.html#surfacenormalsandmultipleobjects/shadingwithsurfacenormals > 구체 표면 상의 노멀벡터 계산 관련 Figure 6 참고)
    rec.t = root;                                    // 반직선 상에서 충돌 지점이 위치하는 비율값 t 계산
    rec.p = r.at(rec.t);                             // 충돌 지점의 좌표값 계산
    vec3 outward_normal = (rec.p - center) / radius; // 구체 표면 상에서 충돌 지점의 정규화된 normal 계산
    rec.set_face_normal(r, outward_normal);          // ray 위치와 그에 따른 충돌 지점의 normal 재계산

    // 반직선 유효범위 내의 비율값 t가 존재한다면, 구체와 반직선의 충돌 지점이 존재하는 것으로 판단하여 true 반환
    return true;
  }

private:
  // 구체를 정의하는 데이터를 private 멤버변수로 정의
  point3 center; // 구체의 중심점 좌표 멤버변수
  double radius; // 구체의 반지름 멤버변수
};

#endif /* SPHERE_HPP */
