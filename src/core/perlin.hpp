#ifndef PERLIN_HPP
#define PERLIN_HPP

#include "common/rtweekend.hpp"

/**
 * perlin noise 를 생성하는 절차적 알고리즘을 구현한 클래스
 */
class perlin
{
public:
  perlin()
  {
    // 1. [0.0, 1.0] 범위 랜덤 float 값으로 난수 테이블 초기화
    // for (int i = 0; i < point_count; i++)
    // {
    //   randfloat[i] = random_double();
    // }

    // 1. 단위 큐브의 각 8개 꼭지점(lattice point)에 할당할 랜덤 방향벡터 테이블 초기화
    for (int i = 0; i < point_count; i++)
    {
      randvec[i] = unit_vector(vec3::random(-1.0f, 1.0f));
    }

    // 2. 입력 좌표의 x, y, z 축별로 사용할 무작위로 섞인 순열 테이블 생성
    //    → 입력 좌표값을 기반으로 난수 테이블 인덱스 해싱할 때 사용됨
    perlin_generate_perm(perm_x);
    perlin_generate_perm(perm_y);
    perlin_generate_perm(perm_z);
  };

  // 3차원 입력 좌표를 받아 해싱된 난수 테이블 인덱스를 통해 랜덤 float 값을 읽어 반환하는 함수
  double noise_hash(const point3 &p) const
  {
    // 3. 입력 좌표를 스케일하여 격자 해상도를 높이고 (하단 필기 참고)
    //    정수 인덱스로 변환한 뒤 & 255 (bitwise AND 연산)를 통해 0~255 범위로 wrap-around (하단 필기 참고)
    auto i = int(4 * p.x()) & 255;
    auto j = int(4 * p.y()) & 255;
    auto k = int(4 * p.z()) & 255;

    // 4. 뒤섞인 각 순열 테이블에서 뽑은 값들을 bitwise XOR 해싱 → 0~255 범위 난수 테이블 인덱스 도출
    //    → randfloat[]에서 해당 위치의 난수 반환
    return randfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
  };

  // 입력 좌표 p 가 포함된 단위 큐브(모서리 길이가 1인 큐브)의 8개 꼭짓점 노이즈 값을 거리 가중치로 보간하여 부드러운 노이즈 값을 반환하는 함수 (하단 필기 참고)
  double noise_trilinear(const point3 &p) const
  {
    // 입력 좌표 p 가 단위 큐브 내에서 갖는 상대 좌표 (소수부) 추출 -> 좌표값 범위는 단위 큐브 내 좌표이므로 [0.0, 1.0] 사이
    auto u = p.x() - std::floor(p.x());
    auto v = p.y() - std::floor(p.y());
    auto w = p.z() - std::floor(p.z());

    // Hermite cubic smoothing (하단 필기 참고)
    // 단위 큐브 내 상대 좌표(이자 trilinear interpolation 의 가중치 역할)인 (u, v, w)를 3u² - 2u³ 형태의 hermite cubic 다항식으로 변환
    // → 상대 좌표 (u, v, w) 가 linear 형태 분포 -> smoothstep 형태 부드러운 곡선 분포를 띄도록 변환
    u = u * u * (3 - 2 * u);
    v = v * v * (3 - 2 * v);
    w = w * w * (3 - 2 * w);

    // 입력 좌표 p가 포함된 단위 큐브의 최소 꼭짓점 좌표 (정수부) 추출
    auto i = int(std::floor(p.x()));
    auto j = int(std::floor(p.y()));
    auto k = int(std::floor(p.z()));

    // 단위 큐브의 8개의 꼭짓점에 대한 난수 값(노이즈 값) 저장할 3차원 배열 테이블 선언
    double c[2][2][2];

    // 8개 꼭짓점 위치값을 기반으로 해싱된 난수 테이블 인덱스로부터 noise 값 조회
    for (int di = 0; di < 2; di++)
    {
      for (int dj = 0; dj < 2; dj++)
      {
        for (int dk = 0; dk < 2; dk++)
        {
          // 단위 큐브 x, y, z 축 방향의 거리인 di, dj, dk 를 최소 꼭짓점 좌표에 더해서 각 꼭지점 위치 계산
          // -> 거리값 di, dj, dk 는 각 꼭지점 순서로 볼 수 있으므로, 각 꼭지점 노이즈 값이 저장되는 테이블 인덱스로 활용
          c[di][dj][dk] = randfloat[
              // 각 꼭지점 위치값((i + di), (j + dj), (k + dk)) 을 & 255 (bitwise AND 연산)를 통해 0~255 범위로 wrap-around
              // wrap-around 된 정수값으로 순열 테이블에서 뽑은 값들을 bitwise XOR 해싱 → 0~255 범위 난수 테이블 인덱스 도출
              perm_x[(i + di) & 255] ^
              perm_y[(j + dj) & 255] ^
              perm_z[(k + dk) & 255]];
        };
      };
    };

    // 단위 큐브 내 상대 위치 (u, v, w) 와의 거리를 기반으로 8개의 꼭짓점 값을 3차원 보간하여 최종적인 부드러운 noise 값 계산
    return trilinear_interp(c, u, v, w);
  };

