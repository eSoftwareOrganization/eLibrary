#pragma once

#include <mmintrin.h> // MMX
#include <xmmintrin.h> // SSE 1
#include <emmintrin.h> // SSE 2
#include <pmmintrin.h> // SSE 3
#include <tmmintrin.h> // SSSE 3
#include <smmintrin.h> // SSE 4.1
#include <nmmintrin.h> // SSE 4.2
#include <wmmintrin.h> // AES
#include <immintrin.h> // AVX
#include <intrin.h> // All

namespace eLibrary {
    float getSummation(const float *NumberList, uintmax_t NumberSize) noexcept {
        __m256 NumberLoad1;
        __m256 NumberLoad2;
        __m256 NumberLoad3;
        __m256 NumberLoad4;
        __m256 NumberSum1 = _mm256_setzero_ps();
        __m256 NumberSum2 = _mm256_setzero_ps();
        __m256 NumberSum3 = _mm256_setzero_ps();
        __m256 NumberSum4 = _mm256_setzero_ps();
        const float *NumberCursor = NumberList;
        for (uintmax_t NumberBlock = 0;NumberBlock < NumberSize / 32;++NumberBlock) {
            NumberLoad1 = _mm256_load_ps(NumberCursor);
            NumberLoad2 = _mm256_load_ps(NumberCursor + 8);
            NumberLoad3 = _mm256_load_ps(NumberCursor + 16);
            NumberLoad4 = _mm256_load_ps(NumberCursor + 24);
            NumberSum1 = _mm256_add_ps(NumberSum1, NumberLoad1);
            NumberSum2 = _mm256_add_ps(NumberSum2, NumberLoad2);
            NumberSum3 = _mm256_add_ps(NumberSum3, NumberLoad3);
            NumberSum4 = _mm256_add_ps(NumberSum4, NumberLoad4);
            NumberCursor += 32;
        }
        NumberSum1 = _mm256_add_ps(NumberSum1, NumberSum2);
        NumberSum3 = _mm256_add_ps(NumberSum3, NumberSum4);
        NumberSum1 = _mm256_add_ps(NumberSum1, NumberSum3);
        const auto *NumberSum256 = (const float*) &NumberSum1;
        float NumberSum = NumberSum256[0] + NumberSum256[1] + NumberSum256[2] + NumberSum256[3] + NumberSum256[4] + NumberSum256[5] + NumberSum256[6] + NumberSum256[7];
        for (uintmax_t NumberRemain = 0;NumberRemain < NumberSize % 32;++NumberRemain)
            NumberSum += NumberCursor[NumberRemain];
        return NumberSum;
    }
}
