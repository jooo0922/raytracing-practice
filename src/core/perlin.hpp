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

private:
  static const int point_count = 256; // 난수 테이블 크기
  double randfloat[point_count];      // 난수 테이블 (재현 가능한 pseudo-random float 값들)

  // 난수 테이블 참조 인덱스를 해싱할 때 사용할 순열들을 무작위로 뒤섞어놓은 순열 테이블 (좌표 해싱용)
  int perm_x[point_count];
  int perm_y[point_count];
  int perm_z[point_count];
};

#endif /* PERLIN_HPP */
