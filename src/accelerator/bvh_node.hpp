#ifndef BVH_NODE_HPP
#define BVH_NODE_HPP

#include "aabb.hpp"
#include "hittable/hittable.hpp"
#include "hittable/hittable_list.hpp"

#include <algorithm>

/**
 * BVH 노드를 나타내는 클래스
 *
 * hittable 클래스를 상속받으므로,
 * 다른 primitive 객체들처럼 hit 검사 가능
 */
class bvh_node : public hittable
{
public:
  // hittable_list 를 받아 BVH 트리의 루트 노드를 구성하는 생성자
  // -> BVH 구성을 위해 hittable_list::objects 정렬해야 하므로, 원본 컨테이너를 변경하지 못하도록 복사본을 인자로 받음
  bvh_node(hittable_list list)
      : bvh_node(list.objects, 0, list.objects.size()) {};

  // hittable 객체 컨테이너 내에서 start ~ end 구간을 BVH 노드로 재귀 분할하는 생성자
  bvh_node(std::vector<std::shared_ptr<hittable>> &objects, size_t start, size_t end)
  {
    // 현재 BVH 노드의 bounding box 누적 계산할 AABB 초기화
    bbox = aabb::empty;

    // 전체 hittable 객체 리스트를 돌면서 현재 BVH 노드를 감싸야 하는 AABB 계산
    for (size_t object_index = start; object_index < end; object_index++)
    {
      bbox = aabb(bbox, objects[object_index]->bounding_box());
    }

    // 현재 BVH 노드 AABB 슬랩 중 가장 긴 축(x, y, z 중)을 기준으로 선택 → 더 공간적으로 효과적인 분할을 위함 (하단 필기 참고)
    int axis = bbox.longest_axis();

    // 선택된 축에 따라 AABB 슬랩 min 값 기준 비교 함수 설정 (std::sort에 사용됨)
    auto comparator = (axis == 0)   ? box_x_compare
                      : (axis == 1) ? box_y_compare
                                    : box_z_compare;

    // 현재 BVH 노드에 들어온 hittable 객체 수 계산
    size_t object_span = end - start;

    if (object_span == 1)
    {
      // 객체가 1개일 경우:
      /**
       * BVH의 재귀 종료 지점이므로 left/right 모두 동일한 객체를 가리키도록 함
       *
       * -> why? 모든 서브트리 포인터가 채워져 있어야
       * BVH 탐색 시 null pointer를 체크하지 않고도
       * 항상 left와 right를 대상으로 hit()을 호출할 수 있기 때문.
       */
      left = right = objects[start];
    }
    else if (object_span == 2)
    {
      // 객체가 2개일 경우: 둘을 각각 left, right로 할당
      left = objects[start];
      right = objects[start + 1];
    }
    else
    {
      // 객체가 3개 이상일 경우:
      // 1. 선택된 축 기준으로 AABB.min 값 기준 정렬 수행
      std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

      // 2. 정렬된 리스트를 중간에서 나눠서 두 서브트리로 재귀 구성
      // -> std::make_shared<bvh_node>(...) 를 호출하는 순간 현재의 생성자 함수가 다시 재귀 호출됨!
      auto mid = start + object_span / 2;
      left = std::make_shared<bvh_node>(objects, start, mid);
      right = std::make_shared<bvh_node>(objects, mid, end);
    }

    // 현재 노드의 bounding box는 left/right 서브트리의 bounding box를 감싼(= 합친) AABB
    bbox = aabb(left->bounding_box(), right->bounding_box());
  };

  // 광선과의 교차 여부 검사
  bool hit(const ray &r, interval ray_t, hit_record &rec) const override
  {
    // 현재 BVH 노드의 AABB 가 광선과 교차하지 않으면 false 반환 후 종료
    if (!bbox.hit(r, ray_t))
    {
      return false;
    }

    // 좌측/우측 자식 노드(서브트리)에 대해 재귀적으로 hit 검사
    bool hit_left = left->hit(r, ray_t, rec);
    bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

    // 두 서브트리 중 하나라도 충돌이 발견되면 true 반환
    return hit_left || hit_right;
  };

  // 현재 노드의 AABB 반환 함수
  aabb bounding_box() const override { return bbox; };

private:
  std::shared_ptr<hittable> left;  // 좌측 서브트리 또는 리프 노드(실제 primitive 객체(ex> sphere))
  std::shared_ptr<hittable> right; // 우측 서브트리 또는 리프 노드(실제 primitive 객체(ex> sphere))
  aabb bbox;                       // 현재 BVH 노드를 감싸는 AABB

private:
  /**
   * 주어진 두 hittable 객체의 AABB에서 axis_index 축 방향 슬랩(interval)의 최소값(min)을 기준으로
   * 오름차순 정렬을 위해 정의한 비교 함수 (std::sort에서 사용)
   */
  static bool box_compare(
      const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b, int axis_index)
  {
    auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
    auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
    return a_axis_interval.min < b_axis_interval.min;
  };

