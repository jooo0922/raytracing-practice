#ifndef HITTABLE_HPP
#define HITTABLE_HPP

#include "common/rtweekend.hpp"
#include "accelerator/aabb.hpp"

/**
 * hit_record 클래스에 material 포인터 멤버변수를 정의해야 하는데,
 * 이 헤더 파일에 material.hpp 포함 시, 순환 참조(circularity of the references) 발생함.
 *
 * 따라서, material 클래스를 전방선언하여 순환 참조를 방지함.
 */
class material;

// hit_record(충돌 정보) 클래스 정의
class hit_record
{
public:
  point3 p;                      // 반직선과 충돌한 지점의 좌표값
  vec3 normal;                   // 반직선과 충돌한 지점의 노멀벡터
  std::shared_ptr<material> mat; // 반직선과 충돌한 object 지점의 산란 계산 시 적용할 material 을 가리키는 포인터
  double t;                      // 반직선 상에서 충돌한 지점이 위치한 비율값 t
  double u;                      // 반직선과 충돌한 지점의 uv 좌표값
  double v;                      // 반직선과 충돌한 지점의 uv 좌표값
  bool front_face;               // 반직선이 hittable 외부/내부에 위치하는지 여부 (관련 필기 하단 참고)

  // ray - hittable 교차점 normal 및 ray 위치 계산 함수 (입력 매개변수 outward_normal 은 항상 단위 벡터로 정규화된다고 가정)
  // 참고로, geometry 특성을 잘 파악하고 있다면, 외부 geometry code(ex> sphere::hit())에서 각 지점의 정규화된 벡터를 미리 계산해서 넘겨주는 게 더 효율적임.
  void set_face_normal(const ray &r, const vec3 &outward_normal)
  {
    // ray - outward_normal 내적값 부호에 따라 ray 가 내부/외부에 위치하는 지 결정
    front_face = dot(r.direction(), outward_normal) < 0;
    // ray 위치에 따라 ray - hittable 교차점 normal 과 ray 가 항상 반대 방향을 향하도록 계산
    normal = front_face ? outward_normal : -outward_normal;
  };
};

// hittable (피충돌 물체) 추상 클래스 정의
class hittable
{
public:
  // 하단 필기 '가상 소멸자와 default' 내용 참고
  virtual ~hittable() = default;

  // 충돌 함수를 순수 가상함수 인터페이스로 정의
  virtual bool hit(const ray &r, interval ray_t, hit_record &rec) const = 0;

  // 현재 hittable 객체를 감싸는 AABB 를 반환하는 순수 가상함수 인터페이스 정의
  virtual aabb bounding_box() const = 0;
};

/**
 * translate 클래스
 *
 * 내부 hittable object를 외부에서 "offset만큼 이동된 객체"처럼 보이게 하는 래퍼 클래스.
 *
 * 핵심 철학은 다음과 같다:
 *
 * 1. object 자체는 변하지 않는다. (정점, AABB 등은 object 로컬 좌표계 기준 그대로 유지)
 * 2. 대신 ray를 역방향으로 변환하여 object 로컬 좌표계로 보내 hit test를 수행한다.
 * 3. 교차 결과(hit point)는 다시 월드 좌표계로 변환하여 일관된 결과를 제공한다.
 * **참고로 "object 로컬 좌표계" 는 실제 hittable object 위치가 원점인 좌표계를 의미**
 *
 * 특히 AABB 처리 방식이 중요하다:
 *
 * - translate::bounding_box()는 내부 object의 AABB에 offset을 적용한 "월드 좌표계 기준의 AABB"이다.
 * - 이 AABB는 BVH 가속 구조에서 월드 좌표계 ray와의 빠른 필터링(bbox 충돌 여부 판단)에 사용된다.
 * - 반면, 내부 object는 로컬 AABB를 사용하고, object 로컬 좌표계로 변환된 ray와 충돌 검사만 수행하면 된다.
 *
 * 이 구조 덕분에:
 * - object는 자신의 좌표계 안에서 "손 하나 까딱하지 않고" 충돌 검사와 AABB 검사만 수행하면 된다.
 * - 모든 변환과 월드-로컬 좌표계 일치 책임은 translate가 맡는다.
 */
