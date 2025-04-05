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

  /** 각 Hittable 객체에 적용할 재질(Material)을 shared_ptr로 생성하여 공유 가능하도록 관리 */
  auto material_ground = std::make_shared<lambertian>(color(0.8f, 0.8f, 0.0f));
  auto material_center = std::make_shared<lambertian>(color(0.1f, 0.2f, 0.5f));
  /**
   * 구체의 기하학적 특성상 전반사를 관찰하기 어려움.
   * 따라서, 주변 매질의 굴절률이 구체 매질의 굴절률보다 높은 상황(ex> 물(굴절률 1.33) 속 공기 방울(굴절률 1.0))을 가정하여
   * 구체에서도 전반사를 쉽게 관찰할 수 있도록 상대 굴절률을 조정함.
   */
  auto material_left = std::make_shared<dielectric>(1.0f / 1.33f);
  auto material_right = std::make_shared<metal>(color(0.8f, 0.6f, 0.2f), 1.0f);

  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;
  world.add(std::make_shared<sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f, material_ground)); // 반지름이 100 인 sphere 추가
  world.add(std::make_shared<sphere>(point3(0.0f, 0.0f, -1.2f), 0.5f, material_center));      // 반지름이 0.5 인 sphere 추가
  world.add(std::make_shared<sphere>(point3(-1.0f, 0.0f, -1.0f), 0.5f, material_left));
  world.add(std::make_shared<sphere>(point3(1.0f, 0.0f, -1.0f), 0.5f, material_right));

  /** camera 객체 생성 및 이미지 렌더링 수행 */
  camera cam;

  // 주요 이미지 파라미터 설정
  cam.image_width = 400;
  cam.aspect_ratio = 16.0f / 9.0f;
  cam.samples_per_pixel = 10;
  cam.max_depth = 20;

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