  // 두 hittable 객체 AABB 의 x축 슬랩을 기준으로 오름차순 정렬하기 위해 정의한 비교 함수
  static bool box_x_compare(const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b)
  {
    return box_compare(a, b, 0);
  };

  // 두 hittable 객체 AABB 의 y축 슬랩을 기준으로 오름차순 정렬하기 위해 정의한 비교 함수
  static bool box_y_compare(const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b)
  {
    return box_compare(a, b, 1);
  };

  // 두 hittable 객체 AABB 의 z축 슬랩을 기준으로 오름차순 정렬하기 위해 정의한 비교 함수
  static bool box_z_compare(const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b)
  {
    return box_compare(a, b, 2);
  };
};

/**
 * bvh_node 클래스 설계
 *
 * --------------------------------------
 * 이 클래스는 BVH 트리의 '노드'이자 '트리 자체'를 동시에 표현한다.
 * - hittable을 상속하므로 다른 오브젝트처럼 hit() 인터페이스를 그대로 사용 가능하다.
 * - 설계상 트리와 노드를 분리하지 않고, 단일 클래스로 트리를 구성한다.
 *   (루트 노드도 결국 하나의 노드로 표현됨)
 *
 * hit() 함수의 구조는 다음과 같다:
 *   1. 이 노드의 AABB(bbox)와 광선의 교차 여부를 검사하여 빠르게 거를 수 있는 경우를 제거
 *   2. AABB를 통과한다면, 좌/우 자식 노드에 대해 각각 hit() 호출
 *   3. 먼저 hit된 쪽의 t값(rec.t)을 기준으로,
 *      나머지 한쪽의 검사 범위(ray_t.max)를 그보다 작게 줄여 검사한다.
 *
 * 3번 과정 최적화는 ray tracing의 렌더링 원리에 기반한 것으로,
 * -----------------------------------------------------------
 * ✅ 픽셀을 결정하는 데에 필요한 것은 광선 상에서 카메라로부터 가장 가까운 오브젝트 단 하나뿐이다.
 * ✅ 따라서 한쪽 서브트리에서 더 가까운 hit이 발견되었다면,
 *    나머지 쪽 서브트리에서 그보다 더 가까운 hit이 있는 경우에만 검사할 가치가 있다.
 * ✅ 이후의 hit() 검사에서는 이미 발견된 rec.t보다 더 큰 값(= 카메라로부터 더 멀리 있는 오브젝트)은 무시해도 되므로,
 *    검사 범위를 [ray_t.min, rec.t]로 줄여서 **불필요한 계산을 절감**할 수 있다.
 *
 * 이러한 계층적 pruning 전략은 BVH가 제공하는 가장 강력한 성능 향상 기법 중 하나이며,
 * 전체 오브젝트에 대해 선형 검색하지 않고도 효율적인 교차 판정을 가능하게 한다.
 *
 * 이러한 최적화는 수천~수백만 개의 오브젝트를 처리하는 레이트레이서의 성능을 좌우한다.
 */

/**
 * 📌 Why splitting along the longest axis improves BVH performance
 *
 *
 * BVH 트리의 각 노드를 구성할 때, 가장 긴 축을 기준으로 분할하면 다음과 같은 장점이 있음:
 *
 * 1. 🎯 공간 분할의 품질이 높아짐:
 *    - 긴 축을 따라 정렬 및 분할하면, 좌/우 자식 노드의 AABB 가 덜 겹치고, 더 명확하게 분리됨
 *    - 이는 광선(ray)이 자식 노드 AABB 중 한 쪽만 통과하게 되는 경우를 더 많이 만들어냄
 *
 * 2. 🚀 불필요한 hit() 연산 감소:
 *    - 덜 겹치는 bounding box는 BVH 탐색 시 더 많은 노드를 '건너뛸 수 있는' 기회를 줌
 *
 * 3. 🔄 트리 균형 향상:
 *    - 축 방향 데이터 분포에 맞춰 나누면, 전체 트리 깊이가 줄어들고 탐색 효율이 증가함
 *
 * 그 결과, 더 빠른 렌더링 속도를 얻을 수 있으며, 저자의 실험에서는 약 18% 성능 향상을 보임.
 */

#endif /* BVH_NODE_HPP */
