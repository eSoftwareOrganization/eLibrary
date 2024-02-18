#include <eLibrary.hpp>
using namespace eLibrary;
using namespace eLibrary::Core;

#undef max
#undef min

#define DOCTEST_CONFIG_IMPLEMENT
#include "3rd_party/doctest.h"

#include <random>

#define doGenerateSigned() (RandomEngine() & 1 ? RandomEngine() : -intmax_t(RandomEngine()))

::std::mt19937 RandomEngine;
::std::mt19937_64 RandomEngine64;
::std::random_device RandomDevice;

TEST_SUITE("Container") {
    int doAddGlobal(int Number1, int Number2) {
        return Number1 + Number2;
    }

    struct FunctionAdder {
        int AdderValue;

        constexpr FunctionAdder() : AdderValue(0) {}

        constexpr FunctionAdder(int AdderSource) : AdderValue(AdderSource) {}

        int doAdd(int Number1, int Number2) {
            return Number1 + Number2;
        }
    };

    TEST_CASE("Any") {
        Any AnyEmpty;
        CHECK_EQ(AnyEmpty.getValueType(), &typeid(void));
        CHECK_FALSE(AnyEmpty.hasValue());

        Any AnyInteger = Integer(1);
        CHECK(AnyInteger.hasValue());
        CHECK_EQ(AnyInteger.getValueType(), &typeid(Integer));
        CHECK_EQ(AnyInteger.getValue<Integer>().getValue<int>(), 1);
        CHECK_THROWS(AnyInteger.getValue<int>());
        AnyInteger = u"Any"_S;
        CHECK(AnyInteger.hasValue());
        CHECK_EQ(AnyInteger.getValueType(), &typeid(String));
        CHECK_EQ(AnyInteger.getValue<String>().toU8String(), "Any");
        CHECK_THROWS(AnyInteger.getValue<Integer>());
    }

    TEST_CASE("ArrayList") {
        ArrayList<NumberBuiltin<uintmax_t>> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
    }

    TEST_CASE("ArraySet") {
        ArraySet<NumberBuiltin<uintmax_t>> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine());
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("DoubleLinkedList") {
        DoubleLinkedList<NumberBuiltin<uintmax_t>> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
    }

    TEST_CASE("DoubleLinkedSet") {
        DoubleLinkedSet<NumberBuiltin<uintmax_t>> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine());
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }

    TEST_CASE("Function") {
        Function<int(int, int)> FunctionAddGlobal = doAddGlobal;
        CHECK_EQ(FunctionAddGlobal(1, 2), 3);
        Function<int(int, int)> FunctionAddLambda = [](int Number1, int Number2) {
            return Number1 + Number2;
        };
        CHECK_EQ(FunctionAddLambda(1, 2), 3);
    }

    TEST_CASE("Functions::doBind&doInvoke") {
        auto FunctionAdd = [](int Number1, int Number2) {
            return Number1 + Number2;
        };
        CHECK_EQ(Functions::doBind(FunctionAdd, 1, 2)(), 3);
        CHECK_EQ(Functions::doBind(FunctionAdd, Functions::PlaceHolder<1>(), Functions::PlaceHolder<2>())(1, 2), 3);
        FunctionAdder AdderObject;
        CHECK_EQ(Functions::doBind(&FunctionAdder::doAdd, &AdderObject, 1, 2)(), 3);
    }

    TEST_CASE("RedBlackTree") {
        RedBlackTree<NumberBuiltin<intmax_t>, nullptr_t> NumberTree;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberTree.doInsert(RandomEngine(), nullptr);
        ArrayList<NumberBuiltin<intmax_t>> NumberList;
        NumberTree.doOrder([&NumberList](const auto &NumberValue, nullptr_t){
            NumberList.addElement(NumberValue);
        });
        CHECK_LE(NumberTree.getSize(), 10000);
        CHECK_EQ(NumberList.getElementSize(), NumberTree.getSize());
        Collections::doTraverse(NumberList.begin(), NumberList.end(), [&NumberTree](const auto &NumberValue){
            CHECK(NumberTree.doSearch(NumberValue).hasValue());
            NumberTree.doRemove(NumberValue);
        });
        Collections::doTraverse(NumberList.begin(), NumberList.end(), [&NumberTree](const auto &NumberValue){
            CHECK_FALSE(NumberTree.doSearch(NumberValue).hasValue());
        });
        CHECK_EQ(NumberTree.getSize(), 0);
    }

    TEST_CASE("SingleLinkedList") {
        SingleLinkedList<NumberBuiltin<uintmax_t>> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1), RandomEngine());
        CHECK_EQ(NumberList.getElementSize(), 10000);
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.removeIndex(RandomEngine() % Objects::getMaximum(NumberList.getElementSize(), 1));
        CHECK(NumberList.isEmpty());
    }

    TEST_CASE("SingleLinkedSet") {
        SingleLinkedSet<NumberBuiltin<uintmax_t>> NumberList;
        for (uintmax_t NumberIndex = 0;NumberIndex < 10000;++NumberIndex)
            NumberList.addElement(RandomEngine());
        CHECK_LE(NumberList.getElementSize(), 10000);
        CHECK(NumberList.doDifference(NumberList).isEmpty());
        CHECK(NumberList.doIntersection(NumberList).doDifference(NumberList).isEmpty());
        CHECK(NumberList.doUnion(NumberList).doDifference(NumberList).isEmpty());
    }
}