  // 입력 좌표 p 가 포함된 단위 큐브(모서리 길이가 1인 큐브)의 8개 꼭짓점에 랜덤 방향벡터(= gradient vector) 할당 후, 벡터 간 내적값을 거리 가중치로 보간하여 부드러운 노이즈 값을 반환하는 함수 (하단 필기 참고)
  // -> perlin::noise_trilinear 로 생성한 noise 의 blocky 한 자국을 줄이기 위해 개선된 방법 (gradient vector 기반 perlin noise 관련 하단 필기 참고)
  double noise_perlin(const point3 &p) const
  {
    // 입력 좌표 p 가 단위 큐브 내에서 갖는 상대 좌표 (소수부) 추출 -> 좌표값 범위는 단위 큐브 내 좌표이므로 [0.0, 1.0] 사이
    auto u = p.x() - std::floor(p.x());
    auto v = p.y() - std::floor(p.y());
    auto w = p.z() - std::floor(p.z());

    // 입력 좌표 p가 포함된 단위 큐브의 최소 꼭짓점 좌표 (정수부) 추출
    auto i = int(std::floor(p.x()));
    auto j = int(std::floor(p.y()));
    auto k = int(std::floor(p.z()));

    // 단위 큐브의 8개의 꼭짓점에 random 방향벡터(= gradient vector)를 저장할 3차원 배열 테이블 선언
    vec3 c[2][2][2];

    // 8개 꼭짓점 위치값을 기반으로 해싱된 랜덤 방향벡터 테이블 인덱스로부터 랜덤 방향벡터 조회
    for (int di = 0; di < 2; di++)
    {
      for (int dj = 0; dj < 2; dj++)
      {
        for (int dk = 0; dk < 2; dk++)
        {
          // 단위 큐브 x, y, z 축 방향의 거리인 di, dj, dk 를 최소 꼭짓점 좌표에 더해서 각 꼭지점 위치 계산
          // -> 거리값 di, dj, dk 는 각 꼭지점 순서로 볼 수 있으므로, 각 꼭지점 랜덤 방향벡터가 저장되는 테이블 인덱스로 활용
          c[di][dj][dk] = randvec[
              // 각 꼭지점 위치값((i + di), (j + dj), (k + dk)) 을 & 255 (bitwise AND 연산)를 통해 0~255 범위로 wrap-around
              // wrap-around 된 정수값으로 순열 테이블에서 뽑은 값들을 bitwise XOR 해싱 → 0~255 범위 랜덤 방향벡터 테이블 인덱스 도출
              perm_x[(i + di) & 255] ^
              perm_y[(j + dj) & 255] ^
              perm_z[(k + dk) & 255]];
        };
      };
    };

    // 단위 큐브 상의 8개의 꼭짓점에 할당된 gradient vector 와 offset vector 간 내적값을
    // 단위 큐브 내 상대 위치 (u, v, w) 와의 거리 가중치를 기반으로 3차원 보간하여 blocky 한 자국을 최소화한 노이즈 값 계산
    return perlin_interp(c, u, v, w);
  };

