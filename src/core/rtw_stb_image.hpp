#ifndef RTW_STB_IMAGE_HPP
#define RTW_STB_IMAGE_HPP

// stb_image.h 라이브러리 내 안좋은 코드 패턴으로 인해 발생하는 MSVC 컴파일러 warning 강제 비활성화
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG // 이미지 로딩 실패 시 친절한 오류 메시지 제공
#include <stb_image.h>

#include <cstdlib> // getenv() 함수 사용을 위해 include
#include <iostream>

/**
 * rtw_image 클래스
 *
 *
 * 이미지 파일을 선형 색공간(linear color space)의 32-bit float 포맷([0.0, 1.0] 범위)으로 로드하고,
 * 8-bit RGB 포맷([0, 255] 범위)으로도 접근할 수 있도록 변환하는 헬퍼 클래스.
 *
 * - stb_image.h 를 이용하여 이미지 파일을 float[] 배열로 로딩한다.
 * - 이후 convert_to_bytes()를 통해 8-bit RGB 포맷(bdata)로 변환하여 사용 가능하다.
 * - RTW_IMAGES 환경변수 또는 상대 경로 기반으로 이미지 경로를 유연하게 탐색한다.
 * - MSVC의 경고 억제를 통해 외부 라이브러리 포함 시 빌드 로그를 깔끔하게 유지한다.
 */
class rtw_image
{
public:
  // 기본 생성자: 아무 작업도 하지 않음
  rtw_image() {};

  // 이미지 파일명을 받아서 로딩 시도. 경로는 RTW_IMAGES 또는 상대 경로들을 탐색함
  rtw_image(const char *image_filename)
  {
    // 주어진 경로 또는 RTW_IMAGES 환경변수 하위 디렉토리에서 이미지 파일을 탐색
    auto filename = std::string(image_filename);
    // 사용자나 시스템이 사전에 설정한 환경 변수를 읽는 함수 -> 저자가 RTW_IMAGES 라는 환경 변수를 시스템에 등록해놓고 사용한 듯.
    auto imagedir = getenv("RTW_IMAGES");

    // 다양한 경로 후보들을 순차적으로 시도하여 이미지 로드
    // 만약 등록된 환경 변수가 있다면, 해당 환경 변수 내 이미지 파일들을 먼저 로드해본다.
    if (imagedir && load(std::string(imagedir) + "/" + image_filename))
      return;
    if (load(filename))
      return;
    if (load("images/" + filename))
      return;
    if (load("../images/" + filename))
      return;
    if (load("../../images/" + filename))
      return;
    if (load("../../../images/" + filename))
      return;
    if (load("../../../../images/" + filename))
      return;
    if (load("../../../../../images/" + filename))
      return;
    if (load("../../../../../../images/" + filename))
      return;

    // 모든 경로에서 이미지 로드 실패 시 에러 메시지 출력
    std::cerr << "ERROR: Could not load image file '" << image_filename << "'\n";
  };

  // 소멸자: 동적 할당된 이미지 데이터 메모리 해제
  ~rtw_image()
  {
    delete[] bdata;   // 8-bit RGB 데이터 메모리 해제
    STBI_FREE(fdata); // stb_image에서 할당한 float 데이터 메모리 해제
  }

  // 주어진 파일 경로로부터 이미지 로드 (float 포맷으로)
  bool load(const std::string &filename)
  {
    // stbi_loadf()를 통해 float 선형 색공간 이미지 데이터 ([0.0, 1.0] 범위) 로딩
    auto n = bytes_per_pixel; // 로드할 원본 이미지 채널 수 (출력은 RGB 3채널로 고정)
    fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
    if (fdata == nullptr)
      return false;

    // 이미지 한 줄의 바이트 수 계산 (이미지 폭 × 픽셀당 바이트 수)
    bytes_per_scanline = image_width * bytes_per_pixel;

    // 32-bit float 데이터 → 8-bit RGB 변환
    converto_to_bytes();
    return true;
  };

  // 로드된 이미지 너비 반환 (로딩 실패 시 0)
  int width() const
  {
    return (fdata == nullptr) ? 0 : image_width;
  };

  // 로드된 이미지 높이 반환 (로딩 실패 시 0)
  int height() const
  {
    return (fdata == nullptr) ? 0 : image_height;
  };

private:
  // fdata(32-bit float) → bdata(8-bit RGB)로 변환
  // .ppm 파일로 출력하기 위해 [0.0, 1.0] float 색상값을 [0, 255] 정수로 변환 (color.hpp 참고)
  void converto_to_bytes() {

  };

private:
  const int bytes_per_pixel = 3;  // 픽셀당 바이트 수 (RGB → 3바이트)
  float *fdata = nullptr;         // 선형 색공간 float 이미지 데이터 ([0.0, 1.0] 범위)
  unsigned char *bdata = nullptr; // 변환된 8-bit RGB 데이터 ([0, 255] 범위)
  int image_width = 0;            // 로드된 이미지 너비
  int image_height = 0;           // 로드된 이미지 높이
  int bytes_per_scanline = 0;     // 로드된 이미지 한 줄(scanline) 당 바이트 수 = image_width * bytes_per_pixel
};

// MSVC 컴파일러 warning 비활성화 복구
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* RTW_STB_IMAGE_HPP */