class translate : public hittable
{
public:
  translate(std::shared_ptr<hittable> object, const vec3 &offset) : object(object), offset(offset)
  {
    /**
     * 월드 좌표계에서 사용할 이동 변환이 적용된 **월드 좌표계 AABB**를 미리 계산한다.
     * 내부 object의 로컬 AABB에 offset을 더한 값으로, 외부 BVH 가속 구조 상에서 **월드 좌표계 ray-AABB 교차 검사**에 쓰인다.
     */
    bbox = object->bounding_box() + offset;
  };

  bool hit(const ray &r, interval ray_t, hit_record &rec) const override
  {
    // object 로컬 좌표계 내에서 hit test 수행을 위해 **월드 좌표계 ray** 를 **object 로컬 좌표계 ray** 로 변환
    ray offset_r(r.origin() - offset, r.direction(), r.time());

    // **object 로컬 좌표계 ray** 를 전달하여 translate 가 적용될 실제 hittable object 와 hit test 수행
    // 실제 hittable object 는 정점, normal, AABB 전부 로컬 좌표계 기준이므로 아무 수정 없이 hit test를 수행 가능
    if (!object->hit(offset_r, ray_t, rec))
    {
      return false;
    }

    // 충돌 지점은 object 로컬 좌표계 기준으로 계산되었으므로, 이를 다시 월드 좌표계로 복원해줘야 실제 충돌 위치로 해석 가능
    rec.p += offset;

    // 회전이 없는 순수 translation 변환에서는 rec.normal 을 따로 월드 좌표계로 복원하지 않아도 정확함

    return true;
  };

  /**
   * 내부 object 의 월드 좌표계 기준 AABB를 반환
   * BVH 등의 외부 구조에서 사용될 AABB는 반드시 월드 좌표 기준이어야 하므로, offset 적용됨.
   * 덕분에 로컬 object 의 월드 좌표계 기준 BVH 트리 가속 필터링 작업을 translate::bounding_box() 에 위임할 수 있게 됨.
   */
  aabb bounding_box() const override { return bbox; };

private:
  std::shared_ptr<hittable> object; // 로컬 좌표계 기준으로 정의된 실제 hittable object
  vec3 offset;                      // object가 이동된 것처럼 보이게 할 translation vector -> 실제로는 월드 좌표계 ray 원점이 offset 만큼 이동됨.
  aabb bbox;                        // 월드 좌표계 기준 이동 변환된 AABB (object->bbox + offset)
};

/*
  가상 소멸자(destructor)와 default

  2. 가상(virtual) 소멸자

  그런데 소멸자에 virtual 이 붙는 이유는 뭘까?

  class Parent
  {
  public:
    ~Parent()
    {
      // ...
    }
  }

  class Child : public Parent
  {
  public:
    ~Child()
    {
      // ...
    }
  }

  위와 같은 상속관계를 갖는 두 클래스에서
  각각 일반 소멸자를 선언해둔 상태라면,

  int main()
  {
    Parent* ptr = new Child();

    delete ptr;

    return 0;
  }

  위와 같이 부모 클래스를 타입으로 하는 포인터 변수에
  자식 클래스를 객체로 생성하여 동적으로 메모리를 할당할 때,

  delete 키워드로 객체의 메모리를 해제하는 시점에
  부모 클래스의 소멸자인 ~Parent() 만 제대로 호출되고,
  자식 클래스의 소멸자인 ~Child() 는 호출되지 않는 문제점이 있음.

  만약, 자식 클래스 소멸자에서 특정 리소스 해제를 관리하고 있다면,
  리소스 해제가 제대로 발생하지 않아 메모리 누수로 이어질 수도 있겠지!

  따라서, 위와 같은 상황에서도 자식 클래스의 소멸자까지 깔끔하게 호출하고 싶다면,
  부모 클래스 소멸자에 virtual 키워드를 붙여 가상 소멸자로 만들어야 함!

  이렇게 함으로써, 부모 클래스를 상속받는 자식 클래스들의 소멸자들도
  모두 '가상 소멸자'로 자동 선언되고,

  객체의 소멸자가 호출되는 시점(delete 사용 등)에
  상속 계층 구조 상 맨 아래에 존재하는 자식 클래스의 소멸자부터 호출되고,
  순차적으로 부모 클래스의 소멸자까지 호출시킴으로써,

  상속 계층에 존재하는 모든 클래스의 소멸자를 호출시켜
  메모리 누수를 완전 방지할 수 있다!


  3. default

  생성자(constructor) 또는 소멸자(destructor) 에는
  default 키워드를 붙여서 '디폴트 생성자' 또는 '디폴트 소멸자'를 선언할 수 있음.

  디폴트는 별 다른 게 아니고,
  만약 어떤 클래스의 생성자와 소멸자가 아래와 같이 구현되어 있다면,

  class A
  {
  public:
    A()
    {
      // 구현부에서 아무런 작업도 하지 않는 생성자
    }

    ~A()
    {
      // 구현부에서 아무런 작업도 하지 않는 소멸자
    }
  }

  위와 같이 클래스 내에 정의한 생성자와 소멸자가
  구현부에서 아무런 작업도 하지 않는다면,
  굳이 저렇게 코드를 '구현부까지' 작성하기 귀찮잖아?

  이럴 때 아래와 같이 default 키워드를 사용해서
  컴파일러에게 아무런 작업도 수행하지 않는 기본 생성자와 기본 소멸자를
  자동으로 만들어두라고 명시적으로 선언하는 키워드라고 보면 됨.

  class A
  {
  public:
    A() = default;
    ~A() = default;
  }

  이렇게 하면 똑같이 아무런 작업도 수행하지 않는
  기본 생성자와 기본 소멸자를 간결한 코드로 작성할 수 있다는 장점이 있음!
*/