  // 여러 주파수의 노이즈를 누산하여 복잡한 텍스처 패턴을 생성하는 turbulence 함수 (turbulence noise 관련 하단 필기 참고)
  double turb(const point3 &p, int depth) const
  {
    auto accum = 0.0f;  // depth 단계별 noise 값 누산 결과를 저장할 변수 초기화
    auto temp_p = p;    // depth 단계에 따라 점차 조밀해지는 입력 좌표를 담을 임시 변수
    auto weight = 1.0f; // depth 단계에 따라 점차 작아지는 (noise 값에 적용할) 가중치 변수 초기화

    // 주어진 depth 단계만큼 입력 좌표의 주파수를 점차 높이고 가중치를 낮추며 노이즈를 누산
    for (int i = 0; i < depth; i++)
    {
      // gradient vector 기반 noise 계산 후 현재 단계의 가중치 적용하여 누산
      accum += weight * noise_perlin(temp_p);

      // 입력 좌표가 조밀해질수록 high frequency 노이즈의 가중치를 절반씩 감소시킴 -> 최종 노이즈 값에 대한 영향력 감소
      weight *= 0.5f;

      // depth 단계에 따라 입력 좌표가 점차 조밀해지도록 좌표 스케일 2배 적용
      // -> perlin::noise_hash() 함수에서 입력 좌표를 4로 스케일링하여 격자 해상도를 높인 것과 동일한 원리!
      temp_p *= 2.0f;
    }

    // gradient vector 기반 noise 는 내적값 기반으로 계산되므로 음수값이 나올 수 있음.
    // -> 음수 noise 값도 절댓값만 추출하여 양수로 변환함으로써, 서로 반대 위치의 두 노이즈값이 동일하게 계산되어 대칭 패턴을 이루도록 함.
    return std::fabs(accum);
  };

private:
  // x, y, z 축별로 사용할 순열 테이블을 받아서 무작위로 섞는 함수
  static void perlin_generate_perm(int *p)
  {
    // 아직 섞이지 않은 [0, 255] 사이의 순열 테이블 원본 생성
    for (int i = 0; i < point_count; i++)
    {
      p[i] = i;
    }

    // 생성된 순열 테이블을 무작위로 섞는다.
    permute(p, point_count);
  };

  // 순열 테이블을 무작위로 섞는 알고리즘 (Fisher–Yates shuffle 알고리즘 기반 배열 섞기)
  static void permute(int *p, int n)
  {
    // 순열 테이블의 마지막 요소 -> 첫 번째 요소까지 순회하며 섞음.
    for (int i = n - 1; i > 0; i--)
    {
      // [0, i] 중 하나의 인덱스 선택 (= 현재 순회 중인 요소의 앞쪽 요소들 중에서 무작위로 뽑음)
      int target = random_int(0, i);

      // 현재 순회 중인 요소와 앞쪽 요소의 위치를 교환한다.
      int tmp = p[i];
      p[i] = p[target];
      p[target] = tmp;
    }
  };

