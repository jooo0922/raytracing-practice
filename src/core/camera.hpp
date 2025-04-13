#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../hittable/hittable.hpp"
#include "../core/material.hpp"

class camera
{
public:
  double aspect_ratio = 1.0f; // .ppm 이미지 종횡비 (기본값 1:1)
  int image_width = 100;      // .ppm 이미지 너비 (기본값 100. 이미지 높이는 너비에 aspect_ratio 를 곱해서 계산.)
  int samples_per_pixel = 10; // antialiasing 을 위해 사용할 각 pixel 주변 random sample 개수
  int max_depth = 10;         // ray bouncing 최대 횟수 (= 각 ray 마다 최대 재귀 순회 깊이 제한)

  double vfov = 90.0f;                        // camera frustum 의 수직 방향 fov(field of view) 각도
  point3 lookfrom = point3(0.0f, 0.0f, 0.0f); // 카메라 위치(카메라 좌표계 기준 원점)
  point3 lookat = point3(0.0f, 0.0f, -1.0f);  // 카메라가 바라보는 지점
  vec3 vup = vec3(0.0f, 1.0f, 0.0f);          // 카메라 기준 위쪽 방향을 정의하는 기준 벡터 (하단 필기 참고)

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
          pixel_color += ray_color(r, max_depth, world);
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
    /**
     * fov 각도의 tan 비 기반으로 viewport 크기를 계산하므로,
     * fov 각도에 따라 카메라가 담을 수 있는 시야각이 달라짐.
     *
     * -> fov 가 커질수록 카메라가 담을 수 있는 시야각이 넓어지므로,
     * 마치 zoom-out 되는 듯한 효과를 줄 수 있음.
     */
    auto focal_length = 1.0;                                                                   // 카메라 중점(eye point)과 viewport 사이의 거리 (현재는 단위 거리 1로 지정함.)
    auto theta = degrees_to_radians(vfov);                                                     // 카메라 수직 방향 fov 각도 단위를 degree -> radian 으로 변환
    auto h = std::tan(theta / 2);                                                              // 카메라 수직 방향 fov 절반 지점 방향을 밑변으로 하는 직각삼각형의 tan 비 계산
    auto viewport_height = 2 * h * focal_length;                                               // viewport 높이 정의 (직각삼각형의 밑변의 길이 * tan 비 = 직각삼각형의 높이 계산(= viewport 높이 절반) -> 여기에 2배를 곱해서 viewport 최종 높이 계산)
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
  color ray_color(const ray &r, int depth, const hittable &world)
  {
    // ray 가 최대 재귀 순회 깊이(= max_depth)만큼 진행되었다면 재귀 순회 종료 (하단 필기 참고)
    if (depth <= 0)
    {
      return color(0.0f, 0.0f, 0.0f);
    }

    // world 에 추가된 hittable objects 들을 순회하며 현재 ray 와 교차 검사 수행
    hit_record rec;

    // ray 충돌 범위가 t = 0.001 이하일 경우, 불필요한 조명 감쇄를 일으키는 충돌로 판정하여 무시함. (하단 필기 참고)
    if (world.hit(r, interval(0.001, infinity), rec))
    {
      // 하나라도 충돌한 hittable object 가 존재한다면, rec 변수에는 현재 ray 방향에서 카메라로부터 가장 가까운 교차점의 충돌 정보가 기록됨.
      // 충돌 지점 p 의 normal vector 중심의 반구 영역 내 랜덤 방향벡터 생성
      // vec3 direction = random_on_hemisphere(rec.normal);

      // ray 충돌 지점 object 의 산란 동작이 재정의된 material::scatter(...) 인터페이스를 호출하여 산란할 ray(= scattered)과 감쇄(= attenuation) 계산
      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered))
      {
        // (일정 확률로)산란할 ray 생성 성공 시 처리
        // ray 를 산란하여 recursive 하게 진행했을 때 반사된 빛(색상)에 감쇄(= attenuation)를 적용하여 반환함.
        return attenuation * ray_color(scattered, depth - 1, world);
      }
      // (일정 확률로)산란할 ray 생성 실패(= 현재 충돌한 ray 가 완전히 흡수되었다고 가정) 시 처리
      // 기여도가 없는 영벡터 색상(검은색) 반환
      return color(0.0f, 0.0f, 0.0f);
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
  vec3 u, v, w;               // 카메라의 로컬 좌표계(= 뷰 좌표계)의 직교 정규 기저 벡터(orthonormal basis) -> 즉, 카메라의 orientation(회전)을 정의하기 위한 카메라 좌표계의 세 로컬 축!
};

