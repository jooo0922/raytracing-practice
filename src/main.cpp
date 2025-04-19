#include "common/rtweekend.hpp" // common header 최상단에 가장 먼저 include (관련 필기 하단 참고)
#include "core/camera.hpp"
#include "core/material.hpp"
#include "hittable/hittable.hpp"
#include "hittable/hittable_list.hpp"
#include "hittable/sphere.hpp"

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

  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;

  /** 각 Hittable 객체에 적용할 재질(Material)을 shared_ptr로 생성하여 공유 가능하도록 관리 */
  auto ground_material = std::make_shared<lambertian>(color(0.8f, 0.8f, 0.0f));
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
          auto albedo = color::random() * color::random();
          sphere_material = std::make_shared<lambertian>(albedo);
          world.add(std::make_shared<sphere>(center, 0.2f, sphere_material));
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

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 10;
  cam.max_depth = 20;

  // camera transform 관련 파라미터 설정
  cam.vfov = 20.0f;
  cam.lookfrom = point3(-2.0f, 2.0f, 1.0f);
  cam.lookat = point3(0.0f, 0.0f, -1.0f);
  cam.vup = vec3(0.0f, 1.0f, 0.0f);

  // defocus blur 관련 파라미터 성정
  cam.defocus_angle = 10.0f;
  cam.focus_dist = 3.4f;

  // 카메라 및 viewport 파라미터 내부에서 자동 초기화 후 .ppm 이미지 렌더링
  cam.render(output_file, world);

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
