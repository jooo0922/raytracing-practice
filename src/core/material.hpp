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

    // 산란할 ray 방향이 영벡터가 되지 않도록 예외 처리 (하단 필기 참고)
    if (scatter_direction.near_zero())
    {
      scatter_direction = rec.normal;
    }

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
 * Metallic reflectance 산란 동작을 재정의하는 material 클래스 정의
 */
class metal : public material
{
public:
  metal(const color &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1.0f ? fuzz : 1.0f) {};

  // Metallic reflectance 산란 동작 재정의
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
  {
    // metallic 표면에 충돌한 incident ray 의 반사벡터 계산 (하단 필기 참고)
    vec3 reflected = reflect(r_in.direction(), rec.normal);
    // 반사벡터의 end point 를 중점으로 하는 퍼짐 구(= fuzz sphere) 상의 임의의 점으로 반사벡터의 end point 업데이트 -> 반사벡터를 약간씩 randomize 함 (하단 필기 참고)
    // (이때, 반사벡터 길이에 따라 퍼짐 구(= fuzz sphere) 상의 random vector 와 벡터의 합 결과가 달라지므로, 일관된 효과 보장을 위해 반사벡터의 길이를 정규화해야 함.)
    reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
    scattered = ray(rec.p, reflected);

    // metal 재질에서의 albedo 는 감쇄된 난반사 색상이 아닌, 파장마다 반사율 차이로 인한 정반사(specular reflection)의 색조(tint)로 봐야 함.
    attenuation = albedo;

    /**
     * 퍼짐 구가 너무 크거나, incident ray 가 표면과 거의 평행하게 스치듯이(= grazing ray) 들어오면,
     * randomize 된 반사 벡터가 표면 아래로 진행될 수 있음.
     *
     * -> 이럴 경우, 표면(정확히는 금속 표면 상 비금속 이물질)이 광선을 흡수한 것으로 판단해서
     * false 를 return 하도록 함.
     */
    return (dot(scattered.direction(), rec.normal) > 0);
  };

private:
  color albedo; // 난반사되는 물체의 색상값
  double fuzz;  // 금속의 specular reflection 을 randomize 하기 위한 퍼짐 구(= fuzz sphere) 반지름
};

/**
 * dielectric(비전도체)의 산란 동작을 재정의하는 material 클래스 정의
 *
 * 참고로, dielectric 은 전기가 통하지 않는 절연체를 의미하며,
 * 빛이 물질을 통과할 때 반사(reflection)와 굴절(refraction)이 동시에 일어나는 모든 (투명한) 비전도체 물질을 뜻함.
 *
 * 유리, 물, 다이아몬드, 플라스틱, 얼음, 아크릴 등이
 * 모두 비전도체에 속한다.
 */
class dielectric : public material
{
public:
  dielectric(double refraction_index) : refraction_index(refraction_index) {};

