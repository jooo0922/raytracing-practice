#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../hittable/hittable.hpp"

class camera
{
public:
  double aspect_ratio = 1.0f; // .ppm 이미지 종횡비 (기본값 1:1)
  int image_width = 100;      // .ppm 이미지 너비 (기본값 100. 이미지 높이는 너비에 aspect_ratio 를 곱해서 계산.)
  int samples_per_pixel = 10; // antialiasing 을 위해 사용할 각 pixel 주변 random sample 개수

public:
  // pixel 들을 순회하며 출력 스트림(std::ofstream or std::ostream)에 데이터 출력(= .ppm 이미지 렌더링)
  void render(std::ostream &output_stream, const hittable &world)
  {
    // 카메라 및 viewport 파라미터 초기화
    initialize();

    /** 생성된 .ppm 이미지 파일에 데이터 출력 */
    // .ppm metadata 출력 (https://raytracing.github.io/books/RayTracingInOneWeekend.html > Figure 1 참고)
    output_stream << "P3\n"
                  << image_width << ' ' << image_height << "\n255\n";

    /** viewport 각 pixel 들을 통과하는 ray 를 순회하며 world 에 casting 했을 때 계산된 최종 색상값을 .ppm 에 출력 */
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
        // 현재 pixel 주변 random sample 을 통과하는 ray 로부터 계산된 색상값들을 누산할 변수 초기화
        color pixel_color(0.0f, 0.0f, 0.0f);

        // random sample 개수만큼 반복문을 돌려서 색상값 누산
        for (int sample = 0; sample < samples_per_pixel; sample++)
        {
          // 카메라 ~ 각 pixel 주변 random sample 까지 향하는 random ray(반직선) 생성
          ray r = get_ray(i, j);

          // 현재 pixel 주변 random sample 을 통과하는 ray 로부터 얻어진 색상값 누산
          pixel_color += ray_color(r, world);
        }

        // 누산된 색상값에 미소 변화량을 곱해(= random sample 개수만큼 평균을 내서) 최종 색상 계산 후 .ppm 파일에 쓰기 -> antialiasing 이 적용된 색상값 출력 가능
        write_color(output_stream, pixel_samples_scale * pixel_color);
      }
    }

    // .ppm 에 색상값을 다 쓰고나면 완료 메시지 출력
    printf("\rDone.                       \n");
    fflush(stdout);
  };

private:
  // 카메라 및 viewport 파라미터 초기화
  void initialize()
  {
    /** .ppm 이미지 종횡비(aspect_ration) 및 너비를 기반으로 해상도(rows, columns) 정의 */
    image_height = static_cast<int>(image_width / aspect_ratio); // 이미지 높이는 정수형이므로, 너비와 종횡비를 곱한 실수값을 정수형으로 casting.
    image_height = (image_height < 1) ? 1 : image_height;        // 이미지 높이는 항상 1보다는 크도록 함.

    // 색상값 적분 시 사용할 미소 변화량 계산 -> 쉽게 표현하면 누산된 색상값 총합해서 random sample 개수만큼 평균을 내는 것.
    pixel_samples_scale = 1.0f / samples_per_pixel;

    /** 카메라 및 viewport 파라미터 정의 */
    auto focal_length = 1.0;                                                                   // 카메라 중점(eye point)과 viewport 사이의 거리 (현재는 단위 거리 1로 지정함.)
    auto viewport_height = 2.0;                                                                // viewport 높이 정의
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height); // viewport 너비 정의 (기존 aspect_ratio 는 casting 에 의해 소수점이 잘려나간 image_width & image_height 의 종횡비와 다르므로, 실제 image_width & image_height 로 종횡비 재계산).
    camera_center = point3(0, 0, 0);                                                           // 3D Scene 상에서 카메라 중점(eye point) > viewport 로 casting 되는 모든 ray 의 출발점이기도 함.

    /**
     * viewport 구조에 존재하는 벡터 및 정점들 정의
     * (https://raytracing.github.io/books/RayTracingInOneWeekend.html > Figure 4 참고)
     */
    auto viewport_u = vec3(viewport_width, 0, 0);   // viewport 왼쪽 끝에서 오른쪽 끝으로 향하는 수평 방향 벡터
    auto viewport_v = vec3(0, -viewport_height, 0); // viewport 위쪽 끝에서 아래쪽 끝으로 향하는 수직 방향 벡터

    pixel_delta_u = viewport_u / image_width;  // pixel grid 의 각 픽셀 사이의 수평 방향 간격
    pixel_delta_v = viewport_v / image_height; // pixel grid 의 각 픽셀 사이의 수직 방향 간격

    // 뷰포트의 좌상단 꼭지점의 '3D 공간 상의' 좌표 계산 (이미지 좌표 아님 주의!) (Figure 4 에서 Q 로 표시)
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;

    // 'pixel grid'의 좌상단 픽셀(이미지 좌표 상으로 (0,0)에 해당하는 픽셀)의 '3D 공간 상의' 좌표 계산 (Figure 4 에서 P(0,0) 으로 표시)
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
  };

  // 카메라 ~ 각 pixel 주변 random sample 까지 향하는 random ray(반직선) 생성 함수 (viewport 상 현재 pixel row(= i), column(= j) 값을 매개변수로 받아서 위치값 계산)
  ray get_ray(int i, int j) const
  {
    /** viewport 각 pixel 을 중심으로 단위 사각형(1*1 size) 범위 내에 존재하는 random sample 계산 */

    // pixel 중점으로부터 띄워줄 단위 사각형 범위 내의 random offset 계산
    auto offset = sample_square();

    // pixel 중점에서 random offset 만큼 변위된 위치값으로 random sample 계산
    auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

    // 카메라 ~ 각 pixel 주변 random sample 방향벡터 계산
    auto ray_origin = camera_center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
  };

  // (-0.5f, -0.5f) ~ (0.5f, 0.5f) 범위 내의 단위 사각형(1*1 size) 안에 존재하는 random point 반환 함수
  vec3 sample_square() const
  {
    return vec3(random_double() - 0.5f, random_double() - 0.5f, 0.0f);
  };

  // 주어진 반직선(ray)을 world 에 casting 하여 계산된 최종 색상값을 반환하는 함수
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
  };

private:
  // 카메라 및 viewport 파라미터 멤버변수 정의
  int image_height;           // .ppm 이미지 높이
  double pixel_samples_scale; // 각 pixel 주변 random sample 을 통과하는 ray 로부터 계산된 색상 적분에 사용할 미소 변화량(속칭 dx) -> used for antialiasing
  point3 camera_center;       // 3D Scene 상에서 카메라 중점(eye point). viewport 로 casting 되는 모든 ray 의 출발점
  point3 pixel00_loc;         // 'pixel grid'의 좌상단 픽셀(이미지 좌표 상으로 (0,0)에 해당하는 픽셀)의 '3D Scene 상의' 좌표값 (Figure 4 에서 P(0,0) 으로 표시)
  vec3 pixel_delta_u;         // pixel grid 의 각 픽셀 사이의 수평 방향 간격
  vec3 pixel_delta_v;         // pixel grid 의 각 픽셀 사이의 수직 방향 간격
};

#endif /* CAMERA_HPP */