  // 단위 큐브 내 8개 꼭짓점에 할당된 random noise 값을
  // 단위 큐브 내 임의의 점 (u, v, w) 과의 거리를 기반으로 3차원 보간(trilinear interpolation)하는 함수
  static double trilinear_interp(double c[2][2][2], double u, double v, double w)
  {
    // 단위 큐브 내 임의의 점과의 거리 가중치에 따라 3차원 보간될 최종 부드러운 노이즈값 -> 당연히 [0.0, 1.0] 범위 내에서 계산됨.
    auto accum = 0.0f;

    // 3중 for 루프를 통해 8개 꼭짓점의 noise 값을 순회하며 단위 큐브 내 임의의 점과의 거리 가중치를 적용하여 3차원 보간
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 2; j++)
      {
        for (int k = 0; k < 2; k++)
        {
          // 각각의 꼭짓점에 대해, 단위 큐브 내 임의의 점 (u, v, w) 와의 "거리 기반" 3차원 보간 가중치를 적용하여 누산
          // -> 임의의 점 (u, v, w) 와 가까운 꼭지점일수록 높은 가중치가 적용되고, 먼 꼭지점일수록 낮은 가중치가 적용됨.
          accum += (i * u + (1 - i) * (1 - u))   // 단위 큐브 x 축 방향 보간 가중치: i == 0 인 꼭지점 → (1 - u), i == 1 인 꼭지점 → u
                   * (j * v + (1 - j) * (1 - v)) // 단위 큐브 y 축 방향 보간 가중치: j == 0 인 꼭지점 → (1 - v), j == 1 인 꼭지점 → v
                   * (k * w + (1 - k) * (1 - w)) // 단위 큐브 x 축 방향 보간 가중치: k == 0 인 꼭지점 → (1 - w), k == 1 인 꼭지점 → w
                   * c[i][j][k];                 // 현재 순회 중인 꼭짓점의 noise 값
        };
      };
    };

    return accum;
  };

  // 단위 큐브 내 8개 꼭짓점에 할당된 random 방향벡터(= gradient vector)와 각 꼭지점 ~ 단위 큐브 내 입력 좌표 (u, v, w) 방향벡터(= offset vector) 내적값을
  // 단위 큐브 내 임의의 점 (u, v, w) 과의 거리를 기반으로 3차원 보간(trilinear interpolation)하는 함수
  static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w)
  {
    // Hermite cubic smoothing (하단 필기 참고)
    // 단위 큐브 내 상대 좌표(이자 trilinear interpolation 의 가중치 역할)인 (u, v, w)를 3u² - 2u³ 형태의 hermite cubic 다항식으로 변환
    // → 상대 좌표 (u, v, w) 가 linear 형태 분포 -> smoothstep 형태 부드러운 곡선 분포를 띄도록 변환
    // → offset vector 인 vec3 weight_v 를 구하려면 단위 큐브 내 입력 상대 좌표의 원본값(u, v, w)를 알아야 하므로,
    // 원본값을 매개변수로 먼저 받은 뒤에 hermite cubic 다항식으로 입력 좌표 분포를 따로 변환하도록 해당 코드를 보간 함수 내부로 옮김.
    auto uu = u * u * (3 - 2 * u);
    auto vv = v * v * (3 - 2 * v);
    auto ww = w * w * (3 - 2 * w);

    // 단위 큐브 내 임의의 점과의 거리 가중치에 따라 3차원 보간될 최종 부드러운 노이즈값 -> 내적값이 누산되므로, [-1.0, 1.0] 범위 내에서 계산됨.
    auto accum = 0.0f;

    // 3중 for 루프를 통해 8개 꼭짓점의 random gradient vector 를 순회하며 단위 큐브 내 임의의 점을 향하는 offset vector 와 내적 후,
    // 내적값을 단위 큐브 내 임의의 점과의 거리 가중치를 적용하여 3차원 보간
    // -> 보간에 사용되는 거리 가중치 계산은 perlin::trilinear_interp 함수에서 구현했던 것과 동일
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 2; j++)
      {
        for (int k = 0; k < 2; k++)
        {
          // 현재 순회 중인 꼭지점 -> 단위 큐브 내 임의의 점 (u, v, w) 방향의 offset vector 계산
          vec3 weight_v(u - i, v - j, w - k);
          // 각각의 꼭짓점에 대해, 단위 큐브 내 임의의 점 (u, v, w) 와의 "거리 기반" 3차원 보간 가중치를 적용하여 누산
          // -> 임의의 점 (u, v, w) 와 가까운 꼭지점일수록 높은 가중치가 적용되고, 먼 꼭지점일수록 낮은 가중치가 적용됨.
          accum += (i * uu + (1 - i) * (1 - uu))   // 단위 큐브 x 축 방향 보간 가중치: i == 0 인 꼭지점 → (1 - uu), i == 1 인 꼭지점 → uu
                   * (j * vv + (1 - j) * (1 - vv)) // 단위 큐브 y 축 방향 보간 가중치: j == 0 인 꼭지점 → (1 - vv), j == 1 인 꼭지점 → vv
                   * (k * ww + (1 - k) * (1 - ww)) // 단위 큐브 x 축 방향 보간 가중치: k == 0 인 꼭지점 → (1 - ww), k == 1 인 꼭지점 → ww
                   * dot(c[i][j][k], weight_v);    // 현재 순회 중인 꼭짓점의 gradient vector 와 offset vector 내적값
        };
      };
    };

    return accum;
  };

