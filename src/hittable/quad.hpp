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
    /** quad 가 속한 평면을 수학적으로 정의하는 두 파라미터 계산 (하단 필기 참고) */
    // 평면의 법선 벡터 계산 (평면의 방향)
    auto n = cross(u, v);
    normal = unit_vector(n);
    // 평면 방정식의 상수 D = n ⋅ Q (Q는 평면 위의 고정된 점) 계산
    D = dot(normal, Q);

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
  vec3 normal;                   // quad 가 속한 평면의 법선 벡터(= 평면의 방향)
  double D;                      // quad 가 속한 평면 방정식의 상수 D = n ⋅ Q
};

/**
 * 주어진 quad 가 속한 평면 정의 및 평면 - Ray 교차 검사
 *
 *
 * 이 quad는 평면 위에 정의된 사각형이다. 이를 위해 먼저 quad가 놓인 평면의 수학적 정의가 필요하다.
 *
 * 일반적인 평면의 방정식은 다음과 같다:
 *    n ⋅ (P - C) = 0
 *    - n: 평면의 법선 벡터 (normal)
 *    - C: 평면 위의 고정된 기준점 (여기서는 quad의 꼭짓점 Q)
 *    - P: 평면 위의 임의의 점
 *
 * 위 식을 전개하면:
 *    n ⋅ P - n ⋅ C = 0
 *    → n ⋅ P = n ⋅ C
 *    → n ⋅ P = D     (단, D = n ⋅ C = n ⋅ Q)
 *
 * 여기서 D는 **평면의 방정식을 고정시키는 상수 항**으로, 해당 평면 위에 임의의 점 P가 놓이려면
 * 반드시 만족해야 하는 조건 n ⋅ P = D의 **우변 값을 의미**한다.
 *
 * 즉, 평면은 n과 D 두 값만으로 다음과 같이 완전히 정의된다:
 *    { (x, y, z) ∈ ℝ³ | n ⋅ P = D }
 *
 * 이때 D는 단순한 숫자가 아니라, **법선 벡터와 기준점 간의 내적값으로부터 계산된 상수**이며,
 * 그 평면이 어디에 위치하는지를 결정짓는 중요한 수치다.
 *
 * 이제 ray와의 교차점을 계산한다.
 * ray는 다음과 같은 수식으로 표현된다:
 *    R(t) = P₀ + t·d
 *    - P₀: ray의 시작점
 *    - d: ray의 방향 벡터
 *
 * ray가 평면과 교차하려면, 교차점 R(t)가 평면 위에 있어야 하므로 다음이 성립해야 한다:
 *    n ⋅ R(t) = D
 *    → n ⋅ (P₀ + t·d) = D
 *    → n ⋅ P₀ + t(n ⋅ d) = D
 *    → t = (D - n ⋅ P₀) / (n ⋅ d)
 *
 * 이 t 값이 유효한 범위(ray_t)에 있고, 분모 n ⋅ d ≠ 0일 경우
 * ray는 quad가 속한 평면과 교차한다고 볼 수 있다.
 *
 * 이 과정에서 D는 해당 평면이 어디에 놓여 있는지(위치)를 나타내는 **기준점 역할**을 하며,
 * ray origin이 평면으로부터 얼마나 떨어져 있는지를 계산할 수 있게 해준다.
 *
 * 따라서 코드 상에서 D = dot(normal, Q)로 계산하여,
 * 평면 방정식과 ray 교차 판정식에 사용되는 핵심 상수로 사용된다.
 *
 * 이후에는 교차 지점이 실제로 quad의 내부에 포함되는지를 별도로 판단하게 된다.
 */

#endif /* QUAD_HPP */
