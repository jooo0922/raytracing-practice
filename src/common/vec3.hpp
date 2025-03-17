#ifndef VEC3_HPP
#define VEC3_HPP

// using 을 이용해서 namespace 안의 특정 함수만 가져올 수 있음.
using std::sqrt;

// vec3 클래스 구현
class vec3
{
public:
  double e[3]; // vec3 의 세 컴포넌트 멤버변수를 double 타입 배열로 선언

  // vec3 생성자 선언
  vec3() : e{0, 0, 0} {}                                   // 매개변수가 없는 기본생성자 > 영벡터로 초기화
  vec3(double e0, double e1, double e2) : e{e0, e1, e2} {} // vec3 의 세 컴포넌트를 직접 매개변수로 전달받을 때의 생성자 오버로딩

  // vec3 각 컴포넌트에 대한 getter 메서드
  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  // -연산자 오버로딩
  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }

  // [] 연산자 오버로딩
  /**
   * 반환 타입이 참조변수(double&)인 연산자 오버로딩은 멤버변수의 값을 호출부에서 변경할 수 있고,
   * 상수 멤버함수로 정의된 연산자 오버로딩은 멤버변수의 값을 '복사'해서 전달함. -> 멤버변수 불변 보장
   */
  double operator[](int i) const { return e[i]; }
  double &operator[](int i) { return e[i]; }

  // += 연산자 오버로딩
  // 연산자 오버로딩 시, 메서드 체이닝을 사용할 수 있도록 객체 자신의 포인터(this) 반환
  vec3 &operator+=(const vec3 &v)
  {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  // *= 연산자 오버로딩
  vec3 &operator*=(double t)
  {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  // /= 연산자 오버로딩
  vec3 &operator/=(double t)
  {
    return *this *= 1 / t;
  }

  // 벡터의 길이를 계산하는 상수 함수들 정의
  double length() const
  {
    return sqrt(length_squared());
  }

  double length_squared() const
  {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  // 랜덤한 방향벡터 계산하는 static 멤버 함수들 정의
  static vec3 random()
  {
    return vec3(random_double(), random_double(), random_double());
  }

  static vec3 random(double min, double max)
  {
    return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
  }
};

// vec3 에 대한 별칭으로써 point3 선언
using point3 = vec3;

/** vec3 관련 연산자 오버로딩 */
// << 연산자 오버로딩
inline std::ostream &operator<<(std::ostream &out, const vec3 &v)
{
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

// + 연산자 오버로딩
inline vec3 operator+(const vec3 &u, const vec3 &v)
{
  return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

// - 연산자 오버로딩
inline vec3 operator-(const vec3 &u, const vec3 &v)
{
  return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

// * 연산자 오버로딩
inline vec3 operator*(const vec3 &u, const vec3 &v)
{
  return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

// 매개변수에 따른 * 연산자 오버로딩 세분화
inline vec3 operator*(double t, const vec3 &v)
{
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3 &v, double t)
{
  return t * v;
}

// / 연산자 오버로딩
inline vec3 operator/(vec3 v, double t)
{
  return (1 / t) * v;
}

/** 벡터 연산 관련 util 함수 */
// 벡터 내적 연산
inline double dot(const vec3 &u, const vec3 &v)
{
  return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

// 벡터 외적 연산
inline vec3 cross(const vec3 &u, const vec3 &v)
{
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
              u.e[2] * v.e[0] - u.e[0] * v.e[2],
              u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

// 벡터 정규화 연산
inline vec3 unit_vector(vec3 v)
{
  return v / v.length();
}

/**
 * inline 키워드
 *
 *
 * 이 키워드가 붙은 함수는 컴파일 과정에서 컴파일러가 함수 호출부를 만나면
 * 함수의 실제 코드로 직접 삽입해서 컴파일하도록 요청함.
 *
 * 이를 통해 함수 호출에 의한 전반적인 오버헤드를 감소시킬 수 있으나,
 * 컴파일러가 최적화를 위해 상황에 따라 함수를 인라인 처리하지 않을수도 있음.
 *
 * 즉, 어디까지나 컴파일러에게 '제안'을 하는 것에 불과함.
 */

/**
 * std::ostream
 *
 *
 * std::cout 같은 스트림 출력을 담당하는 클래스 타입을 의미함.abort
 *
 * << 연산자를 오버로딩할 경우,
 * 아래와 같이 std::cout 같은 표준 스트림 출력 클래스를 사용해서
 * 현재 vec3 클래스에 관한 콘솔 출력 형태를 사용자가 직접 정의할 수 있음.
 *
 * vec3 v(1.0, 2.0, 3.0);
 * std::cout << v; // 1.0 2.0 3.0 와 같이 출력
 */

#endif /* VEC3_HPP */