private:
  static const int point_count = 256; // 난수 테이블 크기
  double randfloat[point_count];      // 난수 테이블 (재현 가능한 pseudo-random float 값들)
  vec3 randvec[point_count];          // random vector 테이블 (재현 가능한 pseudo-random 방향벡터들)

  // 난수 테이블 참조 인덱스를 해싱할 때 사용할 순열들을 무작위로 뒤섞어놓은 순열 테이블 (좌표 해싱용)
  int perm_x[point_count];
  int perm_y[point_count];
  int perm_z[point_count];
};

/**
 * Perlin noise 알고리즘
 *
 *
 * 1. randfloat[] : 256개의 고정된 난수 float 값 테이블 생성
 *    - 단순한 white noise처럼 보이는 값들이지만, 이후 해싱을 통해 공간적으로 의미 있게 사용됨
 *
 * 2. perm_x, perm_y, perm_z[] : 축별로 무작위로 뒤섞인 순열 테이블
 *    - 입력 좌표의 정수 인덱스를 기반으로 이 배열에서 값을 가져오고,
 *      서로 다른 축의 값을 XOR 연산하여 난수 테이블 접근 인덱스를 해싱함
 *
 * 3. noise(p)
 *    - 입력된 3D 좌표 p를 정수 격자 인덱스로 스케일링
 *    - perm 테이블에서 해당 좌표 축마다 섞인 인덱스를 추출
 *    - XOR 해싱으로 최종 인덱스를 계산하여 randfloat[]에서 대응되는 값 반환
 *    - perlin noise 에서 생성된 난수는 입력된 좌표값이 동일하면 항상 일관된 난수를 생성하는 "재현 가능한 난수"
 *
 * 이 구현은 단일 float 값을 반환하는 "pseudo-random noise function"까지만 완성된 상태임.
 * 아직 Perlin Noise의 핵심적인 연속성 구현 요소들(gradient, fade, 보간 등)은 포함되지 않음.
 *
 * 다음 단계에서는:
 *  - 각 격자점에 gradient 벡터를 부여하고
 *  - 입력 좌표에서의 상대 위치 벡터와 내적(dot product) 수행
 *  - 그 결과들을 fade 보간 함수와 trilinear interpolation을 통해
 *    공간적으로 부드럽고 연속적인 Perlin noise 값을 생성하게 됨.
 */

/**
 * Fisher–Yates Shuffle 알고리즘
 *
 * - permute()에서 사용하는 배열 섞기 방식
 * - O(n) 시간에 순열을 무작위로 만드는 고전적인 알고리즘
 */

