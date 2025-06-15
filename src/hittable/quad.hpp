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

    // w = n / (n ⋅ n), quad 가 속한 평면 내 임의의 점 P 를 UV좌표계로 변환하기 위한 캐시 벡터 (하단 필기 참고)
    w = n / dot(n, n);

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
    /** Ray - Plane(quad 가 속한 평면) intersection 검사 (하단 관련 필기 참고) */

    // n ⋅ d → 판별식 분모 계산
    auto denom = dot(normal, r.direction());

    // 판별식 분모가 0 이면 t 의 해가 존재하지 않음. → 기하학적으로 ray 와 평면이 평행하다는 뜻.
    // 따라서, 판별식 분모가 0 에 가까울수록 ray가 평면과 거의 평행함. → 교차 없음으로 간주.
    if (std::fabs(denom) < 1e-8)
    {
      return false;
    }

    // 평면과의 교차 지점 파라미터 t(= 교차 판별식) 계산
    auto t = (D - dot(normal, r.origin())) / denom;

    // t 값(= 교차 지점)이 ray의 유효한 범위 밖이면 무시
    if (!ray_t.contains(t))
    {
      return false;
    }

    // 교차 지점 위치 계산
    auto intersection = r.at(t);

    // hit_record 에 정보 기록
    rec.t = t;
    rec.p = intersection;
    rec.mat = mat;
    rec.set_face_normal(r, normal); // 앞면/뒷면 여부 판정 포함한 노멀 설정

    // 여기까지 통과했으면 교차 성공으로 판단
    return true;
  };

private:
  // quad 를 정의하는 데이터를 private 멤버변수로 정의
  // https://raytracing.github.io/books/RayTracingTheNextWeek.html#quadrilaterals/definingthequadrilateral 참고
  point3 Q;                      // quad의 기준 꼭지점
  vec3 u, v;                     // quad의 두 변 벡터
  vec3 w;                        // quad 가 속한 평면 내 임의의 점 P 좌표를 UV 좌표계로 변환하기 위한 캐시 벡터
  std::shared_ptr<material> mat; // quad에 충돌한 ray 의 산란 계산 시 적용할 material 포인터 멤버변수(reference counting 기반 smart pointer 로 객체 수명 관리)
  aabb bbox;                     // quad를 감싸는 AABB
  vec3 normal;                   // quad 가 속한 평면의 법선 벡터(= 평면의 방향)
  double D;                      // quad 가 속한 평면 방정식의 상수 D = n ⋅ Q
};

/**
 * 주어진 quad 가 속한 평면의 수학적 정의(= 평면의 방정식)
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
 */

/**
 * Ray - Plane(quad 가 속한 평면) intersection 검사
 *
 *
 * 위 주석에서 유도한 평면의 방정식 n ⋅ P = D 를 기반으로 ray와의 교차점을 계산한다.
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

/**
 * 캐시 벡터 w 를 사용하여 quad 가 속한 평면 내 임의의 점 P 를 UV좌표계로 변환하기
 *
 *
 * quad는 평면 위에 정의된 도형이며, 이 평면 위의 한 점 P가 quad 내부에 포함되는지를 판단하려면
 * P가 평면 내에서 어디에 위치해 있는지를 알아야 한다. 이를 위해 평면의 로컬 좌표계인 (u, v) 축을
 * 기준으로 하는 2D 좌표값 (α, β)를 계산한다. 이 좌표값은 다음 조건을 만족한다:
 *
 *    P = Q + α·u + β·v
 *
 * 여기서:
 * - Q: quad의 기준점 (로컬 좌표계의 원점 역할)
 * - u, v: quad의 두 변 벡터 (로컬 좌표계의 기저 축 역할)
 * - α, β: 각각 u, v 방향으로 얼마나 이동했는지를 나타내는 스칼라값
 *
 * α, β를 구하기 위해 우선 p = P - Q 로 잡고,
 * p 를 다음과 같이 식 변형으로 표현할 수 있음을 확인한다:
 *    P = Q + α·u + β·v
 *    → P - Q = α·u + β·v
 *    → p = α·u + β·v
 *
 * 위 식들을 바탕으로 두 벡터식 외적을 전개하면 다음을 얻는다:
 * (구체적인 전개 과정은 https://raytracing.github.io/books/RayTracingTheNextWeek.html#quadrilaterals/derivingtheplanarcoordinates 참고)
 *
 *    v × p = α (v × u)
 *    u × p = β (u × v)
 *
 * 이제 위 식을 두 계수 α, β 에 대한 식으로 변형하면 α, β 를 구할 수 있으므로,
 * 양변에 (v × u) 와 (u × v) 로 나누면 될까? -> NO! 벡터 나눗셈이 정의되지 않기 때문.
 *
 * 그 대신, 양변에 평면의 법선 벡터 n = u × v 를 내적(dot product)하여
 * 스칼라 방정식으로 바꾸면 다음과 같은 형태가 된다:
 *
 *    α = (n ⋅ (p × v)) / (n ⋅ (u × v))
 *    β = (n ⋅ (u × p)) / (n ⋅ (u × v))
 *
 * 여기서 n = u × v 이므로, 분모는 n ⋅ n 이 되고, 이를 미리 계산하여 벡터 w = n / (n ⋅ n) 를 만들어 놓으면
 * 계산이 다음처럼 단순화된다:
 *
 *    α = w ⋅ (p × v)
 *    β = w ⋅ (u × p)
 *
 * 이때 w는 quad 평면에 대해 한 번만 계산하면 되므로, 생성자에서 미리 계산하여 멤버 변수로 캐시해두는 것이 효율적이다.
 *
 * 최종적으로 이 α, β 값이 모두 [0, 1] 범위 내에 있을 경우 해당 점 P는 quad 내부에 존재한다고 볼 수 있고,
 * 동시에 이 값을 UV 텍스처 좌표로 사용할 수도 있다.
 */

#endif /* QUAD_HPP */
