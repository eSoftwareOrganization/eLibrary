#include <random>

#include <eLibrary>
using namespace eLibrary;

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

ankerl::nanobench::Bench TestBench;
std::mt19937 RandomEngine;
std::mt19937_64 RandomEngine64;
std::random_device RandomDevice;

TEST_SUITE("Collection") {
    TEST_CASE("ArrayList") {
        ArrayList<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % std::max(NumberList.getElementSize(), 1LL), {RandomEngine()});
        CHECK(NumberList.getElementSize() == 10000);
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % std::max(NumberList.getElementSize(), 1LL));
        CHECK(NumberList.isEmpty());
    }

    TEST_CASE("ArraySet") {
        ArraySet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK(NumberList.getElementSize() <= 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("DoubleLinkedList") {
        DoubleLinkedList<uintmax_t> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % std::max(NumberList.getElementSize(), 1LL), RandomEngine());
        CHECK(NumberList.getElementSize() == 10000);
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % std::max(NumberList.getElementSize(), 1LL));
        CHECK(NumberList.isEmpty());
    }

    TEST_CASE("DoubleLinkedSet") {
        DoubleLinkedSet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK(NumberList.getElementSize() <= 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("SingleLinkedList") {
        SingleLinkedList<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % std::max(NumberList.getElementSize(), 1LL), {RandomEngine()});
        CHECK(NumberList.getElementSize() == 10000);
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % std::max(NumberList.getElementSize(), 1LL));
        CHECK(NumberList.isEmpty());
    }

    TEST_CASE("SingleLinkedSet") {
        SingleLinkedSet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK(NumberList.getElementSize() <= 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }
}

TEST_SUITE("Integer") {
    TEST_CASE("IntegerAddition") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doAddition(NumberObject2).doCompare(Number1 + Number2) == 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerAddition", [&] {
            NumberObject1.doAddition(NumberObject2);
        });
    }

    TEST_CASE("IntegerComparison") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            if (Number1 > Number2) CHECK(NumberObject1.doCompare(NumberObject2) > 0);
            if (Number1 == Number2) CHECK(NumberObject1.doCompare(NumberObject2) == 0);
            if (Number1 < Number2) CHECK(NumberObject1.doCompare(NumberObject2) < 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerComparison", [&] {
            ankerl::nanobench::doNotOptimizeAway(NumberObject1.doCompare(NumberObject2));
        });
    }

    TEST_CASE("IntegerDivision") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doDivision(NumberObject2).doCompare(Number1 / Number2) == 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerDivision", [&] {
            NumberObject1.doDivision(NumberObject2);
        });
    }

    TEST_CASE("IntegerFactorial") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            uintmax_t NumberValue = RandomEngine() & 1 ? RandomEngine() % 1000 : -RandomEngine() % 1000;
            Integer NumberObject(NumberValue), NumberResult(1);
            for (uintmax_t NumberIteration = 1; NumberIteration <= NumberValue; ++NumberIteration)
                NumberResult = NumberResult.doMultiplication(NumberIteration);

            CHECK(NumberObject.doFactorial().doCompare(NumberResult) == 0);
        }
        Integer NumberObject(RandomEngine64() % 5000);
        TestBench.run("IntegerFactorial", [&] {
            NumberObject.doFactorial();
        });
    }

    TEST_CASE("IntegerModulo") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doModulo(NumberObject2).doCompare(Number1 % Number2) == 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerModulo", [&] {
            NumberObject1.doModulo(NumberObject2);
        });
    }

    TEST_CASE("IntegerMultiplication") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = (RandomEngine() & 1 ? RandomEngine() : -RandomEngine()) % 10000000, Number2 = (RandomEngine() & 1 ? RandomEngine() : -RandomEngine()) % 10000000;
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doMultiplication(NumberObject2).doCompare(Number1 * Number2) == 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerMultiplication", [&] {
            NumberObject1.doMultiplication(NumberObject2);
        });
    }

    TEST_CASE("IntegerPower") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t NumberBase = (RandomEngine() & 1 ? RandomEngine() : -RandomEngine()) % 10000000, NumberExponent = RandomEngine() % 300;
            Integer NumberBaseObject(NumberBase), NumberExponentObject(NumberExponent), NumberResult(1);
            for (intmax_t NumberIteration = 0; NumberIteration < NumberExponent; ++NumberIteration)
                NumberResult = NumberResult.doMultiplication(NumberBaseObject);

            CHECK(NumberBaseObject.doPower(NumberExponentObject).doCompare(NumberResult) == 0);
        }
        Integer NumberBaseObject(RandomEngine64()), NumberExponentObject(RandomEngine64() % 10000);
        TestBench.run("IntegerPower", [&] {
            NumberBaseObject.doPower(NumberExponentObject);
        });
    }

    TEST_CASE("IntegerPrime") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            unsigned NumberTarget = RandomEngine() % 200000;
            CHECK(Mathematics::isPrime(NumberTarget) == Mathematics::isPrimeNative(NumberTarget));
        }
        uintmax_t NumberSource = RandomEngine64();
        TestBench.run("IntegerPrimeMixed", [&] {
            Mathematics::isPrime(NumberSource);
        });
    }

    TEST_CASE("IntegerSubtraction") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK(NumberObject1.doSubtraction(NumberObject2).doCompare(Number1 - Number2) == 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerSubtraction", [&] {
            NumberObject1.doSubtraction(NumberObject2);
        });
    }
}

TEST_SUITE("String") {
    TEST_CASE("StringCase") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t CharacterSource = RandomEngine() % 26;
            CHECK(String(char16_t(CharacterSource + 65)).toLowerCase().doCompare(String(char16_t(CharacterSource + 97))) == 0);
            CHECK(String(char16_t(CharacterSource + 97)).toUpperCase().doCompare(String(char16_t(CharacterSource + 65))) == 0);
        }
    }

    TEST_CASE("StringConversion") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t NumberSource = RandomEngine();
            CHECK(String(std::to_string(NumberSource)).doCompare(String(std::to_wstring(NumberSource)).toU32String()) == 0);
        }
    }
}

int main(int ParameterCount, char *ParameterList[]) {
    RandomEngine64.seed(RandomDevice());
    RandomEngine.seed(RandomEngine64());
    TestBench.performanceCounters(true);
    return doctest::Context(ParameterCount, ParameterList).run();
}
