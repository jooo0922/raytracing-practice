#ifndef QUAD_HPP
#define QUAD_HPP

#include "hittable.hpp"

// quadrilateral(정확히는 평행사변형) 클래스를 hittable(피충돌 물체) 추상 클래스로부터 상속받아 정의
class quad : public hittable
{
public:
  // quad를 생성할 때 기준점 Q와 두 변 벡터 u, v, 그리고 재질 mat를 받음

private:
  // quad 를 정의하는 데이터를 private 멤버변수로 정의
  // https://raytracing.github.io/books/RayTracingTheNextWeek.html#quadrilaterals/definingthequadrilateral 참고
  point3 Q;                      // quad의 기준 꼭지점
  vec3 u, v;                     // quad의 두 변 벡터
  std::shared_ptr<material> mat; // quad에 충돌한 ray 의 산란 계산 시 적용할 material 포인터 멤버변수(reference counting 기반 smart pointer 로 객체 수명 관리)
  aabb bbox;                     // quad를 감싸는 AABB
};

#endif /* QUAD_HPP */