/**
 * & 255 연산자 (bitwise AND 연산)
 *
 * - 입력 좌표를 정수 인덱스로 변환한 후 난수 테이블 크기(256)에 맞춰 [0, 255] 범위로 wrap-around 처리
 * - int(4 * p.x()) & 255 는 int(4 * p.x()) % 256 (나머지 연산) 과 같은 의미이지만 bitwise 연산이 더 빠름
 * - 하위 8비트만 유지하는 비트 마스킹을 통해 안전하게 배열 접근 가능
 */

/**
 * perline::noise() 에서 입력 좌표에 4를 scaling 하는 이유
 *
 *
 * Perlin noise는 좌표를 격자로 나누어 동작하기 때문에,
 * p.x()와 같이 실수 좌표를 정수로 캐스팅하면 동일한 격자에 묶여버려 noise 값이 변하지 않음.
 *
 * 예를 들어, 입력 좌표 p.x() 가 각각 0.1, 0.3, 0.6, 0.9, 1.0 이라면,
 * int(p.x()) 로 바로 캐스팅하면 각각 0, 0, 0, 0, 1 이 되어버림.
 *
 * 반면, int(4 * p.x()) 스케일링 후 캐스팅하면 각각 0, 1, 2, 3, 4 가 되어 버림.
 *
 * 즉, 입력 좌표 상으로 동일한 [0.1, 1.0] 구간이지만,
 * 얼만큼 scaling 하느냐에 따라 동일한 구간 내에서 더 다양한 범위의 정수값을 기반으로
 * 난수 테이블 인덱스를 해싱할 수 있음.
 *
 * 즉, 동일한 구간 내에서 난수 테이블로부터 더 다양한 난수값을 참조해올 수 있다는 뜻임.
 *
 * 이렇게 하면 결과적으로 동일한 구간 내에서
 * 더 다양한 랜덤성을 보이면서 조밀하고 세밀한 noise 패턴이 나오게 되는 것!
 *
 * 그래서 4는 임의의 scaling factor 일 뿐,
 * 더 큰 값을 사용하면 noise 가 조밀해지고 detail 이 많아질 것임.
 */

/**
 * 부드러운 perlin noise (smoothing out noise)
 *
 *
 * 두 번째 버전의 noise() 함수는 입력 좌표 p가 포함된 단위 큐브를 기준으로,
 * 그 8개 꼭짓점에 저장된 난수 값을 거리 기반의 가중치로 보간하여 noise 값을 생성한다.
 *
 * trilinear_interp() 함수는 단위 큐브 내 세 방향(x, y, z)에 대해 선형 보간을 중첩한 3차원 보간 함수로,
 * 가까운 꼭짓점일수록 더 큰 가중치를 부여하여 전체 noise 값의 부드러운 공간적 변화를 유도한다.
 *
 * 보간에 사용되는 (u, v, w)는 p의 소수부로, 입력 좌표 p 의 단위 큐브 내에서의 상대 위치를 의미한다.
 *
 * 이 방식은 주변 꼭짓점들과의 연속적 가중합을 통해
 * 주변에 인접한 p들 간의 유사한 noise 값이 나오도록 하여 smoothing out 되는 효과를 볼 수 있다.
 */

