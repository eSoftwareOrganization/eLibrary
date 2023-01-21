# eLibrary

![](Scroll.png)

![](https://img.shields.io/badge/License-LGPL%202.1-red.svg)

Project Author: ldgeng(ldgeng@outlook.com)

Project Community: [Discord](https://discord.gg/uvktZHHX)

Project Destination: An all-in-one cpp library

Project Repository: [GitHub](https://github.com/ldgeng/eLibrary)

Project Source:
+ [Core::Integer](https://blog.csdn.net/code4101/article/details/38705155) @code4101
+ [Core::Mathematics](https://www.cnblogs.com/CocoonFan/p/3164221.html) @CocoonFan
+ [Core::Mathematics](https://github.com/sympy/sympy) @sympy
+ [IO::AudioSegment](https://github.com/jiaaro/pydub) @Jiaaro
+ [Benchmark](https://github.com/martinus/nanobench)
+ [Test](https://github.com/doctest/doctest)

Project Subproject:
+ [eLibraryDriver](https://github.com/ldgeng/eLibraryDriver)

Project Update Log:
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
- std::greater\<Comparable\>
- std::greater_equal\<Comparable\>
- std::hash\<Hashable\>
- std::not_equal_to\<Comparable\>
>eLibrary V2023.01
- Core::NtDriver doLoadSC(NtServiceManager::doCreateService Implementation)
- Core::NtService getServiceErrorControl / getServiceStartType / getServiceState / getServiceType / setServiceErrorControl / setServiceStartType / setServiceType(New)
- Core::NtServiceManager doCreateService(NtService Implementation)
- std::less\<String\>
- std::greater\<String\>
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
