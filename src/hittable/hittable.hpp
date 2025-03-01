#ifndef HITTABLE_HPP
#define HITTABLE_HPP

#include "ray.hpp"

// hit_record(충돌 정보) 클래스 정의
class hit_record
{
public:
  point3 p;    // 반직선과 충돌한 지점의 좌표값
  vec3 normal; // 반직선과 충돌한 지점의 노멀벡터
  double t;    // 반직선 상에서 충돌한 지점이 위치한 비율값 t
};

// hittable (피충돌 물체) 추상 클래스 정의
class hittable
{
public:
  // 하단 필기 '가상 소멸자와 default' 내용 참고
  virtual ~hittable() = default;

  // 충돌 함수를 순수 가상함수 인터페이스로 정의
  virtual bool hit(const ray &r, double ray_tmin, double ray_tmax, hit_record &rec) const = 0;
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

#endif /* HITTABLE_HPP */
