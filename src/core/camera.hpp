#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../hittable/hittable.hpp"

class camera
{
public:
  double aspect_ratio = 1.0f; // .ppm 이미지 종횡비 (기본값 1:1)
  int image_width = 100;      // .ppm 이미지 너비 (기본값 100. 이미지 높이는 너비에 aspect_ratio 를 곱해서 계산.)
  int samples_per_pixel = 10; // antialiasing 을 위해 사용할 각 pixel 주변 random sample 개수
  int max_depth = 10;         // ray bouncing 최대 횟수 (= 각 ray 마다 최대 재귀 순회 깊이 제한)

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

      // Lambertian distribution 에 더 부합하는 ray 방향벡터 계산 (하단 필기 참고)
      vec3 direction = rec.normal + random_unit_vector();
      // 랜덤 방향벡터로 ray 를 recursive 하게 진행시켰을 때 반사된 빛(색상)을 반환받아 처리함.
      // -> 아직 material 인터페이스 구현 전이므로, 들어온 빛의 50%(= 0.5) 만 반사하고 나머지는 흡수하는 기초적인 diffuse 연산 처리
      return 0.5f * ray_color(ray(rec.p, direction), depth - 1, world);
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

#endif /* CAMERA_HPP */