/**
 * Smoothing noise with trilinear interpoation(3차원 보간)
 *
 *
 * trilinear_interp 함수는 단위 큐브 내 8개 꼭짓점에 저장된 noise 값을,
 * 입력 좌표 p 의 상대 위치 (u, v, w) 와의 거리 기반으로 3차원 보간하여 하나의 noise 값으로 결합한다.
 *
 * 이 보간 과정은 기본적으로 1차원 선형 보간 식:
 *     lerp(a, b, t) = (1 - t) * a + t * b
 * 의 3차원 공간 확장판으로 이해할 수 있다.
 *
 * - 여기서 가중치 t는 입력 좌표의 단위 큐브 내 상대 위치 u, v, w로 대응된다. (거리 기반 가중치)
 * - 보간 기준점 a와 b는 단위 큐브 내 8개의 각 꼭짓점 noise 값에 해당한다.
 *
 * 이를 3차원으로 확장하면:
 *     value = Σ_{i,j,k in {0,1}} weight(i,j,k) * c[i][j][k]
 *     weight(i,j,k) = (i*u + (1-i)*(1-u)) * (j*v + (1-j)*(1-v)) * (k*w + (1-k)*(1-w))
 *
 * 이처럼 각 축의 방향으로 i,j,k 값을 0 또는 1로 설정하여,
 * 각 꼭짓점에서 단위 큐브 내 입력 좌표(u, v, w)와의 거리 기반 보간 가중치를 계산 및 noise 값에 적용하여
 * 최종적으로 모두 누산(accumulate)한다.
 *
 * 보간 다항식이 3차원으로 확장되면 식이 길고 복잡해지기 때문에,
 * 이를 코드로 간결하게 구현하기 위해 3중 for loop + 누산기(accum)를 사용한다.
 *
 * 이 방식은 trilinear 보간의 수학적 원리를 그대로 코드로 풀어낸 형태이며,
 * 주변 꼭짓점과의 거리 기반으로 noise 값을 혼합하여 부드러운 결과를 만든다.
 *
 * 참고: https://en.wikipedia.org/wiki/Trilinear_interpolation#Mathematical_description
 */

/**
 * Hermite Smoothing (Smoothstep)
 *
 * ------------------------------------------
 * u, v, w는 입력 좌표 p의 소수부로서, 단위 큐브 내에서의 상대 위치이며
 * 동시에 trilinear interpolation 에서의 보간 가중치 역할을 한다.
 *
 * 기본적으로는 u, v, w ∈ [0,1] 의 **선형 분포**를 갖는다.
 *     - 이 선형 분포는 보간 값 자체는 연속적이지만,
 *     - **기울기(도함수)는 상수(항상 동일)**이므로 변화의 연속성이 부족해 시각적으로 딱딱한 느낌이 날 수 있다.
 *     - 따라서, 이러한 선형 분포를 기반으로 적용된 noise smoothing 은 Mach band(하단 관련 필기 참고) 착시 효과를 일으킬 수밖에 없다.
 *
 * 이를 줄이기 위해 **Hermite cubic 다항식** (smoothstep):
 *     smoothstep(u) = 3u² - 2u³
 * 를 사용하여 u, v, w를 선형 분포 → 곡선 분포로 변환한다.
 *
 * 이 다항식의 장점:
 * - 도함수: 6u - 6u² → u = 0, 1에서 도함수가 0이 되며, 곡선의 시작과 끝이 수평 기울기를 가짐
 * - 즉, 도함수가 연속적(= 이를 **미분 가능한 상태** 라고도 표현)이며 부드러운 곡선 형태 (C¹ 연속)로 변화함
 * - 결과적으로 보간된 값의 변화율(기울기)까지 부드러워지며 시각적 아티팩트를 줄여준다.
 *
 * 정리하면, smoothstep (Hermite cubic: 3u² - 2u³)은 다음과 같은 특징을 가진다:
 *     - 시작점(u=0), 끝점(u=1)에서 도함수(기울기)가 0이므로 수평으로 부드럽게 연결됨
 *     - 전체 구간에서 기울기가 연속적으로 변화함 (C¹ 연속)
 *     - 따라서 **보간된 값뿐 아니라 그 변화율(기울기)도 부드러워짐**
 *
 * Hermite 보간은 이런 경계에서의 변화율(기울기)도 부드럽게 만들어
 * Mach band 와 같은 인위적인 시각 아티팩트를 완화시킨다.
 *
 * ✅ 응용 분야:
 *  - 그래픽스: Procedural texture, shading noise, anti-aliasing
 *  - 애니메이션: easing 함수 (linear → smooth transition)
 *  - 수학적 곡선 모델링, 자연스러운 움직임 생성 등
 *
 * 요약:
 * 단순히 보간된 값이 연속인 것만으로는 부족하며,
 * **그 값의 변화율(기울기)까지 연속적(도함수 연속)일 때** 시각적으로 더 부드럽고 자연스러운 결과를 얻을 수 있다.
 */