/**
 * hit_record::front_face
 *
 *
 * ray 가 hittable 외부 또는 내부 중 어디에 위치하는지 정보를 저장하는 멤버변수
 *
 * 이게 필요한 이유는 hittable 오브젝트의 안쪽 면을 렌더링할 지,
 * 바깥 쪽 면을 렌더링할 지 파악하기 위함.
 *
 * ray 가 hittable 내부에서 출발했다면,
 * 카메라가 hittable 내부에 위치한다는 뜻이므로,
 * hittable 안쪽 면(back face)을 렌더링해야 함.
 *
 * 반면, ray 가 hittable 외부에서 출발했다면,
 * 카메라가 hittable 외부에 위치한다는 뜻이므로,
 * hittable 바깥쪽 면(front face)를 렌더링해야 함.
 *
 * 따라서, ray 위치에 따라 hittable 의 렌더링 면을 결정할 수 있게 되는 것임.
 *
 *
 * 또한, 안쪽 면을 쉐이딩하려면 ray - hittable 교차점의 inward normal 이 필요하고,
 * 바깥쪽 면을 쉐이딩하려면 반대로 outward normal 이 필요함.
 * 이때, ray - hittable 교차점 normal 과 ray 위치를 관리하는
 * 2가지 design decision 이 존재함.
 *
 *
 * 1. geometry 단계에서 normal 이 항상 outward 를 향하도록 계산하면,
 * shading(coloring) 단계에서 ray 와 outward normal 을 내적하여 ray 위치를 파악해야 함.
 *
 * 2. 반면, normal 이 항상 ray 의 반대 방향을 향하도록 계산하면,
 * geometry intersection 단계(= hittable::hit() 함수)에서 ray 위치를 미리 계산해서
 * hit_record::front_face 상태값에 저장해두고 사용할 수 있음.
 *
 *
 * 두 방식 모두 어차피 교차할 때마다 ray 위치를 파악해야 하므로,
 * 실질적인 연산량은 동일함.
 *
 * 그러나, 이 튜토리얼에서는 만들어야 할 material 클래스들이 아주 많기 때문에,
 * 여러 개의 material 클래스(= 즉, shading 단계)를 만들 때마다
 * ray 와 outward 간 내적으로 ray 위치 파악 코드 중복 작성을 피하기 위해,
 *
 * geometry intersection 단계에서 ray 위치 파악 코드를 한 번 작성해두고
 * hit_record::front_face 에 결과값을 저장해버리면,
 * 여러 material 클래스에서 해당 상태값을 참조해서 front face/back face 렌더링 여부를 결정할 수 있음.
 *
 *
 * 따라서, 코드의 유지 보수 및 가독성을 고려해
 * 2번 방식으로 normal 과 ray 위치를 관리하도록 함.
 */

#endif /* HITTABLE_HPP */
