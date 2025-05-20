#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common/rtweekend.hpp"
#include "rtw_stb_image.hpp"

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
 * 공간 기반 절차적 텍스쳐: 체커무늬 텍스쳐 클래스 정의
 * -> 짝수/홀수 셀에 대해 각기 다른 텍스쳐 적용
 */
class checker_texture : public texture
{
public:
  checker_texture(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
      : inv_scale(1.0f / scale), even(even), odd(odd) {};

  // 색상 2개를 받아 각각 solid_color 텍스처로 변환하여 even/odd에 저장
  checker_texture(double scale, const color &c1, const color &c2)
      : checker_texture(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {};

  color value(double u, double v, const point3 &p) const override
  {
    // 좌표 스케일링: scale 값이 커질수록 inv_scale은 작아지고,
    // 이를 좌표값 p 에 곱한 후 정수로 내림하면, 더 넓은 p 영역이 동일한 셀로 맵핑됨 -> 체크무늬 크기가 커짐
    auto xInteger = int(std::floor(inv_scale * p.x()));
    auto yInteger = int(std::floor(inv_scale * p.y()));
    auto zInteger = int(std::floor(inv_scale * p.z()));

    /**
     * std::floor 사용 이유:
     *
     * - 음수 좌표를 포함하여 정확하게 셀 경계를 나누기 위함
     * - ex> truncate(int cast) 는 -0.3, +0.3 모두 0으로 처리되므로, 같은 셀 영역에 속하게 되버려서 경계가 깨짐.
     * 반면 std::floor 는 -0.3은 -1로 내림하고, 0.3은 0으로 내림하므로, -0.3과 0.3 이 서로 다른 셀 영역에 속하게 되어
     * 체커무늬 경계가 깔끔하게 떨어짐.
     */

    // int 캐스팅 이유: std::floor는 double 타입 반환 -> 정수 모듈로 연산을 위해 명시적 int 변환 필요
    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    // 짝수 셀에는 even 텍스처, 홀수 셀에는 odd 텍스처 적용
    return isEven ? even->value(u, v, p) : odd->value(u, v, p);
  };

private:
  double inv_scale;              // 체크 패턴의 크기를 결정하는 scale 역수
  std::shared_ptr<texture> even; // 짝수 셀에 적용할 텍스처
  std::shared_ptr<texture> odd;  // 홀수 셀에 적용할 텍스처
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

/**
 * checker_texture 텍스처
 *
 *
 * 공간 기반(spatial) 절차적 텍스처의 대표적 예시.
 * - 오브젝트의 UV 좌표에 의존하지 않고, 공간상의 위치(point3 p)를 기반으로 색상을 결정함
 * - 3차원 공간 전체가 마치 바둑판 무늬로 채워져 있으며,
 *   객체가 그 속을 지나가며 셀 색상이 바뀌는 효과를 가짐
 *
 * 절차적 텍스처란?
 * - 이미지 텍스처처럼 픽셀 데이터를 참조하는 방식이 아니라,
 *   수학적 계산 또는 알고리즘에 의해 색상을 직접 생성하는 텍스처 방식
 * - 체커 패턴, 노이즈, 마블, 우드 같은 패턴들이 대표적인 예
 *
 * 셰이더 네트워크 구조:
 * - even/odd 텍스처 모두 texture 객체이므로,
 *   solid_color, noise_texture, mix_texture 등 어떤 텍스처도 재귀적으로 삽입 가능
 * - 이로 인해 텍스처 시스템이 노드 기반 그래프처럼 확장 가능하며,
 *   유연한 조합 및 표현이 가능해짐
 */

#endif /* TEXTURE_HPP */