/**
 * 📌 Mach Band 현상 설명:
 *
 * ------------------------
 * 인간의 시각은 밝기가 일정하게 변하는 경계 근처에서
 * 실제보다 더 뚜렷한 경계나 명암 대비를 느끼는 착시 현상이 발생한다.
 * 이 현상을 **Mach band**라고 하며, 선형 보간만 사용하는 경우 더 뚜렷하게 드러난다.
 *     → 밝기 변화가 급격해 보이는 경계선처럼 인식됨
 */

/**
 * gradient vector 기반 perlin noise 계산 알고리즘
 *
 *
 * 기존 trilinear interpolation 기반의 노이즈(perlin::noise_trilinear)는
 * 단위 큐브의 8개 lattice point마다 고정된 float 값을 할당하고,
 * 입력 좌표와 각 꼭짓점 간 거리(가중치)를 기반으로 단순 선형 보간하여 값을 계산한다.
 *
 * 이 방식은 보간의 기준점 값(lerp 함수로 치면 a, b에 해당)이 입력 좌표에 관계없이 고정되어 있어,
 * 단위 큐브 경계에서 색상이나 값이 급격히 변하는 blocky한 패턴이 시각적으로 드러나는 단점이 있다.
 *
 * 이를 해결하기 위해 **perlin::noise_perlin** 은
 * 단위 큐브 상 8개의 lattice point마다 임의의 방향을 갖는 **랜덤 단위 벡터(= gradient vector)**를 할당하고,
 * 보간 시에는 각 꼭짓점으로부터 단위 큐브 내 입력 좌표까지의 벡터(= offset vector)와 해당 랜덤 벡터 간의 **내적**을 사용한다.
 * 내적은 벡터 간 각도에 따라 결과가 달라지므로, **같은 꼭짓점이라도 입력 좌표 위치가 다르면 다른 보간 기준값**이 나오게 된다.
 *
 * 이렇게 하면 기존과는 달리 보간 기준값 자체(lerp 함수의 a, b)가 입력 위치마다 변동되므로,
 * 결과적으로 더 부드럽고 자연스러운 노이즈가 생성되며, 경계면에서의 blocky artifact가 사라진다.
 *
 * 이 원리는 간단한 선형 보간(lerp)에서의 기준값 a, b를 공간 좌표에 따라 동적으로 변하게 만든 것이며,
 * Perlin 노이즈의 핵심적인 시각적 개선점 중 하나다.
 */

/**
 * 주파수 단계별 noise 값을 중첩하는 turbulence noise 알고리즘
 *
 *
 * turbulence는 여러 주파수의 Perlin 노이즈를 계층적으로 누산하여 더욱 복잡한 패턴을 생성하는 기법이다.
 *
 * 각 단계마다 입력 좌표(temp_p)를 2배씩 스케일하여 점점 더 조밀한(high frequency) 노이즈를 생성하며,
 * 동시에 각 단계에서의 가중치(weight)는 1.0 → 0.5 → 0.25 → ... 식으로 절반씩 줄여서,
 * 고주파 노이즈는 희미하게, 저주파 노이즈는 강하게 반영되도록 조정한다.
 *
 * 결과적으로 low frequency noise를 기반으로 시각적 구조를 만들고,
 * 그 위에 high frequency noise가 섬세하게 얹히는 방식으로 매우 자연스러운 텍스처 효과를 낸다.
 *
 * 마지막에 std::fabs(accum)을 사용하는 이유는,
 * 노이즈 값이 음수일 경우 시각적으로 반대되는 패턴이 나타나므로,
 * 이를 대칭되게 처리하여 marble texture 등에서 반복적이고 부드러운 무늬를 만들기 위함이다.
 */

#endif /* PERLIN_HPP */
