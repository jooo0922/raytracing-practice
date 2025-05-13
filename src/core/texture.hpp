#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common/rtweekend.hpp"

/**
 * texture 추상 클래스
 */
class texture
{
public:
  // texture 을 상속받는 자식 클래스들의 소멸자도 정상 호출되도록 부모 클래스의 소멸자를 가상 소멸자로 정의
  // https://github.com/jooo0922/cpp-study/blob/main/Chapter12/Chapter12_04/Chapter12_04.cpp 참고
  virtual ~texture() = default;

  // 텍스쳐 좌표(u, v) 또는 공간 상 위치(p)에 따른 색상값을 반환하는 인터페이스를 자식 클래스에서 재정의하도록 가상함수로 정의 -> 재정의할 세부 동작 encapsulate
  virtual color value(double u, double v, const point3 &p) const = 0;
};

#endif /* TEXTURE_HPP */
