#include <algorithm>
#include <ctime>
#include <random>

#include <eLibrary>
using namespace eLibrary;

#undef max
#undef min
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

ankerl::nanobench::Bench TestBench;
std::mt19937 RandomEngine;
std::random_device RandomDevice;

TEST_SUITE("Integer") {
    TEST_CASE("IntegerAddition") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doAddition(NumberObject2).doCompare(Number1 + Number2) == 0);
        }
        Integer NumberP1(RandomEngine()), NumberP2(RandomEngine());
        TestBench.run("IntegerAdditionPP", [&] {
            NumberP1.doAddition(NumberP2);
        });
    }

    TEST_CASE("IntegerComparison") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            if (Number1 > Number2) CHECK(NumberObject1.doCompare(NumberObject2) > 0);
            if (Number1 == Number2) CHECK(NumberObject1.doCompare(NumberObject2) == 0);
            if (Number1 < Number2) CHECK(NumberObject1.doCompare(NumberObject2) < 0);
        }
        Integer NumberObject1(RandomEngine()), NumberObject2(RandomEngine());
        TestBench.run("IntegerComparisonPP", [&] {
            ankerl::nanobench::doNotOptimizeAway(NumberObject1.doCompare(NumberObject2));
        });
    }

    TEST_CASE("IntegerDivision") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doDivision(NumberObject2).doCompare(Number1 / Number2) == 0);
        }
        Integer NumberP1(RandomEngine()), NumberP2(RandomEngine());
        TestBench.run("IntegerDivisionPP", [&] {
            NumberP1.doDivision(NumberP2);
        });
    }

    TEST_CASE("IntegerModulo") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doModulo(NumberObject2).doCompare(Number1 % Number2) == 0);
        }
        Integer NumberP1(RandomEngine()), NumberP2(RandomEngine());
        TestBench.run("IntegerModuloPP", [&] {
            NumberP1.doModulo(NumberP2);
        });
    }

    TEST_CASE("IntegerMultiplication") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = (RandomEngine() & 1 ? RandomEngine() : -RandomEngine()) % 10000000, Number2 = (RandomEngine() & 1 ? RandomEngine() : -RandomEngine()) % 10000000;
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doMultiplication(NumberObject2).doCompare(Number1 * Number2) == 0);
        }
        Integer NumberP1(RandomEngine()), NumberP2(RandomEngine());
        TestBench.run("IntegerMultiplicationPP", [&] {
            NumberP1.doMultiplication(NumberP2);
        });
    }

    TEST_CASE("IntegerPower") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            uintmax_t NumberBase = RandomEngine() % 10000, NumberExponent = RandomEngine() % 200;
            Integer NumberBaseObject(NumberBase), NumberExponentObject(NumberExponent), NumberResult(1);
            for (uintmax_t NumberIteration = 0; NumberIteration < NumberExponent; ++NumberIteration)
                NumberResult = NumberResult.doMultiplication(NumberBaseObject);

            CHECK(NumberBaseObject.doPower(NumberExponentObject).doCompare(NumberResult) == 0);
        }
        Integer NumberBaseObject(std::min(RandomEngine(), (uint_fast32_t) 1000000)), NumberExponentObject(RandomEngine() % 1000);
        TestBench.run("IntegerPower", [&] {
            NumberBaseObject.doPower(NumberExponentObject);
        });
    }

    TEST_CASE("IntegerPrime") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            unsigned NumberTarget = RandomEngine() % 100000;
            CHECK(Mathematics::isPrime(NumberTarget) == Mathematics::isPrimeNative(NumberTarget));
        }
        uintmax_t NumberSource = RandomEngine();
        TestBench.run("IntegerPrimeMixed", [&] {
            Mathematics::isPrime(NumberSource);
        });
    }

    TEST_CASE("IntegerSubtraction") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doSubtraction(NumberObject2).doCompare(Number1 - Number2) == 0);
        }
        Integer NumberObject1(RandomEngine()), NumberObject2(RandomEngine());
        TestBench.run("IntegerSubtraction", [&] {
            NumberObject1.doSubtraction(NumberObject2);
        });
    }
}

TEST_SUITE("String") {
    TEST_CASE("StringCase") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t CharacterSource = RandomEngine() % 26;
            CHECK(String(char16_t(CharacterSource + 65)).toLowerCase().doCompare(String(char16_t(CharacterSource + 97))) == 0);
            CHECK(String(char16_t(CharacterSource + 97)).toUpperCase().doCompare(String(char16_t(CharacterSource + 65))) == 0);
        }
    }

    TEST_CASE("StringConversion") {
        RandomEngine.seed(RandomDevice() + time(nullptr));

        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t NumberSource = RandomEngine();
            CHECK(String(std::to_string(NumberSource)).doCompare(String(std::to_wstring(NumberSource)).toU32String()) == 0);
        }
    }
}

int main(int ParameterCount, char *ParameterList[]) {
    TestBench.performanceCounters(true);
    return doctest::Context(ParameterCount, ParameterList).run();
}
