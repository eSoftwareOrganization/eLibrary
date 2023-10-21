#include <eLibrary.hpp>
using namespace eLibrary;
using namespace eLibrary::Core;

#undef max
#undef min

#define DOCTEST_CONFIG_IMPLEMENT
#include "3rd_party/doctest.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#include "3rd_party/nanobench.h"

#include <random>

#define doGenerateSigned() (RandomEngine() & 1 ? RandomEngine() : -intmax_t(RandomEngine()))

ankerl::nanobench::Bench TestBench;
::std::mt19937 RandomEngine;
::std::mt19937_64 RandomEngine64;
::std::random_device RandomDevice;

TEST_SUITE("Container") {
    TEST_CASE("ArrayList") {
        ArrayList<Integer> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
        NumberList.addElement(RandomEngine64());
        TestBench.run("ArrayList(Append)", [&]{
            NumberList.addElement(RandomEngine() % NumberList.getElementSize(), RandomEngine64());
        });
        TestBench.run("ArrayList(Remove)", [&]{
            if (!NumberList.isEmpty()) NumberList.removeIndex(RandomEngine() % NumberList.getElementSize());
        });
    }

    TEST_CASE("ArraySet") {
        ArraySet<Integer> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("DoubleLinkedList") {
        DoubleLinkedList<uintmax_t> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
        NumberList.addElement(RandomEngine64());
        TestBench.run("DoubleLinkedList(Append)", [&]{
            NumberList.addElement(RandomEngine() % NumberList.getElementSize(), RandomEngine64());
        }).run("DoubleLinkedList(Remove)", [&]{
            if (!NumberList.isEmpty()) NumberList.removeIndex(RandomEngine() % NumberList.getElementSize());
        });
    }

    TEST_CASE("DoubleLinkedSet") {
        DoubleLinkedSet<Integer> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("SingleLinkedList") {
        SingleLinkedList<uintmax_t> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
        NumberList.addElement(RandomEngine64());
        TestBench.run("SingleLinkedList(Append)", [&]{
            NumberList.addElement(RandomEngine() % NumberList.getElementSize(), RandomEngine64());
        }).run("SingleLinkedList(Remove)", [&]{
            if (!NumberList.isEmpty()) NumberList.removeIndex(RandomEngine() % NumberList.getElementSize());
        });
    }

    TEST_CASE("SingleLinkedSet") {
        SingleLinkedSet<Integer> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement({RandomEngine()});
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("SingleLinkedStack") {
        SingleLinkedStack<uintmax_t> NumberStack;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberStack.addElement(NumberStack.getElementSize());
        CHECK_EQ(NumberStack.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex) {
            CHECK_EQ(NumberStack.getElement() + 1, NumberStack.getElementSize());
            NumberStack.removeElement();
        }
        CHECK(NumberStack.isEmpty());
    }
}

TEST_SUITE("Concurrent") {
    TEST_CASE("AtomicInteger") {
        AtomicNumber<int64_t> NumberSource(0);
        uintmax_t NumberValue = RandomEngine() % 50000;
        auto ThreadFunctionDecrement = [&]() {
            for (uintmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch)
                NumberSource.getAndDecrement();
        };
        auto ThreadFunctionIncrement = [&]() {
            for (uintmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch)
                NumberSource.getAndIncrement();
        };
        ReentrantFunctionThread ThreadIncrement1(ThreadFunctionIncrement), ThreadIncrement2(ThreadFunctionIncrement);
        ReentrantFunctionThread ThreadDecrement1(ThreadFunctionDecrement), ThreadDecrement2(ThreadFunctionDecrement);
        for (uintmax_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch) {
            NumberValue = RandomEngine() % 100000;
            ThreadIncrement1.doStart();
            ThreadIncrement2.doStart();
            ThreadIncrement1.doJoin();
            ThreadIncrement2.doJoin();
            CHECK_EQ(NumberSource.getValue(), NumberValue * 2);
            ThreadDecrement1.doStart();
            ThreadDecrement2.doStart();
            ThreadDecrement1.doJoin();
            ThreadDecrement2.doJoin();
            CHECK_EQ(NumberSource.getValue(), 0);
        }
    }
}

#if eLibraryFeature(IO)
TEST_SUITE("IO") {
    TEST_CASE("File&FileInputStream&FileOutputStream") {
        IO::FileOutputStream StreamOutput;
        StreamOutput.doOpen({u"FileStream.tst"}, IO::FileOption::OptionBinary, IO::FileOption::OptionCreate);
        IO::ByteBuffer NumberBufferSource(IO::ByteBuffer::doAllocate(10000));
        for (uintmax_t NumberEpoch = 0;NumberEpoch < 10000;++NumberEpoch)
            NumberBufferSource.setValue(RandomEngine() % 256);
        NumberBufferSource.doFlip();
        StreamOutput.doWrite(NumberBufferSource);
        StreamOutput.doClose();
        IO::FileInputStream StreamInput;
        StreamInput.doOpen({u"FileStream.tst"}, IO::FileOption::OptionBinary);
        IO::ByteBuffer NumberBuffer(IO::ByteBuffer::doAllocate(10000));
        StreamInput.doRead(NumberBuffer);
        CHECK_EQ(::memcmp(NumberBuffer.getBufferContainer(), NumberBufferSource.getBufferContainer(), 10000), 0);
        StreamInput.doClose();
        IO::File FileObject(IO::File({u"FileStream.tst"}));
        CHECK(!FileObject.isDirectory());
        CHECK(FileObject.isExists());
        CHECK(FileObject.isFile());
        CHECK_EQ(FileObject.getFileSize(), 10000);
    }
}
#endif

#if eLibraryFeature(Multimedia)
TEST_SUITE("Multimedia") {
    TEST_CASE("AudioSegment") {
        doInitializeOpenAL()
        doDestroyOpenAL()
    }
}
#endif

TEST_SUITE("Number") {
    TEST_CASE("IntegerAddition") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doAddition(NumberObject2).doCompare(Number1 + Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerAddition", [&] {
            NumberObject1.doAddition(NumberObject2);
        });
    }

    TEST_CASE("IntegerComparison") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
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
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doDivision(NumberObject2).doCompare(Number1 / Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerDivision", [&] {
            NumberObject1.doDivision(NumberObject2);
        });
    }

    TEST_CASE("IntegerFactorial") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            Integer NumberObject(RandomEngine() % 1000 + 1);
            CHECK_EQ(NumberObject.doFactorial().doCompare(NumberObject.doSubtraction(1).doFactorial().doMultiplication(NumberObject)), 0);
        }
        Integer NumberObject(RandomEngine() % 10000);
        TestBench.run("IntegerFactorial", [&] {
            NumberObject.doFactorial();
        });
    }

    TEST_CASE("IntegerModulo") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doModulo(NumberObject2).doCompare(Number1 % Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerModulo", [&] {
            NumberObject1.doModulo(NumberObject2);
        });
    }

    TEST_CASE("IntegerMultiplication") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = (doGenerateSigned()) % 10000, Number2 = (doGenerateSigned()) % 10000;
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doMultiplication(NumberObject2).doCompare(Number1 * Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerMultiplication", [&] {
            NumberObject1.doMultiplication(NumberObject2);
        });
    }

    TEST_CASE("IntegerPower") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            uintmax_t NumberBase = RandomEngine() + 1, NumberExponent = RandomEngine() % 10000 + 1, NumberModulo = RandomEngine() + 1;
            CHECK_EQ(Integer(NumberBase).doPower(NumberExponent, NumberModulo).getValue<uintmax_t>(), Mathematics::doPower(NumberBase, NumberExponent, NumberModulo));
        }
        Integer NumberBaseObject(RandomEngine64()), NumberExponentObject(RandomEngine() % 10000 + 1);
        TestBench.run("IntegerPower", [&] {
            NumberBaseObject.doPower(NumberExponentObject);
        });
    }

    TEST_CASE("IntegerPrime") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            auto NumberTarget = RandomEngine();
            CHECK_EQ(Mathematics::isPrime(NumberTarget), Mathematics::isPrimeNative(NumberTarget));
        }
    }

    TEST_CASE("IntegerSubtraction") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doSubtraction(NumberObject2).doCompare(Number1 - Number2), 0);
        }
        Integer NumberObject1(RandomEngine64()), NumberObject2(RandomEngine64());
        TestBench.run("IntegerSubtraction", [&] {
            NumberObject1.doSubtraction(NumberObject2);
        });
    }
}

