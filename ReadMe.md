# eLibrary
Project Name: eLibrary

Project Destination: Lite Version of Java / Python

Project Compatibility(C++ Standard):
  + C++98(All Compatible)
  + C++03(All Compatible)
  + C++11(Partial Compatible)
  + C++14(All Compatible)
  + C++17(All Compatible)
  + C++20(Incompatible)

Project Author: ldgeng(ldgeng@outlook.com)

Project License: GPL v3.0

Project Repository: [GitHub](https://github.com/ldgeng/eLibrary)

Project Source:
+ [Core::Integer](https://blog.csdn.net/code4101/article/details/38705155)

Project Subproject:
+ eLibraryDriver(Compatibility Core::WindowsNT KeLoadDriver KeReadProcessMemory KeTerminateProcess KeWriteProcessMemory KeUnloadDriver)

Project Update Log:

eLibrary V2022.14
- Core::DoubleLinkedList begin / Constructor(std::initializer_list) / end / LinkedListIterator(new)
- Core::Fraction Constructor / doAddition / doCompare / doDivision / doMultiplication / doSubtraction(Signature) getAbsolute / getOpposite(New) getValue(Deprecated New) isNegative / isPositive(New) toString(Signature)
- Core::Integer getValue(Deprecated) isNegative / isPositive(New)
- Core::KeDriver / Constructor(const String&, const String&, DWORD, DWORD) / doLoadNt / doLoadSC / doUnloadNt / doUnloadSC(New)
- Core::Mathematics doEvolution / doExponent / doInverseHyperbolicTangent / doLogarithmE / doPower(New)
- Core::SingleLinkedList begin / Constructor(std::initializer_list) / end / LinkedListIterator(New)
- Core::WindowsNT loadDriver(Remove) loadDriverSC(Remove) unloadDriver(Remove) unloadDriverSC(Remove)

eLibrary V2022.13
- Core::Fraction Constructor(Simplify) doDivision / doMultiplication / doSubtraction / getDenominator / getNumerator / toString(New)
- Core::Integer doDivision / doModulo / doMultiplication / doSubtraction / getValue(Signature) toString(Format)
- Core::Mathematics / getGreatestCommonFactor(New)
- Core::RedBlackTree doInsert / doSearchCore(Compare) doRemove(Exception)
- Core::WindowsNT loadDriver / unloadDriver(Remove Ntdll Linking Requirements) loadDriverSC(...)

![](Nahida.png)
By @eunip
