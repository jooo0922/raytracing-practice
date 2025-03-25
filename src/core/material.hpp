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
 * Lambertian(diffuse) reflectance 산란 동작을 재정의하는 material 클래스 정의
 */
class lambertian : public material
{
public:
  lambertian(const color &albedo) : albedo(albedo) {};

  // Lambertian(diffuse) reflectance 산란 동작 재정의
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
  {
    // Lambertian distribution 기반 scattered_ray 계산 (하단 Lambertian distribution 필기 참고)
    vec3 scatter_direction = rec.normal + random_unit_vector();
    scattered = ray(rec.p, scatter_direction);

    // 입자에 흡수(감쇄)되고 남은 난반사(albedo)를 충돌한 ray 산란(반사)될 때의 attenuation 값으로 할당.
    attenuation = albedo;

    // (일정 확률로)산란할 ray 생성에 성공했다면 true 반환
    // TODO : 추후 일정 확률로 ray 를 산란하지 않고 모두 흡수해버리는 코드도 추가될 수 있음.
    return true;
  };

private:
  color albedo; // 난반사되는 물체의 색상값
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

/**
 * Lambertian distribution
 *
 *
 * 기존 random_on_hemisphere() 함수를 기반으로 ray 방향벡터를 생성하는 방식은
 * 충돌한 표면에 접하는 반구 영역 내에서 랜덤한 방향으로 uniform 하게 ray 를 산란시킴.
 * -> 렌더링 결과가 부드러워 보이지만, 물리적으로 비사실적 렌더링임.
 *
 * 반면, 실제 diffuse object(난반사 물체)는 Lambertian distribution 을 따르는데,
 * 조명 벡터가 충돌한 표면 지점에서 산란되는 대부분의 ray 가 표면의 normal vector 와 가깝게 산란되어야 함.
 * -> 이러한 ray 산란 분포는 물리적으로 더 정확한 diffuse reflectance 을 구현함.
 *
 * 이러한 Lambertian 모델은 기존 그래픽스 파이프라인에서는 '빛의 세기' 관점에서 구현됨.
 * 즉, 프래그먼트 쉐이더 내에서 조명 벡터와 표면의 normal vector 간 내적(dot())을 통해
 * cos 값을 구하게 되고, 이 cos 값을 기반으로 해당 표면에서 반사되는 '빛의 세기' 를 결정함으로써
 * Lambertian 모델을 구현한다면,
 *
 * Ray Tracing 에서는 조명을 recursive ray bouncing 기반으로 계산하게 되므로,
 * 충돌한 표면에서 다음 ray 의 방향을 생성할 때, 표면의 normal vector 에 가까운 방향일수록
 * 확률적으로 더 많은 ray 를 생성시키고, 먼 방향일수록 확률적으로 더 적은 ray 를 생성시키는
 * 'ray 산란 확률 분포' 관점에서 Lambertian 모델이 구현된 것임.
 *
 * 즉, 물리적으로 동일한 모델(Lambertian)을 그래픽스 파이프라인의 특성에 따라
 * 서로 다른 방식으로 구현한 것으로 보면 됨.
 *
 * -> 그래서 실제로 본문에서도 Lambertian 산란 분포는 '조명 벡터와 충돌 표면의 normal vector 간
 * 사잇각의 cos 값에 비례한 분포로 산란된다(~ scatters reflected rays in a manner that is proportional to cos(), ~)'
 * 라고 설명하고 있음. 즉, 두 방식은 본질적으로 동일한 물리 조명 모델을 서로 다른 방식으로 구현하고 있는 것 뿐임!
 *
 *
 * 이때, 충돌 표면의 normal vector 에 더 가까운 ray 를 확률적으로 많이 생성해내는 방법이
 *
 * vec3 direction = rec.normal + random_unit_vector();
 *
 * 즉, '충돌 표면의 normal vector + 충돌 표면 바깥 쪽에 접하는 unit sphere 내의 랜덤 방향벡터' 인 것임!
 *
 * 충돌 표면 바깥쪽 unit sphere 내의 랜덤 방향벡터가 어떤 방향이든 간에,
 * 해당 방향벡터에 충돌 표면 normal vector 를 더하면, 벡터의 합의 성질에 의해
 * 해당 랜덤 방향벡터가 충돌 표면 normal vector 쪽으로 끌려가게 됨.
 *
 * 이렇게 생성된 ray 방향은 충돌 표면 normal vector 쪽으로 가까워질 수밖에 없고,
 * 결과적으로 생성된 ray 들을 보면 전체적으로 normal vector 에 가까운 쪽에 더 많은
 * ray 가 생성된 것처럼 보이는 분포를 보이게 됨.
 */

#endif /* MATERIAL_HPP */