#if eLibraryFeature(Network)
TEST_SUITE("Socket") {
    TEST_CASE("StreamSocket") {
        doInitializeSocket()

        Network::StreamSocket SocketClient(Network::NetworkSocketAddress(Network::NetworkAddress(Array<uint8_t>{127, 0, 0, 1}), 26916));
        Network::StreamSocketServer SocketServer(Network::NetworkSocketAddress(Network::NetworkAddress(Array<uint8_t>{127, 0, 0, 1}), 26916));
        SocketServer.doBind();
        SocketServer.doListen();
        uint8_t SocketCharacter1 = RandomEngine() % 256, SocketCharacter2 = RandomEngine() % 256;
        FunctionThread SocketServerThread([&] {
            auto SocketTarget = SocketServer.doAccept();
            Network::SocketOutputStream::getInstance(SocketTarget).doWrite(SocketCharacter1);
            CHECK_EQ((uint8_t) Network::SocketInputStream::getInstance(SocketTarget).doRead(), SocketCharacter2);
        });
        SocketServerThread.doStart();
        SocketClient.doConnect();
        CHECK_EQ((uint8_t) Network::SocketInputStream::getInstance(SocketClient).doRead(), SocketCharacter1);
        Network::SocketOutputStream::getInstance(SocketClient).doWrite(SocketCharacter2);
        SocketServerThread.doJoin();
        SocketClient.doClose();
        SocketServer.doClose();

        doDestroySocket()
    }
}
#endif

int main(int ParameterCount, char *ParameterList[]) {
    doInitializeCore();

    RandomEngine.seed(RandomDevice());
    RandomEngine64.seed(RandomEngine());
    return doctest::Context(ParameterCount, ParameterList).run();
}
