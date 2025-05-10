#ifndef BVH_NODE_HPP
#define BVH_NODE_HPP

#include "aabb.hpp"
#include "hittable/hittable.hpp"
#include "hittable/hittable_list.hpp"

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
  bvh_node(std::vector<std::shared_ptr<hittable>> &objects, size_t start, size_t end) {
    // To be implemented later.
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

#endif /* BVH_NODE_HPP */