TEST_SUITE("Concurrent") {
    TEST_CASE("AtomicInteger&ThreadExecutor") {
        AtomicStorage<int64_t> NumberSource(0);
        uintmax_t NumberValue = RandomEngine() % 10000;
        auto ThreadFunctionDecrement = [&]() {
            for (uintmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch)
                NumberSource.doFetchSub(1);
        };
        auto ThreadFunctionIncrement = [&]() {
            for (uintmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch)
                NumberSource.doFetchAdd(1);
        };
        ThreadExecutor ThreadExecutorObject(4);
        ThreadExecutorObject.doSubmit(ThreadFunctionIncrement).get();
        ThreadExecutorObject.doSubmit(ThreadFunctionIncrement).get();
        ThreadExecutorObject.doSubmit(ThreadFunctionIncrement).get();
        ThreadExecutorObject.doSubmit(ThreadFunctionIncrement).get();
        CHECK_EQ(NumberSource.getValue(), NumberValue * 4);
        ThreadExecutorObject.doSubmit(ThreadFunctionDecrement).get();
        ThreadExecutorObject.doSubmit(ThreadFunctionDecrement).get();
        ThreadExecutorObject.doSubmit(ThreadFunctionDecrement).get();
        ThreadExecutorObject.doSubmit(ThreadFunctionDecrement).get();
        CHECK_EQ(NumberSource.getValue(), 0);
    }

    TEST_CASE("Mutex&MutexLocker&ThreadExecutor") {
        uintmax_t NumberSource(0);
        uintmax_t NumberValue = RandomEngine() % 10000;
        Mutex NumberMutex;
        auto ThreadFunction = [&]() {
            for (uintmax_t NumberEpoch = 0; NumberEpoch < NumberValue; ++NumberEpoch) {
                MutexLocker NumberLocker(NumberMutex);
                ++NumberSource;
            }
        };
        ThreadExecutor ThreadExecutorObject(4);
        ThreadExecutorObject.doSubmit(ThreadFunction).get();
        ThreadExecutorObject.doSubmit(ThreadFunction).get();
        ThreadExecutorObject.doSubmit(ThreadFunction).get();
        ThreadExecutorObject.doSubmit(ThreadFunction).get();
        CHECK_EQ(NumberSource, NumberValue * 4);
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
        CHECK_EQ(NumberBuffer.doCompare(NumberBufferSource), 0);
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
    }

    TEST_CASE("IntegerComparison") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            if (Number1 > Number2) CHECK_GT(NumberObject1.doCompare(NumberObject2), 0);
            if (Number1 == Number2) CHECK_EQ(NumberObject1.doCompare(NumberObject2), 0);
            if (Number1 < Number2) CHECK_LT(NumberObject1.doCompare(NumberObject2), 0);
        }
    }

    TEST_CASE("IntegerDivision") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doDivision(NumberObject2).doCompare(Number1 / Number2), 0);
        }
    }

    TEST_CASE("IntegerFactorial") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            Integer NumberObject(RandomEngine() % 1000 + 1);
            CHECK_EQ(NumberObject.doFactorial().doCompare(NumberObject.doSubtraction(1).doFactorial().doMultiplication(NumberObject)), 0);
        }
    }

    TEST_CASE("IntegerModulo") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = doGenerateSigned(), Number2 = doGenerateSigned();
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doModulo(NumberObject2).doCompare(Number1 % Number2), 0);
        }
    }

    TEST_CASE("IntegerMultiplication") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            intmax_t Number1 = (doGenerateSigned()) % 10000, Number2 = (doGenerateSigned()) % 10000;
            Integer NumberObject1(Number1), NumberObject2(Number2);

            CHECK_EQ(NumberObject1.doMultiplication(NumberObject2).doCompare(Number1 * Number2), 0);
        }
    }

    TEST_CASE("IntegerPower") {
        for (uintmax_t NumberEpoch = 0; NumberEpoch < 10000; ++NumberEpoch) {
            uintmax_t NumberBase = RandomEngine() + 1, NumberExponent = RandomEngine() % 10000 + 1, NumberModulo = RandomEngine() + 1;
            CHECK_EQ(Integer(NumberBase).doPower(NumberExponent, NumberModulo).getValue<uintmax_t>(), Mathematics::doPower(NumberBase, NumberExponent, NumberModulo));
        }
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
    }
}

#if eLibraryFeature(Network)
TEST_SUITE("Socket") {
    TEST_CASE("StreamSocket") {
        doInitializeSocket();

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

        doDestroySocket();
    }
}
#endif

int main(int ParameterCount, char *ParameterList[]) {
    RandomEngine.seed(RandomDevice());
    RandomEngine64.seed(RandomEngine());
    return doctest::Context(ParameterCount, ParameterList).run();
}
