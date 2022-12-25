#ifdef __GNUC__
#include <bits/stdc++.h>
#else
#include <chrono>
#include <cstdio>
#include <ctime>
#include <random>
#endif

#include <eLibrary>

using namespace eLibrary;

#ifdef eLibraryDoctest
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

std::mt19937 RandomEngine;
std::random_device RandomDevice;

TEST_CASE("IntegerAddition") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();
    for (unsigned NumberEpoch = 0;NumberEpoch < 1000000;++NumberEpoch) {
        unsigned Number1 = RandomEngine(), Number2 = RandomEngine();
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doAddition(NumberObject2).getValue() == (long long) Number1 + Number2);
    }
    printf("IntegerAddition %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - TimeStart).count());
}

TEST_CASE("IntegerDivision") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();
    for (unsigned NumberEpoch = 0;NumberEpoch < 1000000;++NumberEpoch) {
        unsigned Number1 = RandomEngine(), Number2 = RandomEngine();
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doDivision(NumberObject2).getValue() == (long long) Number1 / Number2);
    }
    printf("IntegerDivision %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - TimeStart).count());
}

TEST_CASE("IntegerModulo") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();
    for (unsigned NumberEpoch = 0;NumberEpoch < 1000000;++NumberEpoch) {
        unsigned Number1 = RandomEngine(), Number2 = RandomEngine();
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doModulo(NumberObject2).getValue() == (long long) Number1 % Number2);
    }
    printf("IntegerModulo %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - TimeStart).count());
}

TEST_CASE("IntegerMultiplication") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();
    for (unsigned NumberEpoch = 0;NumberEpoch < 1000000;++NumberEpoch) {
        unsigned Number1 = RandomEngine(), Number2 = RandomEngine();
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doMultiplication(NumberObject2).getValue() == (long long) Number1 * Number2);
    }
    printf("IntegerMultiplication %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - TimeStart).count());
}

TEST_CASE("IntegerSubtraction") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();
    for (unsigned NumberEpoch = 0;NumberEpoch < 1000000;++NumberEpoch) {
        unsigned Number1 = RandomEngine(), Number2 = RandomEngine();
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doSubtraction(NumberObject2).getValue() == (long long) Number1 - Number2);
    }
    printf("IntegerSubtraction %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - TimeStart).count());
}

#else

int main(int ArgumentCount, char *ArgumentList[]) {
    return 0;
}

#endif