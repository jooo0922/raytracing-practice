#include "common/rtweekend.hpp" // common header 최상단에 가장 먼저 include (관련 필기 하단 참고)
#include "core/camera.hpp"
#include "hittable/hittable.hpp"
#include "hittable/hittable_list.hpp"
#include "hittable/sphere.hpp"

// 주어진 반직선(ray)에 대한 특정 색상을 반환하는 함수
color ray_color(const ray &r, const hittable &world)
{
  // world 에 추가된 hittable objects 들을 순회하며 현재 ray 와 교차 검사 수행
  hit_record rec;
  if (world.hit(r, interval(0, infinity), rec))
  {
    // 하나라도 충돌한 hittable object 가 존재한다면, rec 변수에는 현재 ray 방향에서 카메라로부터 가장 가까운 교차점의 충돌 정보가 기록됨.
    // -> 카메라에서 가장 가까운 교차점의 노멀벡터([-1.0, 1.0] 범위)를 [0.0, 1.0] 범위의 색상값으로 맵핑
    return 0.5f * (rec.normal + color(1.0f, 1.0f, 1.0f));
  }

  // 반직선을 길이가 1인 단위 벡터로 정규화
  vec3 unit_direction = unit_vector(r.direction());
  // [-1.0, 1.0] 범위로 정규화된 단위 벡터의 y 값을 [0.0, 1.0] 범위로 맵핑
  auto a = 0.5f * (unit_direction.y() + 1.0f);

  // 흰색과 파란색을 [0.0, 1.0] 범위의 a값에 따라 혼합(선형보간)하여 .ppm 에 출력할 색상 계산
  return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

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

  /** .ppm 이미지 파일의 종횡비(aspect_ration) 및 해상도(rows, columns) 정의 */
  auto aspect_ratio = 16.0 / 9.0;                                  // 이미지의 종횡비를 16:9 로 설정
  int image_width = 400;                                           // 이미지 너비는 상수이며, 이미지 너비 값에 따라 이미지 높이 값이 종횡비와 곱해져서 계산됨.
  int image_height = static_cast<int>(image_width / aspect_ratio); // 이미지 높이는 정수형이므로, 너비와 종횡비를 곱한 실수값을 정수형으로 형변환함.
  image_height = (image_height < 1) ? 1 : image_height;            // 이미지 너비는 항상 1보다는 크도록 함.

  /** world(scene) 역할을 수행하는 hittable_list 생성 및 hittable object 추가 */
  hittable_list world;
  world.add(std::make_shared<sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f));      // 반지름이 0.5 인 sphere 추가
  world.add(std::make_shared<sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f)); // 반지름이 100 인 sphere 추가

  /** 카메라 및 viewport 파라미터 정의 */
  auto focal_length = 1.0;                                                                   // 카메라 중점(eye point)과 viewport 사이의 거리 (현재는 단위 거리 1로 지정함.)
  auto viewport_height = 2.0;                                                                // viewport 높이 정의
  auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height); // viewport 너비 정의 (aspect_ratio 는 실제 이미지 사이즈의 종횡비와 달라, 실제 이미지 크기로부터 종횡비를 다시 계산해서 적용).
  auto camera_center = point3(0, 0, 0);                                                      // 3D Scene 상에서 카메라 중점(eye point) > viewport 로 casting 되는 모든 ray 의 출발점이기도 함.

  /**
   * viewport 구조에 존재하는 벡터 및 정점들 정의
   * (https://raytracing.github.io/books/RayTracingInOneWeekend.html > Figure 4 참고)
   */
  auto viewport_u = vec3(viewport_width, 0, 0);   // viewport 왼쪽 끝에서 오른쪽 끝으로 향하는 수평 방향 벡터
  auto viewport_v = vec3(0, -viewport_height, 0); // viewport 위쪽 끝에서 아래쪽 끝으로 향하는 수직 방향 벡터

  auto pixel_delta_u = viewport_u / image_width;  // pixel grid 의 각 픽셀 사이의 수평 방향 간격
  auto pixel_delta_v = viewport_v / image_height; // pixel grid 의 각 픽셀 사이의 수직 방향 간격

  // 뷰포트의 좌상단 꼭지점의 '3D 공간 상의' 좌표 계산 (이미지 좌표 아님 주의!) (Figure 4 에서 Q 로 표시)
  auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;

  // 'pixel grid'의 좌상단 픽셀(이미지 좌표 상으로 (0,0)에 해당하는 픽셀)의 '3D 공간 상의' 좌표 계산 (Figure 4 에서 P(0,0) 으로 표시)
  auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  /** 생성된 .ppm 이미지 파일에 데이터 출력 */
  // .ppm metadata 출력 (https://raytracing.github.io/books/RayTracingInOneWeekend.html > Figure 1 참고)
  output_file << "P3\n"
              << image_width << ' ' << image_height << "\n255\n";

  /** viewport 각 픽셀들을 순회하며 각 픽셀 지점을 통과하는 ray 로부터 계산된 색상값을 .ppm 에 출력 */
  for (int j = 0; j < image_height; ++j)
  {
    /**
     * .ppm 에 출력할 색상값을 한 줄(row)씩 처리할 때마다 남아있는 줄 수 콘솔 출력
     * (참고로, fflush(stdout) 는 출력 스트림(stdout)의 버퍼를 비움.)
     * -> <iostream> 은 버퍼링으로 인해 덮어쓰기가 정상 작동하지 않아 cstdio 함수를 사용하여 출력함.
     */
    printf("\rScanlines remaining: %d ", image_height - j);
    fflush(stdout);
    for (int i = 0; i < image_width; ++i)
    {
      // viewport 각 픽셀 중점의 '3D 공간 상의' 좌표 계산
      auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);

      // 카메라 중점 ~ viewport 각 픽셀 중점까지 향하는 방향벡터 계산
      auto ray_direction = pixel_center - camera_center;

      // 카메라 ~ viewport 각 픽셀 중점까지 향하는 반직선(ray) 생성
      ray r(camera_center, ray_direction);

      // 주어진 반직선(ray) r 에 대해 intersection 검사를 수행한 결과 현재 pixel 에 출력할 색상 계산 후 .ppm 파일에 쓰기
      auto pixel_color = ray_color(r, world);
      write_color(output_file, pixel_color);
    }
  }

  // .ppm 에 색상값을 다 쓰고나면 완료 메시지 출력 및 파일 닫기
  printf("\rDone.                       \n");
  fflush(stdout);
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
