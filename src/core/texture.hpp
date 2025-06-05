#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common/rtweekend.hpp"
#include "rtw_stb_image.hpp"
#include "perlin.hpp"

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
 * 이미지 기반 텍스쳐 클래스 정의
 * -> rtw_image 클래스로부터 로드된 이미지 데이터 적용
 */
class image_texture : public texture
{
public:
  image_texture(const char *filename) : image(filename) {};

  // 입력된 uv 좌표값으로 로드된 이미지의 픽셀 데이터를 읽어서 반환
  color value(double u, double v, const point3 &p) const override
  {
    // 로드된 이미지가 없으면 디버깅을 위해 cyan 색상 반환
    if (image.height() <= 0)
    {
      return color(0.0f, 1.0f, 1.0f);
    }

    // [0.0, 1.0] 범위 내로 uv 좌표값 clamping
    u = interval(0.0f, 1.0f).clamp(u);
    // uv 좌표계와 이미지 좌표계의 수직 방향(v축)이 서로 반대이므로 v 좌표를 뒤집어 줌.
    v = 1.0f - interval(0.0f, 1.0f).clamp(v);

    // uv 좌표를 정수형 이미지 픽셀 좌표로 casting 하여 픽셀 데이터를 읽어온다.
    auto i = int(u * image.width());
    auto j = int(v * image.height());
    auto pixel = image.pixel_data(i, j);

    // rtw_image 에서 읽어온 8-bit RGB 픽셀 데이터([0, 255])를 [0.0f, 1.0f] 범위의 float 색상값으로 반환
    auto color_scale = 1.0f / 255.0f;
    return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
  };

private:
  rtw_image image; // stb_image 라이브러리를 래핑하여 이미지 데이터 로드, 변환, 읽기 등을 처리하는 헬퍼 클래스 멤버
};

/**
 * perlin noise 알고리즘 기반 노이즈 텍스쳐(절차적 텍스쳐) 클래스 정의
 */
class noise_texture : public texture
{
public:
  noise_texture(double scale) : scale(scale) {};

  // 입력된 point3 좌표값을 기반으로 계산된 perlin noise 값을 grayscale 색상으로 반환
  color value(double u, double v, const point3 &p) const override
  {
    /** trilinear interpolation 기반 perlin noise 사용 */
    // scale 값을 입력 좌표에 곱하여 perlin noise의 노이즈 주기를 높임(더 자글자글한 노이즈로 보이도록...)
    // -> perlin::noise_hash() 함수에서 입력 좌표를 4로 스케일링하여 격자 해상도를 높인 것과 동일한 원리!
    // return color(1.0f, 1.0f, 1.0f) * noise.noise_trilinear(scale * p);

    /** random gradient vector 내적값 기반 perlin noise 사용 */
    // -> 내적값의 범위가 [-1.0, 1.0] 이므로, noise 값 범위도 동일. 따라서, 해당 범위를 [0.0, 1.0] 으로 맵핑시켜 최종 색상값 계산.
    // -> 음수인 색상값이 나오면 안되니까!
    // return color(1.0f, 1.0f, 1.0f) * 0.5f * (1.0f + noise.noise_perlin(scale * p));

    /** 여러 주파수 단계별 noise 를 중첩시켜서 복잡한 패턴을 만드는 turbulence noise 사용 */
    // 7단계 주파수 noise 를 중첩시킨 noise 값을 grayscale 색상으로 변환
    // return color(1.0f, 1.0f, 1.0f) * noise.turb(p, 7);

    /** z축 방향 sin 파형에 turbulence noise 를 더해 위상(phase)을 불규칙하게 흔든 marble 무늬 생성 (하단 필기 참고) */
    return color(0.5f, 0.5f, 0.5f) * (1.0f + std::sin(scale * p.z() + 10.0f * noise.turb(p, 7)));
  };

private:
  perlin noise; // perlin noise 알고리즘으로 절차적 노이즈를 생성하는 클래스
  double scale; // perlin noise 의 공간적 주기(frequency)를 조절하는 스케일 값
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

/**
 * turbulence noise 를 응용하여 marble pattern 생성하기
 *
 *
 * z축 방향의 거리값에 따라 sin(scale * p.z()) 파형을 색상값으로 변환하면,
 * z축 방향으로 규칙적인 물결 패턴(= sin 파)이 렌더링된다.
 *
 * 여기에 noise.turb(p, 7)를 위상(phase shift) 값으로 더함으로써,
 * 규칙적인 sin 곡선을 turbulence noise 기반으로 불규칙하게 흔든 형태의 패턴이 생성된다.
 *
 * sin 함수의 입력값(= 입력 좌표의 z값이 기본.)이 turbulence noise 에 의해 불규칙적으로 변형됨에 따라
 * 결과적으로 스트라이프가 물결치듯 변형된 marble(대리석) 질감이 형성된다.
 * 이처럼 turbulence는 단순한 패턴에 변형을 주는 방식으로 자주 활용된다.
 */

#endif /* TEXTURE_HPP */
