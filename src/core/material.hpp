#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "common/rtweekend.hpp"
#include "hittable/hittable.hpp"

/**
 * material 추상 클래스
 */
class material
{
public:
  // material 을 상속받는 자식 클래스들의 소멸자도 정상 호출되도록 부모 클래스의 소멸자를 가상 소멸자로 정의
  // https://github.com/jooo0922/cpp-study/blob/main/Chapter12/Chapter12_04/Chapter12_04.cpp 참고
  virtual ~material() = default;

  // ray 충돌 시 산란 방식을 정의하는 인터페이스를 자식 클래스에서 재정의하도록 가상함수로 정의 -> 재정의할 세부 동작 encapsulate
  virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const { return false; };
};

/**
 * material::scatter() 역할
 *
 *
 * 가상함수로 캡슐화된 material::scatter() 함수의 역할은 아래 두 가지로 구현되어야 함.
 *
 * 1. ray 충돌 지점에서 각 material 특성에 맞게 산란할 ray 를 생성한다. (또는 일정 확률로 ray 를 흡수(= 생성x)한다.)
 * 2. 만약 (일정 확률로)산란할 ray 를 생성했다면, 그 산란된 ray 가 얼마나 감쇄(attenuation)되어야 할 지 결정한다.
 *
 * 여기서, 2번 역할의 감쇄가 어떤 의미인 지 파악해야 함.
 *
 * 물리적인 관점에서 보면, ray 충돌 지점에서 굴절된 광선들 중
 * 일부는 입자에 흡수되고, 일부는 다시 표면 밖으로 빠져나온다.
 * 다시 빠져나오는 난반사는 물체의 색상(albedo)이 됨.
 *
 * 이때, 입자에 흡수되고 남은 난반사를 '감쇄되었다(attenuation)'고 표현한 것임.
 *
 * 그래서 가상함수를 재정의하는 자식 클래스 lambertian::scatter() 함수 내에서
 * 출력 참조변수 attenuation 에 albedo 색상값을 그대로 복사하여 사용함.
 *
 * -> why? 입자에 흡수(감쇄)되고 남은 난반사 자체가 인간의 눈에는 물체의 색상으로 보이는 거니까!
 */

#endif /* MATERIAL_HPP */