/**
 * ray tracing 의 가장 기본 원리!
 *
 *
 * camera::ray_color() 함수처럼
 * ray 충돌 지점의 normal vector 를 중심으로 하는 반구 영역 내 랜덤 방향벡터를
 * ray 의 다음 진행 방향으로 결정함.
 *
 * 해당 진행 방향으로 recursive 하게 ray 를 쐈을 때,
 * 반사되어 돌아오는 색상값에 대해 여러 material property 및 렌더링 방정식에 따라
 * 다시 반사시킬 조명값을 계산함.
 *
 * 이렇게 계산된 빛을 현재 자신의 재귀순회 호출자(= caller. 재귀 부모)에게
 * 다시 반환함으로써, 각 카메라 pixel 로 들어오는 최종 조명값을 계산하는 것임.
 *
 * -> ray tracing 알고리즘의 핵심은 광선(ray)의 진행 방향을
 * 따라 재귀적으로(scene traversal) 최종 조명값 계산하는 것!
 */

/**
 * max_depth
 *
 *
 * ray 가 scene 객체들과 끊임없이 충돌하다 보면 렌더링 성능이 급격히 저하되고,
 * 재귀 호출이 과도하게 깊어져서 재귀 스택이 가득차서 overflow 가 발생할 수 있음.
 *
 * 이를 방지하기 위해, ray 가 최대 재귀 순회 깊이(= max_depth)만큼 진행되었음에도
 * 계속 scene 객체들과 충돌한다면, 해당 pixel 지점 최종 색상값에
 * 기여도가 없는 영벡터 색상을 반환하고 재귀를 종료함.
 *
 * (-> 이렇게 반환된 영벡터들이 나중에 ray tracing 결과물 noise 에 영향을 줌.)
 *
 * 그래서 ray_color() 를 재귀 호출할 때마다 현재 depth 값을 추적할 수 있도록
 * 두 번째 인자에 현재 재귀 depth - 1 만큼 decrement 해서 전달함.
 */

/**
 * Shadow Acne 현상 해결
 *
 *
 * ray 충돌 범위가 t = 0.001 이하일 경우,
 * 부정확한 부동소수점 반올림(floating point rounding errors)으로 인해
 * 잘못 계산된 교차점이 충돌한 표면 살짝 아래로 계산된 것으로 판정함.
 *
 * 왜냐하면, 충돌한 표면 살짝 아래에서 새로운 ray 를 쏘면, t = 0.0000000001 처럼
 * 해당 ray 출발점과 아주 가까운 지점에서 동일한 표면 자기 자신과 중복해서 부딪히기 때문.
 *
 * 이러한 잘못된 충돌들이 하나씩 쌓일 때마다,
 * 불필요한 조명 감쇄(0.5f *)를 발생시켜 밝은 영역을 더 어둡게 만듦
 *
 * 이는 그림자가 없어야 할 영역에 어두운 그림자를 만들어낸다는 점에서
 * 기존 shadow acne 현상과 유사한 맥락이라고 볼 수 있음.
 *
 * 이를 해결하기 위해, 유효한 ray 충돌 범위를 0.001 이상으로 보고,
 * 그 이하는 표면 아래에서 충돌한 잘못된 ray 충돌로 판정하여 조명 감쇄를 발생시키지 않는 것임
 */

/**
 * vup (= view up)
 *
 *
 * 예제 코드에 등장하는 두 벡터 vup 과 v 의 정의와 역할은 다음과 같이 구분 가능함.
 *
 * 1. vup
 * - "카메라의 세계 기준 위쪽이 어디인가?" 를 알려주는 기준(reference) 벡터 (≠ 실제 카메라의 up 벡터)
 * - '카메라 기준 머리 방향이 어디냐'를 결정해주는 방향 벡터
 * - 카메라를 ‘기울일지 말지’를 결정하는 정보
 *   (ex > lookfrom = (0, 0, 0), lookat = (0, 0, -1), vup = (0, 1, 0) 이라면, 카메라는 정면(-Z) 를 바라보며 수평을 유지한 상태이나,
 *    vup = (1, 0, 0) 이면, 카메라는 여전히 같은 곳을 바라보지만, 90도 옆으로 기울어진 상태가 됨. -> 카메라의 세계 기준 위쪽을 Positive X 방향으로 결정했으니까!)
 * - 아무 벡터든 view direction(lookfrom -> lookat)과 평행하지만 않으면 vup 으로 정의할 수 있음.
 *   -> 단지 (0, 1, 0) 이 대부분의 world 좌표계가 y-up 기준이라서 편하고 안전한 기본값으로 사용할 뿐.
 *
 * 2. v
 * - u, w 와 함께 형성되는 카메라의 로컬 좌표계(= 뷰 좌표계)의 직교 기저 벡터(u, v, w) 중 위쪽(up) 방향을 가리키는 로컬 축
 * - 즉, 카메라 기저 벡터 중 위쪽을 가리키는 y축 역할에 해당하는 벡터
 * - 이를 구하려면 view direction 에 직교하는 평면에 vup 을 투영('Project this up vector onto the plane orthogonal to the view direction ...')
 *   해서 구해야 하지만, 실제 코드 상에서는 투영과 동등한 효과를 cross() 연산으로 구현함.
 */

#endif /* CAMERA_HPP */
