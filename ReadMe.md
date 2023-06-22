# eLibrary
> An all-in-one cpp library

![](https://img.shields.io/github/actions/workflow/status/eSoftwareOrganization/eLibrary/ci.yml)
![](https://ci.appveyor.com/api/projects/status/fn5qnydylfm5xe93?svg=true)
![](https://img.shields.io/github/license/eSoftwareOrganization/eLibrary)
![](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fldgeng%2FeLibrary.svg?type=shield)
![](https://img.shields.io/github/repo-size/eSoftwareOrganization/eLibrary)
![](https://img.shields.io/tokei/lines/github/eSoftwareOrganization/eLibrary)

Project Credits:

This repository draws from some preexisting work. Credits to their authors.
+ [audioop(Python)](https://docs.python.org/3/library/audioop.html)

This project implements the audio processing api

+ [pydub(Python)](https://github.com/jiaaro/pydub)

This project implements the audio processing api

+ [sympy(Python)](https://github.com/sympy/sympy)

This project implements the Baillie-PSW Primality Test Algorithm
+ [\<Unnamed>](https://blog.csdn.net/code4101/article/details/38705155)

This project implements the large integer structure
+ [\<Unnamed>](https://www.cnblogs.com/CocoonFan/p/3164221.html)

This project implements parts of the mathematical algorithms

Project Dependencies:
+ [doctest](https://github.com/doctest/doctest)
+ [ffmpeg](https://ffmpeg.org)
+ [nanobench](https://github.com/martinus/nanobench)
+ [OpenAL Soft](https://github.com/kcat/openal-soft)

Project Repository: [GitHub](https://github.com/eSoftwareOrganization/eLibrary)

Project Update Log:
> eLibrary V2023.09
- Core::ArrayList addElement / doAssign / doConcat / doReverse / removeIndex(std::copy Implementation) indexOf / isContains(Comparison)
- Core::ArraySet addElement / removeElement(std::copy Implementation) getElementSize / isEmpty / toArrayList(New) isContains(Comparison)
- Core::DoubleLinkedList addElement / removeIndex(Segmentation Fault) / indexOf(Comparison & Modifier) getElementSize / isContains / isEmpty(New) removeElement(...)
- Core::DoubleLinkedSet / addElement / doDifference / doIntersection / doUnion / getElementSize / isContains / isEmpty / removeElement / toDoubleLinkedSet(New)
- Core::Integer doDivision / doModulo / doSubtraction(Signature) doPower(0⁰)
- Core::Object isEqual(Remove)
- Core::RedBlackTree NodeColorEnumeration(...)
- Core::SingleLinkedList indexOf(Comparison & Modifier) getElementSize / isContains / isEmpty(New)
- Core::SingleLinkedSet / addElement / doDifference / doIntersection / doUnion / getElementSize / isContains / isEmpty / removeElement / toSingleLinkedList(New)
- Core::String Constructor / ~Constructor / doAssign(...)
- Core::StringStream(...)
- IO::AudioBuffer getBufferIndex / setBufferData(New)
- IO::AudioSegment / doAssign / doOpenWAV / doSplitChannel / doSynchronize / getBitSample / getChannelCount / getSampleRate / setBitSample / setChannelCount / setSampleRate / toAudioBuffer(New) doExport / doExportWAV / doOpen(Experimental / New)
- IO::AudioSource / doPause / doPlay / doRewind / doStop / setAudioBuffer / setAudioGain / setAudioLoop / setAudioPitch(New)
- IO::FileInputStream Constructor / doSeek / getFileLength / getFilePosition(...)
- IO::FileOutputStream Constructor / doSeek / doTruncate / getFilePosition(...)
- IO::MediaCodec / doFindDecoder / doFindEncoder / getCodecObject(New)
- IO::MediaCodecContext / doAllocate / getChannelCount / getContextObject / getSampleRate(New)
- IO::MediaFormatContext / doFindBestStream / doFindStreamInformation / doOpen / getContextObject / setIOContext / setOutputFormat(New)
- IO::MediaFrame / doAllocate / getFrameObject(New)
- IO::MediaIOContext / doOpen / getContextObject(New)
- IO::MediaPacket / doAllocate / getPacketObject(New)
> eLibrary V2023.08
- Core::ArrayList doClear(...)
- Core::ArraySet / addElement / doClear / doDifference / doIntersection / doUnion / removeElement(New)
- Core::Fraction doPower(New) getValue(...)
- Core::Integer doPower(NumberExponentSource < 0) getValue(...)
- Core::NtFile doRead / doWrite(New)
- Core::NtProcess doCreate / doResume / doSuspend(New)
- Network::NetworkException(New)
- Network::Socket doReceive / doSend(Remove) getInputStream / getOutputStream(Experimental / New)
- Network::SocketInputStream / doRead / getInstance / isAvailable(Experimental / New)
- Network::SocketOutputStream / doClose / doWrite / getInstance / isAvailable(Experimental / New)
> eLibrary V2023.07
- Core::ArithmeticException(New)
- Core::Fraction doAddition / doSubtraction(isNegative / isPositive Implementation) toString(0)
- Core::Integer toString(0)
- Core::Mathematics doCosecant / doCosecantFraction / doCotangent / doCotangentFraction / doSecant / doSecantFraction / doSineFraction / doTangentFraction(New)
- Core::MathematicsContext getFunctionPrecision / setFunctionPrecision(New)
- IO::FileInputStream / doClose / doRead / doSeek / isAvailable / getFileLength / getFilePosition(New)
- IO::FileOutputStream / doClose / doFlush / doSeek / doTruncate / doWrite / getFilePosition(New)
- IO::InputStream doRead(byte*, int, int)(New)
- IO::IOException(New)
- IO::OutputStream doWrite(byte*, int, int)(New)
- IO::URLConnection(Remove)
- Network::DatagramSocket / doClose / doReceive / doSend / isClosed / setBroadcast(New)
- Network::NetworkAddress / isAnyLocalAddress / isLinkLocalAddress / isLoopbackAddress / isMulticastAddress / isSiteLocalAddress / NetworkAddressProtocol(New)
- Network::NetworkSocketAddress(IO::SocketInetAddress) getAddressInformation(Remove) getSocketAddress(From getSocketIP) Constructor / getSocketIP(...)
- Network::StreamSocket(IO::Socket) doClose / doConnect(...) doReceive / doSend / getRemoteSocketAddress / isClosed / isConnected / setAddressReuse(New)
- Network::StreamSocketServer(IO::SocketServer) isBound / isClosed(New) setAddressReuse(From setConnectionReuse)
> eLibrary V2023.06
- Core::AbstractQueuedSynchronizer / AbstractQueuedNode / tryAcquireExclusive / tryAcquireShared / tryReleaseExclusive / tryReleaseShared(New)
- Core::ArrayList toSTLArray(From toArray)
- Core::AtomicNumber / addAndGet / compareAndSet / decrementAndGet / getAndAdd / getAndDecrement / getAndIncrement / getAndSet / getValue / incrementAndGet(New)
- Core::ConcurrentArrayList(Remove)
- Core::ConcurrentUtility / doCompareAndExchange / doCompareAndExchangeReference / doCompareAndSet / doCompareAndSetReference / getAndAddNumber / getAndSetNumber(New)
- Core::ConditionVariable(Remove)
- Core::Mutex(Remove)
- Core::MutexExecutor(Remove)
- Core::Semaphore(Remove)
- Core::String ~Constructor(StringReference)
- IO::AudioSegment(Remove)
- IO::Buffer getBufferLimit / getBufferMark / getBufferPosition / getRemaining / hasRemaining / setBufferLimit / setBufferPosition(v2023.05)
- IO::ByteBuffer Constructor(unsigned) / Constructor(const ByteBuffer&) / ~Constructor / doCompact(New)
> eLibrary V2023.05
- Core::ArrayList doReverse(...) removeElement(...)
- Core::ConcurrentArrayList doAssign / operator= / toSTLVector(New)
- Core::ConditionVariable / doWait / getHandle / notifyAll / notifyOne / getHandle(New)
- Core::DoubleLinkedList toArrayList / toSTLList(New)
- Core::Fraction toString(StringStream Implementation)
- Core::Integer doDivision / doModulo(0 Dividing Handling) doFactorial(New)
- Core::Mathematics doCombinator(New)
- Core::Mutex / doLock / doUnlock / getHandle / tryLock(New)
- Core::MutexExecutor / doExecute / doExecuteVoid(New)
- Core::NtDriver Constructor / ~Constructor / doClose / doControl / doOpen / doRead / doUnloadSC / doWrite(Remove) doLoadNt / doUnloadNt(Parameter & Static)
- Core::Object toString(StringStream Implementation)
- Core::Semaphore / doAcquire / doRelease / getHandle(New)
- Core::SingleLinkedList indexOf(From doFind) removeElement(...) toArrayList / toDoubleLinkedList / toSTLForwardList(New)
- IO::Buffer / doClear / doDiscardMark / doFlip / doMark / doReset / doRewind / getBufferCapacity(New)
> eLibrary V2023.04
- Core::ArrayList ~Constructor(...) doAssign / operator= / toSTLVector(New) doConcat(Memory Overflow Fix) Constructor(std::array\<>) / doReverse(New) indexOf(From doFind) toString(StringStream Implementation)
- Core::ConcurrentArrayList Constructor(std::array\<E, ElementSourceSize>) / doConcat / doReverse(New) indexOf(From doFind)
- Core::DoubleLinkedList begin / end / LinkedListIterator(Remove) indexOf(From doFindElement + Modifier + Segment Fault Fix) toString(StringStream Implementation)
- Core::Fraction getValue(Overflow Protection)
- Core::Integer Constructor(T)(std::numeric_limits<intmax_t>::min()) doCompare(Signature) isNegative / isPositive(0) isEven / isOdd / NumberBaseUnit(New) getValue(Overflow Protection) toString(StringStream Implementation)
- Core::Mathematics doCosineFraction / doExponentFraction / doInverseHyperbolicTangentFraction(Parameter `NumberPrecision`) getAbsolute(New) isPrimeLucas(Implementation) isPrimeRabinMiller(Integer::isEven Implementation)
- Core::Object ~Constructor(New)
- Core::SingleLinkedList begin / end / LinkedListIterator(Remove) doFind(From doFindElement + Segment Fault Fix) toString(StringStream Implementation)
- Core::String Constructor(char16_t)(New) doConcat / doReplace / doStrip / doTruncate / toLowerCase / toUpperCase(StringStream Implementation) doReplace(Modifier) doReverse(New) toWString(...) valueOf(T, \[unsigned short])(Remove)
- Core::StringStream addCharacter / addString / doClear() / toString(New)
- Core::NtDriver Constructor(Parameter)
- Core::NtFile NtFileAccess / NtFileAttribute / NtFileDisposition / NtFileOption / NtFileShare(From Core::*)
- Core::NtService NtServiceErrorControl / NtServiceStartType(From Core::*) NtServiceType(New)
- Core::NtServiceManager doCreateService
- IO::AudioSegment doOpenWAV(Remove)
- IO::FastBufferedInputStream(Remove)
- IO::FastBufferedOutputStream(Remove)
- IO::InputStream doRead(byte[], int, int) / doSkip(Remove)
- IO::IOException(Remove)
- IO::OutputStream doWrite(byte[], int, int)(Remove)
- IO::SocketInetAddress Constructor(...)
