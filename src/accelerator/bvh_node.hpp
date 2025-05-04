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
};

#endif /* BVH_NODE_HPP */
