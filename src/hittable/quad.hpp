#ifndef QUAD_HPP
#define QUAD_HPP

#include "hittable.hpp"

// quadrilateral(정확히는 평행사변형) 클래스를 hittable(피충돌 물체) 추상 클래스로부터 상속받아 정의
class quad : public hittable
{
public:
  // quad를 생성할 때 기준점 Q와 두 변 벡터 u, v, 그리고 재질 mat를 받음
  quad(const point3 &Q, const vec3 &u, const vec3 &v, std::shared_ptr<material> mat)
      : Q(Q), u(u), v(v), mat(mat)
  {
    // quad 생성과 동시에 aabb 설정
    set_bounding_box();
  };

  // quad의 네 꼭짓점을 포함하는 AABB 계산
  virtual void set_bounding_box()
  {
    // 두 대각선 방향으로 AABB 두 개 생성
    auto bbox_diagonal1 = aabb(Q, Q + u + v); // 점 Q에서 점 Q+u+v 로 향하는 대각선을 감싸는 AABB 생성
    auto bbox_diagonal2 = aabb(Q + u, Q + v); // 점 Q+u에서 점 Q+v 로 향하는 대각선을 감싸는 AABB 생성

    // 두 AABB를 합쳐 quad 를 전부 감싸는 AABB 생성
    bbox = aabb(bbox_diagonal1, bbox_diagonal2);
  };

  // quad의 AABB 반환 함수
  aabb bounding_box() const override { return bbox; };

  // 순수 가상 함수 hit 재정의(override)
  bool hit(const ray &r, interval ray_t, hit_record &rec) const override
  {
    // TODO : 구현 예정
    return false;
  };

private:
  // quad 를 정의하는 데이터를 private 멤버변수로 정의
  // https://raytracing.github.io/books/RayTracingTheNextWeek.html#quadrilaterals/definingthequadrilateral 참고
  point3 Q;                      // quad의 기준 꼭지점
  vec3 u, v;                     // quad의 두 변 벡터
  std::shared_ptr<material> mat; // quad에 충돌한 ray 의 산란 계산 시 적용할 material 포인터 멤버변수(reference counting 기반 smart pointer 로 객체 수명 관리)
  aabb bbox;                     // quad를 감싸는 AABB
};

#endif /* QUAD_HPP */
