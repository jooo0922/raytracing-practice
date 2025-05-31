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
    for (int i = 0; i < point_count; i++)
    {
      randfloat[i] = random_double();
    }

    // 2. 입력 좌표의 x, y, z 축별로 사용할 무작위로 섞인 순열 테이블 생성
    //    → 입력 좌표값을 기반으로 난수 테이블 인덱스 해싱할 때 사용됨
    perlin_generate_perm(perm_x);
    perlin_generate_perm(perm_y);
    perlin_generate_perm(perm_z);
  };

  // 3차원 입력 좌표를 받아 해싱된 난수 테이블 인덱스를 통해 랜덤 float 값을 읽어 반환하는 함수
  // double noise(const point3 &p) const
  // {
  //   // 3. 입력 좌표를 스케일하여 격자 해상도를 높이고 (하단 필기 참고)
  //   //    정수 인덱스로 변환한 뒤 & 255 (bitwise AND 연산)를 통해 0~255 범위로 wrap-around (하단 필기 참고)
  //   auto i = int(4 * p.x()) & 255;
  //   auto j = int(4 * p.y()) & 255;
  //   auto k = int(4 * p.z()) & 255;

  //   // 4. 뒤섞인 각 순열 테이블에서 뽑은 값들을 bitwise XOR 해싱 → 0~255 범위 난수 테이블 인덱스 도출
  //   //    → randfloat[]에서 해당 위치의 난수 반환
  //   return randfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
  // };

  // 입력 좌표 p 가 포함된 단위 큐브(모서리 길이가 1인 큐브)의 8개 꼭짓점 노이즈 값을 거리 가중치로 보간하여 부드러운 노이즈 값을 반환하는 함수 (하단 필기 참고)
  double noise(const point3 &p) const
  {
    // 입력 좌표 p 가 단위 큐브 내에서 갖는 상대 좌표 (소수부) 추출 -> 좌표값 범위는 단위 큐브 내 좌표이므로 [0.0, 1.0] 사이
    auto u = p.x() - std::floor(p.x());
    auto v = p.y() - std::floor(p.y());
    auto w = p.z() - std::floor(p.z());

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

private:
  static const int point_count = 256; // 난수 테이블 크기
  double randfloat[point_count];      // 난수 테이블 (재현 가능한 pseudo-random float 값들)

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

#endif /* PERLIN_HPP */
