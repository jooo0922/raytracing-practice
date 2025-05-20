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

  // 주어진 파일 경로로부터 이미지 로드 (float 포맷으로)
  bool load(const std::string &filename) {

  };
};

// MSVC 컴파일러 warning 비활성화 복구
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* RTW_STB_IMAGE_HPP */
