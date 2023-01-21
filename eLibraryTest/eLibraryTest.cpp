#ifdef __GNUC__
#include <bits/stdc++.h>
#else
#include <ctime>
#include <random>
#endif

#include <eLibrary>

using namespace eLibrary;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

std::mt19937 RandomEngine;
std::random_device RandomDevice;

TEST_CASE("IntegerAddition") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doAddition(NumberObject2).getValue() == Number1 + Number2);
        CHECK(NumberObject2.doAddition(NumberObject1).getValue() == Number2 + Number1);
    }
    intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
    Integer NumberObject1(Number1), NumberObject2(Number2);
    ankerl::nanobench::Bench().run("IntegerAddition", [&] {
        NumberObject1.doAddition(NumberObject2);
    });
    ankerl::nanobench::Bench().run("BuiltinIntegerAddition", [&] {
        ankerl::nanobench::doNotOptimizeAway(Number1 + Number2);
    });
}

TEST_CASE("IntegerComparison") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
        Integer NumberObject1(Number1), NumberObject2(Number2);

        if (Number1 > Number2) CHECK(NumberObject1.doCompare(NumberObject2) > 0);
        if (Number1 == Number2) CHECK(NumberObject1.doCompare(NumberObject2) == 0);
        if (Number1 < Number2) CHECK(NumberObject1.doCompare(NumberObject2) < 0);
    }
    intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
    Integer NumberObject1(Number1), NumberObject2(Number2);
    ankerl::nanobench::Bench().run("IntegerComparison", [&] {
        ankerl::nanobench::doNotOptimizeAway(NumberObject1.doCompare(NumberObject2));
    });
    ankerl::nanobench::Bench().run("BuiltinIntegerComparison", [&] {
        ankerl::nanobench::doNotOptimizeAway(Number1 <=> Number2);
    });
}

TEST_CASE("IntegerDivision") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doDivision(NumberObject2).getValue() == Number1 / Number2);
    }
    intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
    Integer NumberObject1(Number1), NumberObject2(Number2);
    ankerl::nanobench::Bench().run("IntegerDivision", [&] {
        NumberObject1.doDivision(NumberObject2);
    });
    ankerl::nanobench::Bench().run("BuiltinIntegerDivision", [&] {
        ankerl::nanobench::doNotOptimizeAway(Number1 / Number2);
    });
}

TEST_CASE("IntegerModulo") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doModulo(NumberObject2).getValue() == Number1 % Number2);
    }
    intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
    Integer NumberObject1(Number1), NumberObject2(Number2);
    ankerl::nanobench::Bench().run("IntegerModulo", [&] {
        NumberObject1.doModulo(NumberObject2);
    });
    ankerl::nanobench::Bench().run("BuiltinIntegerModulo", [&] {
        ankerl::nanobench::doNotOptimizeAway(Number1 % Number2);
    });
}

TEST_CASE("IntegerMultiplication") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doMultiplication(NumberObject2).getValue() == Number1 * Number2);
        CHECK(NumberObject2.doMultiplication(NumberObject1).getValue() == Number2 * Number1);
    }
    intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
    Integer NumberObject1(Number1), NumberObject2(Number2);
    ankerl::nanobench::Bench().run("IntegerMultiplication", [&] {
        NumberObject1.doMultiplication(NumberObject2);
    });
    ankerl::nanobench::Bench().run("BuiltinIntegerMultiplication", [&] {
        ankerl::nanobench::doNotOptimizeAway(Number1 * Number2);
    });
}

TEST_CASE("IntegerPower") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t NumberBase = RandomEngine() - 10000000, NumberExponent = RandomEngine() % 100;
        Integer NumberBaseObject(NumberBase), NumberExponentObject(NumberExponent), NumberResult(1);
        for (intmax_t NumberIteration = 0;NumberIteration < NumberExponent;++NumberIteration) NumberResult = NumberResult.doMultiplication(NumberBaseObject);

        CHECK(NumberBaseObject.doPower(NumberExponentObject).doCompare(NumberResult) == 0);
    }
    intmax_t NumberBase = RandomEngine() - 10000000, NumberExponent = RandomEngine() % 100;
    Integer NumberBaseObject(NumberBase), NumberExponentObject(NumberExponent);
    ankerl::nanobench::Bench().run("IntegerPower", [&] {
        NumberBaseObject.doPower(NumberExponentObject);
    });
}


TEST_CASE("IntegerPrime") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        unsigned NumberTarget = RandomEngine() % 1000000;
        CHECK(Mathematics::isPrime(NumberTarget) == Mathematics::isPrimeNative(NumberTarget));
    }
}

TEST_CASE("IntegerSubtraction") {
    RandomEngine.seed(RandomDevice() + time(nullptr));

    for (unsigned NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
        intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
        Integer NumberObject1(Number1), NumberObject2(Number2);

        CHECK(NumberObject1.doSubtraction(NumberObject2).getValue() == Number1 - Number2);
    }
    intmax_t Number1 = RandomEngine() - 10000000, Number2 = RandomEngine() - 10000000;
    Integer NumberObject1(Number1), NumberObject2(Number2);
    ankerl::nanobench::Bench().run("IntegerSubtraction", [&] {
        NumberObject1.doSubtraction(NumberObject2);
    });
    ankerl::nanobench::Bench().run("BuiltinIntegerSubtraction", [&] {
        ankerl::nanobench::doNotOptimizeAway(Number1 - Number2);
    });
}
