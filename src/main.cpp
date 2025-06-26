#include "common/rtweekend.hpp" // common header 최상단에 가장 먼저 include (관련 필기 하단 참고)
#include "accelerator/bvh_node.hpp"
#include "core/camera.hpp"
#include "core/material.hpp"
#include "core/texture.hpp"
#include "hittable/hittable.hpp"
#include "hittable/hittable_list.hpp"
#include "hittable/sphere.hpp"
#include "hittable/quad.hpp"

// bouncing spheres scene 렌더링 함수
void bouncing_spheres(std::ofstream &output_file)
{
  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;

  /** 각 Hittable 객체에 적용할 재질(Material)을 shared_ptr로 생성하여 공유 가능하도록 관리 */
  // checker texture 생성 후 ground_material 에 적용
  auto checker = std::make_shared<checker_texture>(0.32f, color(0.2f, 0.3f, 0.1f), color(0.9f, 0.9f, 0.9f));
  auto ground_material = std::make_shared<lambertian>(checker);
  world.add(std::make_shared<sphere>(point3(0.0f, -1000.0f, -1.0f), 1000.0f, ground_material)); // 반지름이 1000 인 지면 sphere 추가

  /** 484개(= 22 * 22)의 소형 sphere 생성 후 world 에 추가 */
  for (int a = -11; a < 11; a++)
  {
    for (int b = -11; b < 11; b++)
    {
      // 소형 sphere 에 적용할 재질 유형을 결정하기 위한 [0.0, 1.0] 사이의 난수 생성
      auto choose_mat = random_double();
      // 격자 좌표(a, 0.2, b) 주변에 약간의 난수 오프셋을 더해, sphere 위치를 자연스럽게 분산시키기
      point3 center(a + 0.9f * random_double(), 0.2f, b + 0.9f * random_double());

      // 특정 위치(4, 0.2, 0)에 배치된 대형 금속 sphere 와 겹치지 않도록 하기 위해, 해당 위치로부터 일정 거리 이상 떨어진 경우에만 소형 sphere 추가
      if ((center - point3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
      {
        std::shared_ptr<material> sphere_material;

        if (choose_mat < 0.8f)
        {
          // 80% 확률로 diffuse material 적용하여 소형 sphere 생성
          // 정적 sphere(center) 대신 time=0 ~ 1 사이 선형 이동하는 moving sphere(center → center2)로 생성
          auto albedo = color::random() * color::random();
          sphere_material = std::make_shared<lambertian>(albedo);
          auto centrt2 = center + vec3(0.0f, random_double(0.0f, 0.5f), 0.0f);
          world.add(std::make_shared<sphere>(center, centrt2, 0.2f, sphere_material));
        }
        else if (choose_mat < 0.95f)
        {
          // 15% 확률로(0.95 - 0.8 = 0.15) metal material 적용하여 소형 sphere 생성
          auto albedo = color::random(0.5f, 1.0f);
          auto fuzz = random_double(0.0f, 0.5f);
          sphere_material = std::make_shared<metal>(albedo, fuzz);
          world.add(std::make_shared<sphere>(center, 0.2f, sphere_material));
        }
        else
        {
          // 5% 확률로(1.0 - 0.95 = 0.05) glass material 적용하여 소형 sphere 생성
          sphere_material = std::make_shared<dielectric>(1.5f);
          world.add(std::make_shared<sphere>(center, 0.2f, sphere_material));
        }
      }
    }
  }

  /** 각 재질이 적용된 3개의 대형 sphere 생성 후 world 에 추가 */
  auto material1 = std::make_shared<dielectric>(1.5f);
  world.add(std::make_shared<sphere>(point3(0.0f, 1.0f, 0.0f), 1.0f, material1));

  auto material2 = std::make_shared<lambertian>(color(0.4f, 0.2f, 0.1f));
  world.add(std::make_shared<sphere>(point3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

  auto material3 = std::make_shared<metal>(color(0.7f, 0.6f, 0.5f), 0.0f);
  world.add(std::make_shared<sphere>(point3(4.0f, 1.0f, 0.0f), 1.0f, material3));

  // 현재 world 내의 hittable 객체들을 가지고서 BVH 트리를 구축함.
  world = hittable_list(std::make_shared<bvh_node>(world));

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 50;
  cam.max_depth = 20;
  // ray 와 충돌한 물체가 없을 경우 반환할 scene 배경색(solid color. no gradient) 정의
  cam.background = color(0.7f, 0.8f, 1.0f);

  // camera transform 관련 파라미터 설정
  cam.vfov = 20.0f;
  cam.lookfrom = point3(13.0f, 2.0f, 3.0f);
  cam.lookat = point3(0.0f, 0.0f, 0.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 0.6f;
  cam.focus_dist = 10.0f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, world);
}

// checkered spheres scene 렌더링 함수
void checkered_spheres(std::ofstream &output_file)
{
  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;

  /** 각 Hittable 객체에 적용할 재질(Material)을 shared_ptr로 생성하여 공유 가능하도록 관리 */
  // checker texture 생성 후 lambertian material 에 적용
  auto checker = std::make_shared<checker_texture>(0.32f, color(0.2f, 0.3f, 0.1f), color(0.9f, 0.9f, 0.9f));
  // 반지름이 10 인 두 checkered sphere 추가
  world.add(std::make_shared<sphere>(point3(0.0f, -10.0f, 0.0f), 10.0f, std::make_shared<lambertian>(checker)));
  world.add(std::make_shared<sphere>(point3(0.0f, 10.0f, 0.0f), 10.0f, std::make_shared<lambertian>(checker)));

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 50;
  cam.max_depth = 20;
  // ray 와 충돌한 물체가 없을 경우 반환할 scene 배경색(solid color. no gradient) 정의
  cam.background = color(0.7f, 0.8f, 1.0f);

  // camera transform 관련 파라미터 설정
  cam.vfov = 20.0f;
  cam.lookfrom = point3(13.0f, 2.0f, 3.0f);
  cam.lookat = point3(0.0f, 0.0f, 0.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 0.0f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, world);
};

// earth scene 렌더링 함수
void earth(std::ofstream &output_file)
{
  // earthmap.jpg 이미지 로드 및 적용을 위해 image_texture 생성 후 lambertian material 에 적용
  auto earth_texture = std::make_shared<image_texture>("earthmap.jpg");
  auto earth_surface = std::make_shared<lambertian>(earth_texture);
  // 반지름이 2 인 glob sphere 생성
  auto globe = std::make_shared<sphere>(point3(0.0f, 0.0f, 0.0f), 2.0f, earth_surface);

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 100;
  cam.max_depth = 50;
  // ray 와 충돌한 물체가 없을 경우 반환할 scene 배경색(solid color. no gradient) 정의
  cam.background = color(0.7f, 0.8f, 1.0f);

  // camera transform 관련 파라미터 설정
  cam.vfov = 20.0f;
  cam.lookfrom = point3(0.0f, 0.0f, 12.0f);
  cam.lookat = point3(0.0f, 0.0f, 0.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 0.0f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, hittable_list(globe));
};

// perlin noise sphere scene 렌더링 함수
void perlin_sphere(std::ofstream &output_file)
{
  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;

  // perlin noise 적용을 위해 noise_texture 생성 후 lambertian material 에 적용
  auto pertext = std::make_shared<noise_texture>(4);
  // 반지름이 각각 2, 1000 인 두 sphere 추가
  world.add(std::make_shared<sphere>(point3(0.0f, -1000.0f, 0.0f), 1000.0f, std::make_shared<lambertian>(pertext)));
  world.add(std::make_shared<sphere>(point3(0.0f, 2.0f, 0.0f), 2.0f, std::make_shared<lambertian>(pertext)));

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 100;
  cam.max_depth = 50;
  // ray 와 충돌한 물체가 없을 경우 반환할 scene 배경색(solid color. no gradient) 정의
  cam.background = color(0.7f, 0.8f, 1.0f);

  // camera transform 관련 파라미터 설정
  cam.vfov = 20.0f;
  cam.lookfrom = point3(13.0f, 2.0f, 3.0f);
  cam.lookat = point3(0.0f, 0.0f, 0.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 0.0f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, hittable_list(world));
};

// quad scene 렌더링 함수
void quads(std::ofstream &output_file)
{
  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;

  /** 각 quad 객체에 적용할 재질(Material)을 shared_ptr로 생성 */
  auto left_red = std::make_shared<lambertian>(color(1.0f, 0.2f, 0.2f));
  auto back_green = std::make_shared<lambertian>(color(0.2f, 1.0f, 0.2f));
  auto right_blue = std::make_shared<lambertian>(color(0.2f, 0.2f, 1.0f));
  auto upper_orange = std::make_shared<lambertian>(color(1.0f, 0.5f, 0.0f));
  auto lower_teal = std::make_shared<lambertian>(color(0.2f, 0.8f, 0.8f));

  // 서로 다른 색상의 material 을 전달하여 5개의 quad 생성 후 world 에 추가
  world.add(std::make_shared<quad>(point3(-3.0f, -2.0f, 5.0f), vec3(0.0f, 0.0f, -4.0f), vec3(0.0f, 4.0f, 0.0f), left_red));
  world.add(std::make_shared<quad>(point3(-2.0f, -2.0f, 0.0f), vec3(4.0f, 0.0f, 0.0f), vec3(0.0f, 4.0f, 0.0f), back_green));
  world.add(std::make_shared<quad>(point3(3.0f, -2.0f, 1.0f), vec3(0.0f, 0.0f, 4.0f), vec3(0.0f, 4.0f, 0.0f), right_blue));
  world.add(std::make_shared<quad>(point3(-2.0f, 3.0f, 1.0f), vec3(4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 4.0f), upper_orange));
  world.add(std::make_shared<quad>(point3(-2.0f, -3.0f, 5.0f), vec3(4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -4.0f), lower_teal));

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 1.0f;
  cam.samples_per_pixel = 100;
  cam.max_depth = 50;
  // ray 와 충돌한 물체가 없을 경우 반환할 scene 배경색(solid color. no gradient) 정의
  cam.background = color(0.7f, 0.8f, 1.0f);

  // camera transform 관련 파라미터 설정
  cam.vfov = 80.0f;
  cam.lookfrom = point3(0.0f, 0.0f, 9.0f);
  cam.lookat = point3(0.0f, 0.0f, 0.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 0.0f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, world);
};

// light scene 렌더링 함수
void simple_light(std::ofstream &output_file)
{
  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;

  // perlin noise 적용을 위해 noise_texture 생성 후 lambertian material 에 적용
  auto pertext = std::make_shared<noise_texture>(4);
  // 반지름이 각각 2, 1000 인 두 sphere 추가
  world.add(std::make_shared<sphere>(point3(0.0f, -1000.0f, 0.0f), 1000.0f, std::make_shared<lambertian>(pertext)));
  world.add(std::make_shared<sphere>(point3(0.0f, 2.0f, 0.0f), 2.0f, std::make_shared<lambertian>(pertext)));

  /**
   * 광원 객체에 적용할 재질(diffuse_light)을 shared_ptr로 생성
   * -> 고강도 광원 색상값(color(4.0f, 4.0f, 4.0f) 사용 목적 관련 하단 필기 참고
   */
  auto difflight = std::make_shared<diffuse_light>(color(4.0f, 4.0f, 4.0f));

  // 광원으로 사용할 sphere 생성 후 world 에 추가
  world.add(std::make_shared<sphere>(point3(0.0f, 7.0f, 0.0f), 2.0f, difflight));
  // 광원으로 사용할 quad 생성 후 world 에 추가
  world.add(std::make_shared<quad>(point3(3.0f, 1.0f, -2.0f), vec3(2.0f, 0.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f), difflight));

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 100;
  cam.max_depth = 50;
  // ray 와 충돌한 물체가 없을 경우 반환할 scene 배경색(solid color. no gradient) 정의
  cam.background = color(0.0f, 0.0f, 0.0f);

  // camera transform 관련 파라미터 설정
  cam.vfov = 20.0f;
  cam.lookfrom = point3(26.0f, 3.0f, 6.0f);
  cam.lookat = point3(0.0f, 2.0f, 0.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 0.0f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, world);
};

int main(int argc, char *argv[])
{
  /** 명령줄 인수로 출력 파일(= .ppm 이미지 파일) 경로 전달받기 */
  // 기본 출력 파일 경로 지정
  std::string output_path = "output/image.ppm";
  if (argc > 1)
  {
    output_path = argv[1];
  }

  /** 전달받은 경로에 .ppm 이미지 파일 생성 및 열기 */
  // '파일 생성 및 쓰기' 해야 하므로 std::ofstream 사용 (기존 파일 읽기 시 std::ifstream)
  std::ofstream output_file(output_path);
  if (!output_file)
  {
    // 파일 생성 및 열기 실패 처리
    fprintf(stderr, "Error: could not open file %s for writing.\n", output_path.c_str());
    return 1;
  }

  // switch 문으로 렌더링을 원하는 장면 선택 가능
  switch (6)
  {
  case 1:
    bouncing_spheres(output_file);
    break;
  case 2:
    checkered_spheres(output_file);
    break;
  case 3:
    earth(output_file);
    break;
  case 4:
    perlin_sphere(output_file);
    break;
  case 5:
    quads(output_file);
    break;
  case 6:
    simple_light(output_file);
    break;
  }

  output_file.close();

  return 0;
}

/**
 * common header 를 가장 먼저 include 하는 이유
 *
 *
 * 수학 상수, utility 함수, color, ray, vec3 등 기본 헤더들이 정의된
 * 공통 헤더로써 rtweekend.hpp 를 main.cpp 최상단에 가장 먼저 include 해야 함.
 *
 * 그래야 다른 헤더 파일들에서 이미 rtweekend.hpp 가 이미 포함되어 있다고 가정하고
 * 그 안에 선언되어 있는 코드들을 가져다가 사용할 수 있음.
 *
 * 이렇게 하면 rtweekend.hpp 가 필요한 파일들에 중복 포함을 방지할 수 있음.
 */

/**
 * 전반사가 반영된 렌더링 결과 확인 방법
 *
 *
 * 구체의 기하학적 특성상 전반사를 관찰하기 어려움.
 *
 * 따라서, 주변 매질의 굴절률이 구체 매질의 굴절률보다 높은 상황
 * (ex> 물(굴절률 1.33) 속 공기 방울(굴절률 1.0))을 가정한다면,
 *
 * 구체에서도 전반사를 쉽게 관찰할 수 있도록 상대 굴절률을 조정함.
 */

/**
 * 속이 빈 유리 구슬(Hollow glass sphere) 생성 방법
 *
 *
 * 일정 두께를 갖는 속이 빈 유리 구슬을 분석하면 2개의 구체로 이루어져 있다는 것을 알 수 있음.
 *
 * 1. 외부 매질이 공기이고, 내부 매질이 유리(= 두께가 있는 유리)인 outer sphere
 * 2. 외부 매질이 유리(= 두께가 있는 유리)이고, 내부 매질이 공기(= 속이 비어있음)인 inner sphere
 *
 * 따라서, outer sphere 의 상대 굴절률은 1.5(유리 굴절률) / 1.0(진공 굴절률) = 1.5 로 설정하면 되고,
 * inner sphere 의 상대 굴절률은 그 반대인 1.0 / 1.5 = 0.67 로 설정하면 됨.
 *
 * 또한, 유리 구슬 두께를 고려하여 inner sphere 의 반지름을 outer sphere 반지름보다
 * 유리 구슬 두께만큼 작게 설정해야 할 것임.
 */

/**
 * 고강도 광원 색상값 (ex> color(4.0f, 4.0f, 4.0f))을 사용하는 이유
 *
 * ray tracing에서 조명을 시뮬레이션할 때, 광원의 색상 값은 단순한 색 표현을 넘어
 * 빛의 '강도(intensity)'를 의미합니다. 예를 들어 color(4.0f, 4.0f, 4.0f)는 흰색 광원이지만,
 * 일반적인 color(1.0f, 1.0f, 1.0f)보다 4배 더 밝은 빛을 방출합니다.
 *
 * 내부적으로 광선(ray)이 장면을 따라 반사되거나 산란될 때마다 감쇠(attenuation)가 발생합니다.
 * 이때, 광원의 출발 강도가 충분히 높다면 감쇠가 누적되더라도 여전히
 * 최종적으로 1.0에 가까운 값이 유지될 수 있습니다.
 *
 * 최종 이미지 버퍼에 픽셀 값을 기록할 때는 write_color() 함수에서 각 색상 성분이
 * [0.0, 1.0] 범위로 clamp되므로, 계산 결과가 1.0보다 크더라도 출력은 1.0에 맞춰집니다.
 * 하지만 **1.0에 가까운 값이 많을수록 더 밝게 보이며**, 충분히 강한 광원 색상은
 * 간접 조명이나 다중 반사를 거친 후에도 시각적으로 더 밝은 결과를 만들어냅니다.
 *
 * 따라서, 장면을 실제로 밝게 비추기 위해서는 광원 색상을 (4,4,4)와 같이
 * 고강도로 설정하는 것이 필요합니다.
 *
 * -> 이 개념은 HDR 렌더링(High Dynamic Range Rendering)의 핵심 아이디어 중 하나
 * https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/HDR/MyShaders/hdr.fs
 * https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/HDR/hdr.cpp
 */
