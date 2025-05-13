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

/**
 * 단일 색상 고정 텍스쳐 클래스 정의 (하단 필기 참고)
 */
class solid_color : public texture
{
public:
  solid_color(const color &albedo) : albedo(albedo) {};

  solid_color(double red, double green, double blue)
      : solid_color(color(red, green, blue)) {};

  // 입력 좌표(u, v, p)와 무관하게 항상 동일한 색상(albedo) 반환
  color value(double u, double v, const point3 &p) const override
  {
    return albedo;
  };

private:
  color albedo; // 고정된 색상 값 (constant color)
};

/**
 * solid_color 텍스처
 *
 *
 * - "모든 색상을 텍스처로 만든다"는 아키텍처 철학을 구현한 클래스
 * - 단색도 텍스처로 추상화함으로써, 다른 텍스처들과 동일한 방식(value(u,v,p))으로 처리됨
 * - 텍스처 조합, 절차적 패턴, 쉐이더 네트워크 등 다양한 곳에 재사용 가능
 *
 * 주요 특징:
 * - (u, v, p) 입력값과 관계없이 항상 고정된 색상을 반환
 * - 이미지 텍스처와 달리 메모리를 거의 사용하지 않음
 * - 단순하지만 텍스처 시스템의 일관성을 유지하는 데 중요한 역할을 함
 *
 * 사용 예시:
 * - checker_texture의 even/odd 인자로 사용
 * - mix_texture, noise_texture 등 다른 텍스처와 혼합
 */

#endif /* TEXTURE_HPP */
