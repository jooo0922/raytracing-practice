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

  // 현재 벡터의 영벡터 여부 검사 함수
  bool near_zero() const
  {
    // 현재 벡터의 각 컴포넌트가 0인지 판단할 epsilon 값 정의
    auto s = 1e-8;

    // 현재 벡터의 각 컴포넌트의 절댓값이 epsilon 보다 작으면 0으로 판정
    return (std::fabs(e[0]) < s) && (std::fabs(e[1] < s)) && (std::fabs(e[2]) < s);
  };

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

// 단위 원(unit sphere. 반지름 1) 표면 상의 랜덤 방향벡터 연산 (rejection method 기반)
inline vec3 random_unit_vector()
{
  while (true)
  {
    auto p = vec3::random(-1, 1);
    auto lensq = p.length_squared();
    // 생성된 방향벡터의 길이가 10의 -160 제곱보다 작은 경우에도 reject 한다. (하단 필기 참고)
    if (1e-160 < lensq && lensq <= 1)
    {
      return p / sqrt(lensq);
    }
  }
}

// 반구 영역 표면 상의 랜덤 방향벡터 연산
inline vec3 random_on_hemisphere(const vec3 &normal)
{
  // 단위 원 표면 상의 랜덤 방향벡터 계산
  vec3 on_unit_sphere = random_unit_vector();

  // 단위 원 랜덤 방향벡터와 반구의 방향벡터(normal) 간 내적값을 기준으로 반구 영역 내 포함 여부 판단
  if (dot(on_unit_sphere, normal) > 0.0f)
  {
    // 내적값이 0.0f 보다 크면, 두 벡터의 사잇각이 90도 보다 작으므로, 랜덤 방향벡터는 반구 영역 내에 위치함.
    return on_unit_sphere;
  }
  else
  {
    // 내적값이 0.0f 보다 작으면,두 벡터의 사잇각이 90도 보다 크므로, 랜덤 방향벡터는 반구 영역을 벗어남.
    // -> 방향벡터를 뒤집어서 반구 영역 내에 위치하도록 교정함.
    return -on_unit_sphere;
  }
}

// 노멀벡터 n 을 기준으로 한 incident ray v 의 반사벡터 계산
inline vec3 reflect(const vec3 &v, const vec3 &n)
{
  // glsl reflect() 함수와 동일한 공식 (참고 : https://registry.khronos.org/OpenGL-Refpages/gl4/html/reflect.xhtml)
  // 반사벡터 계산 원리는 raytracing one weekeend 본문 참고 (참고 : https://raytracing.github.io/books/RayTracingInOneWeekend.html#metal/mirroredlightreflection)
  // 참고로, dot(v, n)은 내적으로 v 를 n 방향으로 투영한 정사영 벡터의 스칼라 성분(= 길이)를 구한 것 (참고 : 미적분학 p.470 > 정사영 벡터)
  return v - 2.0f * dot(v, n) * n;
};

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

/**
 * rejection method
 *
 *
 * random_unit_vector() 함수에서 단위 원 표면 상 랜덤 방향벡터 계산 시,
 * 아래와 같은 절차를 따름.
 *
 * 1. 단위 원 내의 랜덤 방향벡터를 생성한다.
 * 2. 이 방향벡터를 단위 원 표면까지 닿도록(= 단위 원 반지름 1과 길이를 맞추도록) 정규화한다.
 *
 * 이때, 1번 단계에서 단위 원을 벗어나는 방향벡터가 생성되었을 경우,
 * 해당 sample 은 reject 하고, 조건을 만족하는 방향벡터가 생성될 때까지 반복문을 계속 순회함.
 * -> 이것이 rejection method
 */

/**
 * 방향벡터의 길이를 1e-160(10의 -160제곱) 이상으로 제한한 이유
 *
 *
 * 단위 원 내의 랜덤 방향벡터의 길이가 0에 가까울 정도로 너무 작을 경우,
 * 너무 작은 소수점 길이를 제곱한 lensq 값은
 * double 타입이 부동소수점 정밀도로 표현 가능한 크기(= 1e-160)보다 작아질 확률이 있음.
 *
 * 이럴 경우 길이 제곱값 lensq 가 부동소수점 정밀도 한계로 인해 0 으로 underflow 됨.
 *
 * 따라서, p / sqrt(lensq) 는 분모가 0인 나눗셈이 되어버리고,
 * 이는 각 컴포넌트가 무한대인 bogus vector(= invalid vector) 로 계산됨.
 *
 * 이를 방지하기 위해 double 타입으로 저장 가능한 가장 작은 값인
 * 1e-160 보다 길이 제곱값이 작은 sample 도 rejection 하는 것!
 */

#endif /* VEC3_HPP */
