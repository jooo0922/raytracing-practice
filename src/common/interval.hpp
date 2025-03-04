#ifndef INTERVAL_HPP
#define INTERVAL_HPP

/**
 * interval 클래스
 *
 * ray (비율값 t 의)유효 범위를 실수인 최솟값(min)과 최댓값(max)으로 추상화한 클래스
 * -> hittable::hit() 함수 매개변수에서 ray 유효 범위 지정 시 사용됨.
 */
class interval
{
public:
  // 유효 범위의 최솟값을 양의 무한대, 최댓값을 음의 무한대로 초기화하는 기본 생성자 -> ray 유효 범위가 없는(= inteval 이 비어 있는) 상태로 초기화
  interval() : min(+infinity), max(-infinity) {};
  interval(double min, double max) : min(min), max(max) {};

public:
  // ray 유효 범위 간격 반환
  double size() const { return max - min; };

  // 특정 비율값 t 가 ray 유효 범위 내부 또는 경계선(min, max)에 포함되는지 확인
  bool contains(double x) const { return min <= x && x <= max; };

  // 특정 비율값 t 가 ray 유효 범위 내부에 완전히 포함되는지 확인 (경계선에 걸치는 경우 제외)
  bool surrounds(double x) const { return min < x && x < max; };

public:
  // 자주 사용하게 될 ray 유효 범위를 정적 멤버변수로 미리 정의 -> 어디서든 쉽게 접근 및 사용 가능.
  static const interval empty;    // ray 비어있는 유효 범위 -> 유효 범위가 없음. 주로 유효 범위의 초기 상태로 사용.
  static const interval universe; // ray 무한대 유효 범위 -> 어떤 비율값 t 든 해당 범위 내에 포함.

public:
  double min;
  double max;
};

// 클래스 외부에서 정적 멤버변수 초기화
const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

/**
 * 정적 멤버변수는 클래스 외부에서 초기화해야 하는 이유
 *
 *
 * C++ 표준에 따르면, 정적 멤버변수는 프로그램 실행 중
 * 정적 영역(static storage)에 '최초로 단 한 번만' 초기화되어야 함.
 *
 * 그러나, 클래스 내부에서 정적 멤버변수를 초기화하면
 * 해당 클래스가 '인스턴스화될 때마다' 정적 멤버변수를 초기화하려고 시도하므로
 * 컴파일 에러가 발생함.
 *
 * 반면, 클래스 외부에서 정적 멤버변수를 초기화하면,
 * '클래스 인스턴스화 횟수와 관계없이' 정적 영역에 단 한 번만 초기화되므로
 * 컴파일 에러 없이 올바르게 동작함.
 *
 * 따라서, 일반적인 정적 멤버변수는 클래스 내부에서 '선언만' 가능하며,
 * '초기화는 클래스 외부' (헤더 또는 .cpp 파일)에서 수행해야 함.
 *
 * 단, 정적 멤버변수가 `constexpr` (컴파일 타임 상수)인 경우,
 * 예외적으로 클래스 내부에서 즉시 초기화 가능함.
 *
 * 이유는, `constexpr` 변수는 '컴파일 타임에 단 한 번만 초기화'되므로
 * 정적 변수의 초기화 규칙을 위반하지 않기 때문.
 *
 * 그러나, `interval` 클래스에서 사용하는 `infinity` 값은
 * `std::numeric_limits<double>::infinity()` 를 사용하므로
 * 컴파일 타임 상수(literal)가 아니며, `constexpr`로 초기화할 수 없음.
 */

#endif /* INTERVAL_HPP */
