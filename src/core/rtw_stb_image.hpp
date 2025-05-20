#ifndef RTW_STB_IMAGE_HPP
#define RTW_STB_IMAGE_HPP

// stb_image.h 라이브러리 내에서 사용되는 복잡한 문법에 의한 MSVC 컴파일러 warning 을 강제로 비활성화함
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG // 이미지 로딩 실패 시 친절한 오류 메시지 제공
#include <stb_image.h>

#include <cstdlib>
#include <iostream>

#endif /* RTW_STB_IMAGE_HPP */
