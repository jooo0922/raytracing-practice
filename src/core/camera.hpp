#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../hittable/hittable.hpp"
#include "../core/material.hpp"

/**
 * camera 동작 원리를 추상화한 클래스 (defocus blur 관련 하단 필기 참고)
 */
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

  double defocus_angle = 0.0f; // 조리개 개방 각 -> 이 각도가 커질수록 조리개 반경이 커져서 더 많은 빛이 들어옴 -> defocus blur 강도가 커짐.
  double focus_dist = 10.0f;   // camera lens 에서 focus plane(= viewport or pixel grid) 까지의 거리

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
    /**
     * 카메라 중점에서 viewport 까지의 거리를 focus_dist(= 카메라 렌즈에서 focus plane 까지의 거리)와 맞추기로 했으므로,
     * focal_length -> focus_dist 로 교체함.
     */
    auto theta = degrees_to_radians(vfov);                                                     // 카메라 수직 방향 fov 각도 단위를 degree -> radian 으로 변환
    auto h = std::tan(theta / 2);                                                              // 카메라 수직 방향 fov 절반 지점 방향을 밑변으로 하는 직각삼각형의 tan 비 계산
    auto viewport_height = 2 * h * focus_dist;                                                 // viewport 높이 정의 (직각삼각형의 밑변의 길이 * tan 비 = 직각삼각형의 높이 계산(= viewport 높이 절반) -> 여기에 2배를 곱해서 viewport 최종 높이 계산)
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height); // viewport 너비 정의 (기존 aspect_ratio 는 casting 에 의해 소수점이 잘려나간 image_width & image_height 의 종횡비와 다르므로, 실제 image_width & image_height 로 종횡비 재계산).
    camera_center = lookfrom;                                                                  // 3D Scene 상에서 카메라 중점(eye point) -> 카메라 위치(= view direction 출발점)으로 정의

    /**
     * 카메라의 orientation(회전)을 정의하기 위해
     * 카메라의 로컬 좌표계(= 뷰 좌표계)의 3개의 로컬 축(= 직교 정규 기저 벡터(orthonormal basis)) 계산
     * (https://raytracing.github.io/books/RayTracingInOneWeekend.html > Figure 20 참고)
     * (원래 회전 변환은 표준기저벡터에 회전변환이 적용된 세 로컬 축을 사용해서 정의됨. -> 게임수학 p.339 > 그림 10-5 참고)
     */
    w = unit_vector(lookfrom - lookat); // 카메라 view direction 반대 방향 기저 축(이 튜토리얼에서 오른손 좌표계를 사용하므로, +Z 축이 카메라 뒷쪽을 바라봄.)
    u = unit_vector(cross(vup, w));     // 카메라 오른쪽 방향 기저 축
    v = cross(w, u);                    // 카메라 위쪽 방향 기저 축

    /**
     * viewport 구조에 존재하는 벡터 및 정점들 정의
     * (https://raytracing.github.io/books/RayTracingInOneWeekend.html > Figure 4 참고)
     */
    auto viewport_u = viewport_width * u;   // viewport 왼쪽 끝에서 오른쪽 끝으로 향하는 수평 방향 벡터 -> 회전된 카메라 오른쪽 방향 기저 축을 따라 viewport 수평 벡터도 회전
    auto viewport_v = viewport_height * -v; // viewport 위쪽 끝에서 아래쪽 끝으로 향하는 수직 방향 벡터 -> 회전된 카메라 위쪽 방향 기저 축을 따라 viewport 수직 벡터도 회전

    pixel_delta_u = viewport_u / image_width;  // pixel grid 의 각 픽셀 사이의 수평 방향 간격
    pixel_delta_v = viewport_v / image_height; // pixel grid 의 각 픽셀 사이의 수직 방향 간격

    // 뷰포트의 좌상단 꼭지점의 '3D 공간 상의' 좌표 계산 (이미지 좌표 아님 주의!) (Figure 4 에서 Q 로 표시)
    // (카메라 중점(= camera_center) 에서 view direction(= -w) 방향으로 focal_length 길이만큼 떨어진 지점의 좌표를 viewport 중점으로 계산)
    auto viewport_upper_left = camera_center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;

    // 'pixel grid'의 좌상단 픽셀(이미지 좌표 상으로 (0,0)에 해당하는 픽셀)의 '3D 공간 상의' 좌표 계산 (Figure 4 에서 P(0,0) 으로 표시)
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // 조리개 개방 각을 기반으로 조리개 반경 계산 (하단 필기 참고)
    auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle) / 2.0f);

    // 조리개에 의해 개방된 반경만큼의 lens disk 로컬 기저벡터 계산 (하단 필기 참고)
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
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

  // defocus lens 상 랜덤한 ray 출발점 반환 함수
  point3 defocus_disk_sample() const
  {
    // 표준기저벡터로 이루어진 좌표계 상 단위 원 내의 랜덤 점 반환
    auto p = random_in_unit_disk();
    // 단위 원 상의 랜덤 점 -> defocus disk 상의 랜덤 점으로 변환
    return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
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
  vec3 defocus_disk_u;        // 조리개 원판 영역(defocus_disk)의 수평 방향 반지름 벡터 (카메라 right 방향으로 회전 및 조리개 반경만큼 스케일한 수평 방향 기저벡터)
  vec3 defocus_disk_v;        // 조리개 원판 영역(defocus_disk)의 수직 방향 반지름 벡터 (카메라 up 방향으로 회전 및 조리개 반경만큼 스케일한 수직 방향 기저벡터)
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

/**
 * Defocus Blur (Depth of Field)
 *
 * 기존 카메라 클래스는 모든 ray를 동일한 카메라 중심(camera::camera_center)에서 발사하므로,
 * 실제로는 film 상에 모든 상이 정확히 한 점으로 맺히는 pinhole camera 모델과 같다.
 * 이 경우, 초점 심도(depth of field)가 무한대이며, 모든 오브젝트가 선명하게 보이게 된다.
 *
 * 반면, 현실의 카메라는 렌즈와 조리개(aperture)를 사용하여 film과 scene 사이에 빛을 굴절시키고,
 * 이로 인해 특정 거리의 오브젝트만 film 상의 한 점에 선명하게 맺히며, 나머지 오브젝트는 흐릿하게 맺히는
 * defocus blur(= depth of field) 현상이 발생한다.
 *
 * Defocus blur를 구현하려면, 카메라 모델을 pinhole이 아닌
 * '렌즈 + 조리개' 구조로 추상화해야 한다.
 * 단, 실제 광학적 굴절이나 필름 상 맺힘, 상하 반전 등의 내부 과정은
 * 렌더링에는 불필요하므로 생략하고, 이를 단순화한 모델이 바로 'Thin Lens Approximation'이다.
 *
 * Thin Lens Approximation에서는 카메라 렌즈를 무한히 얇은 원판으로 모델링하며,
 * 이 렌즈 위에서 무작위로 선택된 출발점들로부터 viewport(pixel grid) 상의 pixel을 향해 ray를 발사한다.
 *
 * 이때, 렌즈로부터 특정 거리만큼 떨어진 평면을 'focus plane',
 * 그 거리를 'focus distance'라고 한다.
 *
 * 튜토리얼에서는 이 focus plane과 동일한 위치에 viewport를 두고,
 * focus plane 위의 픽셀로 향하는 ray를 각기 다른 렌즈 위의 출발점에서 발사함으로써
 * defocus blur를 구현한다.
 *
 * 이 구조에서는 focus plane 위에 위치한 오브젝트는
 * 여러 출발점에서 날아온 ray들이 동일한 위치에서 충돌하므로, 동일한 색을 반환하게 되어 선명하게 보인다.
 *
 * 반면, focus plane에서 멀리 떨어진 오브젝트는
 * 출발점에 따라 충돌 위치가 달라지고, 각기 다른 색을 반환하게 되어
 * 해당 픽셀에는 다양한 색이 혼합되며 blur처럼 보이게 된다.
 *
 * 또한, 조리개 반경(lens radius)이 커질수록 defocus blur는 더욱 강하게 나타나며,
 * 이 조리개 개방 각은 카메라 파라미터(defocus_angle)로 제어할 수 있다.
 */

/**
 * defocus_angle 에 따라 일정한 blur 강도를 유지하는 조리개 반경 계산 방법
 *
 *
 * defocus blur 강도를 제어하는 조리개 개방 각도(defocus_angle)를 기반으로,
 * focus plane 중심에서 바라본 cone의 반각을 이용해 defocus disk의 반지름을 계산한다.
 *
 * 즉, viewport center 가 꼭지점이고, defocus disk 가 밑면을 이루는 cone 에서
 * 꼭지점 부분의 각도를 defocus_angle(조리개 개방 각)으로 정의한다.
 *
 * 이때, cone 개방 각의 절반 지점에 해당하는 직각삼각형에 대한 tan 비를
 * focus_dist(= 직각삼각형의 밑변) 와 곱해서 defocus_radius(= 직각삼각형의 높이)를 구한다.
 *
 * 이렇게 하면 항상 직각삼각형이 일정한 각도와 비율을 유지하게 됨으로써,
 * focus_dist 파라미터가 사용자에 의해 변경되더라도,
 * defocus_angle 이 일정하면 blur 또한 일정하게 유지될 수 있음.
 *
 * 왜냐하면, 카메라가 focus_plane 에 가까워질수록 blur 의 강도가 강해져도,
 * defocus_angle 을 일정하게 유지하기 위해 그만큼 defocus_radius 또한 줄어들기 때문에
 * blur 정도가 일정해지는 것!
 */

/**
 * lens disk 상의 로컬 기저벡터(= x축, y축) 계산 이유
 *
 *
 * random_in_unit_disk() 유틸 함수로 반환받은 좌표값은
 * 표준기저벡터 [1, 0](x축), [0, 1](y축) 로 이루어진 좌표계 상의
 * 단위 원 내의 랜덤한 점을 반환해 줌.
 *
 * 그러나, 우리는 이 랜덤한 점을 현재 카메라 lens 상의
 * 랜덤한 점으로 변환해줘야 함.
 *
 * 따라서, 카메라 orienting 에 의해 회전된 두 카메라 기저벡터 u,v 와
 * defocus_radius 를 사용하여 조리개 반경만큼 노출된 dist_lens 상의
 * 로컬 좌표계의 두 기저벡터 defocus_disk_u, defocus_disk_v 를 계산한 것임.
 *
 * 이 로컬 기저벡터를 사용하면 단위 원 상의 랜덤한 점을
 * 조리개 반경만큼 노출된 카메라 lens 상의 랜덤한 점으로 변환(맵핑)할 수 있음.
 */
#endif /* CAMERA_HPP */
