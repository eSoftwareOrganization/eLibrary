#include <eLibrary>
using namespace eLibrary;

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <random>

ankerl::nanobench::Bench TestBench;
std::mt19937 RandomEngine;
std::mt19937_64 RandomEngine64;
std::random_device RandomDevice;

TEST_SUITE("Collection") {
    TEST_CASE("ArrayList") {
        ArrayList<uintmax_t> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
        NumberList.addElement(RandomEngine64());
        TestBench.run("ArrayList(Append)", [&]{
            NumberList.addElement(RandomEngine64());
        });
        TestBench.run("ArrayList(Remove)", [&]{
            if (!NumberList.isEmpty()) NumberList.removeIndex(-1);
        });
    }

    TEST_CASE("ArraySet") {
        ArraySet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
        Integer NumberSource(RandomEngine64());
        TestBench.run("ArraySet(Append)", [&]{
            NumberList.addElement(NumberSource);
        }).run("ArraySet(Remove)", [&]{
            if (NumberList.isContains(NumberSource)) NumberList.removeElement(NumberSource);
        });
    }

    TEST_CASE("DoubleLinkedList") {
        DoubleLinkedList<uintmax_t> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
        TestBench.run("DoubleLinkedList(Append)", [&]{
            NumberList.addElement(RandomEngine64());
        }).run("DoubleLinkedList(Remove)", [&]{
            if (!NumberList.isEmpty()) NumberList.removeIndex(-1);
        });
    }

    TEST_CASE("DoubleLinkedSet") {
        DoubleLinkedSet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
        Integer NumberSource(RandomEngine64());
        TestBench.run("DoubleLinkedSet(Append)", [&]{
            NumberList.addElement(NumberSource);
        }).run("DoubleLinkedSet(Remove)", [&]{
            if (NumberList.isContains(NumberSource)) NumberList.removeElement(NumberSource);
        });
    }

    TEST_CASE("SingleLinkedList") {
        SingleLinkedList<uintmax_t> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
        TestBench.run("SingleLinkedList(Append)", [&]{
            NumberList.addElement(RandomEngine64());
        }).run("SingleLinkedList(Remove)", [&]{
            if (!NumberList.isEmpty()) NumberList.removeIndex(-1);
        });
    }

    TEST_CASE("SingleLinkedSet") {
        SingleLinkedSet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
        Integer NumberSource(RandomEngine64());
        TestBench.run("SingleLinkedSet(Append)", [&]{
            NumberList.addElement(NumberSource);
        }).run("SingleLinkedSet(Remove)", [&]{
            if (NumberList.isContains(NumberSource)) NumberList.removeElement(NumberSource);
        });
    }

    TEST_CASE("TreeSet") {
        TreeSet<Integer> NumberList;
        for (unsigned NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
        Integer NumberSource(RandomEngine64());
        TestBench.run("TreeSet(Append)", [&]{
            NumberList.addElement(NumberSource);
        }).run("TreeSet(Remove)", [&]{
            if (NumberList.isContains(NumberSource)) NumberList.removeElement(NumberSource);
        });
    }
}

TEST_SUITE("CharacterString") {
    TEST_CASE("StringConversion") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t NumberSource = RandomEngine();
            CHECK_EQ(String(std::to_string(NumberSource)).doCompare(String(std::to_wstring(NumberSource)).toU32String()), 0);
        }
    }
}

#include <thread>

TEST_SUITE("Concurrent") {
    TEST_CASE("AtomicInteger") {
        AtomicNumber<int64_t> NumberSource(0);
        for (uint32_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            uint32_t NumberValue = RandomEngine() % 50000;
            auto ThreadFunctionDecrement = [&]() {
                for (intmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch)
                    NumberSource.getAndDecrement();
            };
            auto ThreadFunctionIncrement = [&]() {
                for (intmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch)
                    NumberSource.getAndIncrement();
            };
            std::thread ThreadIncrement1(ThreadFunctionIncrement), ThreadIncrement2(ThreadFunctionIncrement);
            ThreadIncrement1.join();
            ThreadIncrement2.join();
            CHECK_EQ(NumberSource.getValue(), NumberValue * 2);
            std::thread ThreadDecrement1(ThreadFunctionDecrement), ThreadDecrement2(ThreadFunctionDecrement);
            ThreadDecrement1.join();
            ThreadDecrement2.join();
            CHECK_EQ(NumberSource.getValue(), 0);
        }
        TestBench.run("AtomicIntegerIncrement", [&]{
            NumberSource.getAndIncrement();
        });
    }
}

#if eLibraryFeature(IO)
TEST_SUITE("File") {
    TEST_CASE("FileInputStream&FileOutputStream") {
        IO::FileOutputStream StreamOutput({u"FileStream.tst"});
        uint8_t NumberBufferSource[10000];
        for (uint8_t &NumberEpoch : NumberBufferSource)
            NumberEpoch = RandomEngine() % 256;
        StreamOutput.doWrite(NumberBufferSource, 0, 10000);
        StreamOutput.doClose();
        IO::FileInputStream StreamInput({u"FileStream.tst"});
        uint8_t NumberBuffer[10000];
        StreamInput.doRead(NumberBuffer, 0, 10000);
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch)
            CHECK_EQ(NumberBufferSource[NumberEpoch], NumberBuffer[NumberEpoch]);
        StreamInput.doClose();
    }
}
#endif