  // dielectric 산란 동작 재정의
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
  {
    // 광선이 반사 또는 굴절 시 아무런 감쇄 없이 100% 투과 -> 즉, 비전도체 중에서도 물, 유리 등 이상적인 투명체에 대한 산란 동작만 구현.
    attenuation = color(1.0f, 1.0f, 1.0f);

    /**
     * Snell's Law 기반 굴절광선 계산 시 필요한
     * 입사광선 R 의 위치에 따른 두 매질의 상대 굴절률 η / η′ 계산
     *
     * 충돌 지점이 바깥쪽 표면이라면,
     * 입사광선 R 이 속한 매질이 진공이므로, 진공의 굴절률 1.0 이 분자 η 이 되고,
     *
     * 충돌 지점이 안쪽 표면이라면,
     * 입사광선 R 이 속한 매질이 현재 재질이므로, 현재 재질의 절대 굴절률 refraction_index 이 분자 η 이 된다.
     */
    double ri = rec.front_face ? (1.0f / refraction_index) : refraction_index;

    // 굴절광선 계산을 위해 입사광선 R 정규화
    vec3 unit_direction = unit_vector(r_in.direction());
    // 입사광선 R 이 현재 재질에서 충돌했을 때의 굴절광선 R' 계산
    vec3 refracted = refract(unit_direction, rec.normal, ri);

    // 굴절광선 R' 을 다음 산란 방향으로 정의
    scattered = ray(rec.p, refracted);
    return true;
  };

private:
  /**
   * 주변 매질에 대한 현재 재질의 상대 굴절률로써,
   * 실제 사용자는 아래 두 가지 케이스 중 하나로 굴절률을 계산하여 넘겨주어야 함.
   *
   * 1. 진공/공기 기준 현재 재질의 절대 굴절률(= 주변 매질이 진공(굴절률 1.0)인 상대 굴절률이라고도 볼 수 있음.)
   * 2. 임의의 주변 매질에 대한 현재 재질의 상대 굴절률
   */
  double refraction_index;
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

/**
 * Lambertian reflectance scattering modeling 방식
 *
 *
 * 원문에서 Lambertian reflectance 의 산란 동작을 구현하는 3가지 방법을 제시하고 있음.
 *
 * 1.
 * 표면에 ray 가 충돌할 때마다 항상 산란할 ray 를 계산하되,
 * 반사율 R(= 여기서는 Material 에 저장된 albedo)만큼 해당
 * 방향으로 산란된 ray 세기를 감쇄시키는 방법. 가장 구현이 간단하고 용이함.
 * -> 현재 lambertian::scatter() 함수에 구현된 방법
 *
 * 2.
 * 표면에 ray 가 충돌할 때, 애초부터 산란할 ray 생성 확률을 제한함.
 * 그래서 albedo 를 사용한 감쇄를 처리하지 않더라도, 반사(산란)되는 ray 의 확률을
 * (1 - 반사율 R) 만큼으로 제한하여 처음부터 산란되는 ray 수를 줄여서 빛의 감쇄를 구현.
 *
 * 3.
 * 표면에 ray 가 충돌할 때, 고정된 산란 확률 p(= some fixed probability p)만큼 ray 생성을 제한함.
 * 이때, 2번 방법과의 차이점은 일정 확률로 산란할 ray 생성 시, 해당 ray 의 세기를 (albedo / p) 만큼 감쇄시킴.
 * ray 생성 확률도 제한하고 감쇄도 적용한다는 점에서 1번과 2번 방법의 조합(= mixture of both those strategies)
 * 으로 볼 수도 있음.
 */

/**
 * 산란할 ray 방향이 영벡터 예외 처리
 *
 *
 * 충돌 표면 바깥 쪽 unit sphere 내 랜덤 벡터의 방향이 충돌 지점 normal vector 와 반대 방향이라면?
 * 두 벡터를 더해서 계산하면 영벡터가 나올 것임.
 *
 * 그런데, 산란할 ray 방향이 영벡터가 되면
 * 해당 벡터를 정규화하는 과정에서 bogus vector 생성할 수도 있고,
 * 이 외에도 충돌 계산에서 여러 문제가 발생할 수 있음.
 *
 * 이같은 영벡터에 의한 문제들을 방지하기 위한 예외 처리 필수!
 */

/**
 * Metallic reflectance
 *
 *
 * 이론적으로 metal(금속)은 굴절된 빛을 모두 흡수하기 때문에,
 * 굴절된 빛이 표면 밖으로 탈출하는 난반사(diffuse reflection)가 존재하지 않음.
 *
 * 따라서, metal 재질 표면에서 충돌한 ray 의 다음 진행 방향을 계산하려면,
 * 해당 표면의 normal vector 방향을 기준으로 (굴절 없이)곧바로 반사되는 '정반사(specular reflection)'만을
 * 고려하여 ray 의 다음 진행 방향을 결정하도록 산란 방식을 정의한 것임.
 *
 *
 * Fuzziness
 *
 *
 * 이론적으로 metal(금속)은 난반사가 없기는 하지만,
 * 현실에서는 실제 금속 표면 위에 먼지, 스크래치, 산화막 등의 비금속 이물질로 인해
 * 아주 약간의 난반사가 발생하거나 미세하게 흩어질 수 있음.
 *
 * metal 클래스에 구현된 fuzz 파라미터는
 * 이처럼 실제 금속 표면 위의 비금속 이물질로 인한 난반사 효과를
 * 간소화해서 모델링한 것이라고 이해하면 됨.
 *
 * 그래서, 원본 반사벡터의 end point 를 중점으로 하는 퍼짐 구(= fuzz sphere) 상의 임의의 점으로
 * 반사벡터의 end point 업데이트하여 반사벡터를 일정 각도 범위 내에서 약간씩 randomize 함으로써
 * 비금속 이물질의 난반사를 흉내내려는 것!
 */

#endif /* MATERIAL_HPP */
