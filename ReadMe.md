# eLibrary

![](Scroll.png)

![](https://img.shields.io/badge/License-Apache%202.0-red.svg)

Project Community:
+ [Discord](https://discord.gg/ejYKQZpxkx)

Project Dependencies:
+ [doctest](https://github.com/doctest/doctest)
+ [nanobench](https://github.com/martinus/nanobench)

Project Description: An all-in-one cpp library

Project Reference:
+ [pydub](https://github.com/jiaaro/pydub)
+ [sympy](https://github.com/sympy/sympy)
+ [\<Unnamed>](https://blog.csdn.net/code4101/article/details/38705155)
+ [\<Unnamed>](https://www.cnblogs.com/CocoonFan/p/3164221.html)

Project Update Log:
> eLibrary V2023.04
- Core::ArrayList ~Constructor(...) doAssign / operator= / toSTLVector(New) doConcat(Memory Overflow Fix) Constructor(std::array\<>) / doReverse(New) indexOf(From doFind) toString(StringStream Implementation)
- Core::ConcurrentArrayList Constructor(std::array\<E, ElementSourceSize>) / doConcat / doReverse(New) indexOf(From doFind)
- Core::DoubleLinkedList begin / end / LinkedListIterator(Remove) indexOf(From doFindElement + Modifier + Segment Fault Fix) toString(StringStream Implementation)
- Core::Fraction getValue(Overflow Protection)
- Core::Integer Constructor(T)(std::numeric_limits<intmax_t>::min()) doCompare(Signature) isNegative / isPositive(0) isEven / isOdd / NumberBaseUnit(New) getValue(Overflow Protection) toString(StringStream Implementation)
- Core::Mathematics doCosineFraction / doExponentFraction / doInverseHyperbolicTangentFraction(Parameter`NumberPrecision`) getAbsolute(New) isPrimeLucas(Implementation) isPrimeRabinMiller(Integer::isEven Implementation)
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
> eLibrary V2023.03
- Core::ArrayList begin / end(Remove) doFind(From doFindElement)
- Core::ArrayListIterator(Remove)
- Core::ConcurrentArrayList / addElement / doClear / doFind / getElement / getElementSize / removeElement / removeIndex / setElement / toArray / toString(New)
> eLibrary V2023.02
- Core::ArrayList doConcat / toArray(New) toString(...)
- Core::Comparable(New)
- Core::DoubleLinkedList toString(...)
- Core::Fraction Constructor(const Integer&) doCompare(±0)
- Core::getSummationAVX(Remove)
- Core::Hashable(New)
- Core::Integer Constructor() / Constructor(T)(From Constructor(intmax_t)) Constructor(const String&, unsigned short)(...) doCompare(±0) doPower / toString(unsigned short)(New)
- Core::Mathematics doCosine / doEvolution / doExponent / doHyperbolicCosine / doHyperbolicSine / doHyperbolicTangent / doInverseHyperbolicTangent / doLogarithmE / doPower / doSine / doTangent / getNearestPower2Lower / getNearestPower2Upper / toDegrees / toRadians(std::enable_if => requires) doCosineFraction / doEvolutionFraction / doExponentFraction / doInverseHyperbolicTangentFraction / doLogarithmEFraction / doHyperbolicCosineFraction / doHyperbolicSineFraction / doHyperbolicTangentFraction / doPowerFraction / getTrailingZeroCount / isPrime / isPrimeLucas(Not Implemented) / isPrimeNative / isPrimeRabinMiller(New)
- Core::NtDriver doCancel(Remove)
- Core::NtFile / doCreate(New)
- Core::NtFileAccess(New)
- Core::NtFileAttribute(New)
- Core::NtFileDisposition(New)
- Core::NtFileOption(New)
- Core::NtFileShare(New)
- Core::NtProcess / doOpen / doTerminate(New)
- Core::NtService addDependency / getServiceName / getServicePath / setServicePath / updateServiceConfiguration(New) Constructor / getServiceErrorControl / setServiceErrorControl(Parameter & Return)
- Core::NtServiceErrorControl(New)
- Core::NtServiceManager doOpenService(...)
- Core::NtServiceStartType(New)
- Core::Object toString(New)
- Core::RedBlackTree / doRemove(...)
- Core::SingleLinkedList toString(...)
- Core::String begin / Constructor(char16_t*, intmax_t) / Constructor(char16_t*, intmax_t, bool) / end(Remove) doConcat(char16_t) / doConcat(const String&) / doReplace / doStrip / doTruncate / toLowerCase / toUpperCase(std::basic_stringstream<char16_t> Implementation) valueOf(const T&)(New) valueOf(T, unsigned short)(std::enable_if => requires)
- Core::StringIterator(Remove)
- IO::AudioSegment / doOpenWAV(New)
- IO::FastBufferedInputStream doInput(From operator>> & operator()) operator bool(Remove)
- IO::FastBufferedOutputStream doOutput(From operator<< & operator())
- IO::InputStream doRead(Modifier) doSkip(...)
- IO::SocketInetAddress getSocketIP / getSocketPort(New)
- std::equal_to\<Comparable>
- std::less\<Comparable>
- std::less_equal\<Comparable>
- std::greater\<Comparable>
- std::greater_equal\<Comparable>
- std::hash\<Hashable>
- std::not_equal_to\<Comparable>
>eLibrary V2023.01
- Core::NtDriver doLoadSC(NtServiceManager::doCreateService Implementation)
- Core::NtService getServiceErrorControl / getServiceStartType / getServiceState / getServiceType / setServiceErrorControl / setServiceStartType / setServiceType(New)
- Core::NtServiceManager doCreateService(NtService Implementation)
- std::less\<String>
- std::greater\<String>
>eLibrary V2022.16
- Core::NtDriver(KeDriver) doLoadNt / doUnloadNt (NtModule Implementation) doLoadSC / doUnloadSC(NtService / NtServiceManager Implementation) doRead / doWrite(New) doLoadSC / doUnloadNt / doUnloadSC(Modifier)
- Core::NtModule / getFunction(New)
- Core::NtService / doControl / doDelete / doStart(New)
- Core::NtServiceManager / doCreateService / doOpenService(New)
- Core::String Constructor(const std::wstring&) (New)
>eLibrary V2022.15
- Core::getSummationAVX(const double*, uintmax_t)(New) Core::getSummation(Rename => Core::getSummationAVX)
- Core::KeDriver doCancel / doClose / doControl / doOpen(New)
- Core::Mathematics doCosine / doHyperbolicCosine / doHyperbolicSine / doHyperbolicTangent / doSine / doTangent / getNearestPower2Lower / getNearestPower2Upper / toDegrees / toRadians(New) doEvolution / doExponent / doInverseHyperbolicTangent / doLogarithmE / doPower(Type Check)
>eLibrary V2022.14
- Core::DoubleLinkedList begin / Constructor(std::initializer_list) / end / LinkedListIterator(new)
- Core::Fraction Constructor / doAddition / doCompare / doDivision / doMultiplication / doSubtraction(Signature) getAbsolute / getOpposite(New) getValue(Deprecated New) isNegative / isPositive(New) toString(Signature)
- Core::Integer getValue(Deprecated) isNegative / isPositive(New)
- Core::KeDriver / Constructor(const String&, const String&, DWORD, DWORD) / doLoadNt / doLoadSC / doUnloadNt / doUnloadSC(New)
- Core::Mathematics doEvolution / doExponent / doInverseHyperbolicTangent / doLogarithmE / doPower(New)
- Core::SingleLinkedList begin / Constructor(std::initializer_list) / end / LinkedListIterator(New)
- Core::WindowsNT loadDriver(Remove) loadDriverSC(Remove) unloadDriver(Remove) unloadDriverSC(Remove)
>eLibrary V2022.13
- Core::Fraction Constructor(Simplify) doDivision / doMultiplication / doSubtraction / getDenominator / getNumerator / toString(New)
- Core::Integer doDivision / doModulo / doMultiplication / doSubtraction / getValue(Signature) toString(Format)
- Core::Mathematics / getGreatestCommonFactor(New)
- Core::RedBlackTree doInsert / doSearchCore(Compare) doRemove(Exception)
- Core::WindowsNT loadDriver / unloadDriver(Remove Ntdll Linking Requirements) loadDriverSC(...)
