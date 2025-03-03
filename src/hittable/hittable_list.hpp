#ifndef HITTABLE_LIST_HPP
#define HITTABLE_LIST_HPP

#include <memory>
#include <vector>
#include "hittable.hpp"

/**
 * hittable_list 클래스
 *
 *
 * ray 와 intersection 검사를 통해 렌더링할 모든 hittable object 를 관리하는 클래스
 * 일반적인 그래픽 엔진에서의 scene 과 유사한 개념
 *
 * 이때, hittable_list::hit() 함수를 통해
 * 카메라에서 출발해서 각 pixel 방향으로 casting 된 모든 ray 들에 대해
 * scene 에 추가된 object 들을 순회하며 ray intersection 검사를 수행함.
 *
 * -> 이 과정에서 해당 ray 방향에서 카메라로부터
 * 가장 가까운 지점에서 발생한 intersection 의 hit_record 를 출력변수를 통해 전달.
 */
class hittable_list : public hittable
{
public:
  hittable_list() {};
  hittable_list(std::shared_ptr<hittable> object) { add(object); };

public:
  // scene 에 추가된 hittable object 제거
  void clear() { objects.clear(); };

  // scene 에 새로운 hittable object 추가
  void add(std::shared_ptr<hittable> object)
  {
    objects.push_back(object);
  };

  // scene 에 추가된 hittable object 순회하며 ray intersection 검사
  bool hit(const ray &r, double ray_tmin, double ray_tmax, hit_record &rec) const override
  {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_tmax;

    // 등록된 hittable object 를 순회하며 현재 ray 와의 intersection 검사
    for (const auto &object : objects)
    {
      if (object->hit(r, ray_tmin, closest_so_far, temp_rec))
      {
        // 하나라도 교차된 object 가 존재하면 true 반환
        hit_anything = true;

        // ray 의 비율값 t 최대 유효 범위를 현재 교차된 지점의 비율값 t 로 업데이트
        // -> (현재 ray 방향에서) 점점 카메라로부터 더 가까운 교차점 t 를 찾아나갈 수 있도록 함.
        closest_so_far = temp_rec.t;

        // 현재 교차점에 대한 hit_record 업데이트
        rec = temp_rec;
      }
    }

    return hit_anything;
  };

public:
  // scene 에 hittable object 를 추가하는 컨테이너
  // -> RAII 패턴 기반 메모리 안정적 관리 및 예상치 못한 소멸자 호출 방지 등을 위해 hittable 객체를 std::shared_ptr 로 관리
  std::vector<std::shared_ptr<hittable>> objects;
};

#endif /* HITTABLE_LIST_HPP */