TEST_SUITE("Mathematics") {
    TEST_CASE("FunctionCosine") {
        for (uint32_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            auto NumberSource = Mathematics::toRadians(double(RandomEngine() & 1 ? RandomEngine() : -RandomEngine()));
            CHECK_EQ(Mathematics::doCosine(NumberSource), doctest::Approx(::cos(NumberSource)));
        }
        auto NumberSource = Mathematics::toRadians((long double) (RandomEngine() & 1 ? RandomEngine64() : -RandomEngine64()));
        TestBench.run("FunctionCosine", [&]{
            ankerl::nanobench::doNotOptimizeAway(Mathematics::doCosine(NumberSource));
        });
    }

    TEST_CASE("FunctionEvolution") {
        for (uint32_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            double NumberSource = Objects::getMinimum(RandomEngine(), 2U);
            CHECK_EQ(Mathematics::doEvolution(NumberSource, 2.), doctest::Approx(::sqrt(NumberSource)));
        }
        double NumberSource = Objects::getMinimum(RandomEngine(), 2U);
        TestBench.run("FunctionEvolution", [&]{
            ankerl::nanobench::doNotOptimizeAway(Mathematics::doEvolution(NumberSource, 2.));
        });
    }

    TEST_CASE("FunctionExponent") {
        for (uint32_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            long double NumberSource = RandomEngine() % 50;
            CHECK_EQ(Mathematics::doExponent(NumberSource), doctest::Approx(::exp(NumberSource)));
        }
        long double NumberSource = RandomEngine() % 50;
        TestBench.run("FunctionExponent", [&]{
            ankerl::nanobench::doNotOptimizeAway(Mathematics::doExponent(NumberSource));
        });
    }

    TEST_CASE("FunctionLogarithmE") {
        for (uint32_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            double NumberSource = Objects::getMinimum(RandomEngine64(), 2U);
            CHECK_EQ(Mathematics::doLogarithmE(NumberSource), doctest::Approx(::log(NumberSource)));
        }
        double NumberSource = Objects::getMinimum(RandomEngine64(), 2U);
        TestBench.run("FunctionLogarithmE", [&]{
            ankerl::nanobench::doNotOptimizeAway(Mathematics::doLogarithmE(NumberSource));
        });
    }

    TEST_CASE("FunctionSine") {
        for (uint32_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            auto NumberSource = Mathematics::toRadians(double(RandomEngine() & 1 ? RandomEngine() : -RandomEngine()));
            CHECK_EQ(Mathematics::doSine(NumberSource), doctest::Approx(::sin(NumberSource)));
        }
        auto NumberSource = Mathematics::toRadians((long double) (RandomEngine() & 1 ? RandomEngine64() : -RandomEngine64()));
        TestBench.run("FunctionSine", [&]{
            ankerl::nanobench::doNotOptimizeAway(Mathematics::doSine(NumberSource));
        });
    }
}

TEST_SUITE("Number") {
    TEST_CASE("IntegerAddition") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doAddition(NumberObject2).doCompare(Number1 + Number2), 0);
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

            if (Number1 > Number2) CHECK_GT(NumberObject1.doCompare(NumberObject2), 0);
            if (Number1 == Number2) CHECK_EQ(NumberObject1.doCompare(NumberObject2), 0);
            if (Number1 < Number2) CHECK_LT(NumberObject1.doCompare(NumberObject2), 0);
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

            CHECK_EQ(NumberObject1.doDivision(NumberObject2).doCompare(Number1 / Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerDivision", [&] {
            NumberObject1.doDivision(NumberObject2);
        });
    }

    TEST_CASE("IntegerFactorial") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            Integer NumberObject(RandomEngine() % 10000 + 1);
            CHECK_EQ(NumberObject.doFactorial().doCompare(NumberObject.doSubtraction(1).doFactorial().doMultiplication(NumberObject)), 0);
        }
        Integer NumberObject(RandomEngine() % 50000);
        TestBench.run("IntegerFactorial", [&] {
            NumberObject.doFactorial();
        });
    }

    TEST_CASE("IntegerModulo") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine(), Number2 = RandomEngine() & 1 ? RandomEngine() : -RandomEngine();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doModulo(NumberObject2).doCompare(Number1 % Number2), 0);
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

            CHECK_EQ(NumberObject1.doMultiplication(NumberObject2).doCompare(Number1 * Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerMultiplication", [&] {
            NumberObject1.doMultiplication(NumberObject2);
        });
    }

    TEST_CASE("IntegerPower") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t NumberBase = (RandomEngine() & 1 ? RandomEngine() : -RandomEngine()) % 10000000, NumberExponent = RandomEngine() % 500;
            Integer NumberBaseObject(NumberBase), NumberExponentObject(NumberExponent), NumberResult(1);
            for (intmax_t NumberIteration = 0; NumberIteration < NumberExponent; ++NumberIteration)
                NumberResult = NumberResult.doMultiplication(NumberBaseObject);

            CHECK_EQ(NumberBaseObject.doPower(NumberExponentObject).doCompare(NumberResult), 0);
        }
        Integer NumberBaseObject(RandomEngine64()), NumberExponentObject(RandomEngine() % 1000);
        TestBench.run("IntegerPower", [&] {
            NumberBaseObject.doPower(NumberExponentObject);
        });
    }

    TEST_CASE("IntegerPrime") {
        for (unsigned NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            unsigned NumberTarget = RandomEngine() % 500000;
            CHECK_EQ(Mathematics::isPrime(NumberTarget), Mathematics::isPrimeNative(NumberTarget));
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

            CHECK_EQ(NumberObject1.doSubtraction(NumberObject2).doCompare(Number1 - Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerSubtraction", [&] {
            NumberObject1.doSubtraction(NumberObject2);
        });
    }
}

int main(int ParameterCount, char *ParameterList[]) {
    RandomEngine.seed(RandomDevice());
    RandomEngine64.seed(RandomEngine());
    return doctest::Context(ParameterCount, ParameterList).run();
}
