#pragma once

#ifdef eLibraryCore_WindowsNT

#include <Windows.h>
#include <ntdef.h>
#include <ntstatus.h>
#include <shlwapi.h>
#include <string>
#include <sstream>

namespace eLibrary {
    typedef NTSTATUS NTAPI (*NtCloseType)(IN HANDLE Handle);

    typedef NTSTATUS NTAPI (*NtCreateKeyType)(OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, ULONG TitleIndex, IN OPTIONAL PUNICODE_STRING Class, IN ULONG CreateOptions, OUT OPTIONAL PULONG Disposition);

    typedef NTSTATUS NTAPI (*NtLoadDriverType)(IN PUNICODE_STRING DriverServiceName);

    typedef NTSTATUS NTAPI (*NtSetValueKeyType)(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN OPTIONAL ULONG TitleIndex, IN ULONG Type, IN OPTIONAL PVOID Data, IN ULONG DataSize);

    typedef NTSTATUS NTAPI (*NtUnloadDriverType)(IN PUNICODE_STRING DriverServiceName);

    typedef NTSTATUS NTAPI (*RtlAdjustPrivilegeType)(IN ULONG Privilege, IN BOOLEAN Enable, IN BOOLEAN CurrentThread, OUT PBOOLEAN Enabled OPTIONAL);

    typedef NTSTATUS NTAPI (*RtlInitUnicodeStringType)(OUT PUNICODE_STRING DestinationString, IN PCWSTR SourceString);

#define SeLoadDriverPrivilege 0xa

    class KeDriver final {
    private:
        DWORD DriverErrorControl;
        String DriverFilePath;
        String DriverServiceName;
        DWORD DriverStartType;
    public:
        KeDriver(const String &FilePath, const String &Name, DWORD ErrorControl, DWORD StartType) noexcept : DriverErrorControl(ErrorControl), DriverFilePath(FilePath), DriverServiceName(Name), DriverStartType(StartType) {}

        void doLoadNt() {
            HMODULE ModuleNtDll = LoadLibraryW(L"ntdll.dll");
            NtCloseType NtClose = (NtCloseType) GetProcAddress(ModuleNtDll, "NtClose");
            NtCreateKeyType NtCreateKey = (NtCreateKeyType) GetProcAddress(ModuleNtDll, "NtCreateKey");
            NtLoadDriverType NtLoadDriver = (NtLoadDriverType) GetProcAddress(ModuleNtDll, "NtLoadDriver");
            NtSetValueKeyType NtSetValueKey = (NtSetValueKeyType) GetProcAddress(ModuleNtDll, "NtSetValueKey");
            RtlAdjustPrivilegeType RtlAdjustPrivilege = (RtlAdjustPrivilegeType) GetProcAddress(ModuleNtDll, "RtlAdjustPrivilege");
            RtlInitUnicodeStringType RtlInitUnicodeString = (RtlInitUnicodeStringType) GetProcAddress(ModuleNtDll, "RtlInitUnicodeString");

            std::basic_stringstream<wchar_t> DriverImagePathStream;
            DriverImagePathStream << L"\\??\\" << DriverFilePath.toWString();

            std::basic_stringstream<wchar_t> DriverRegistrationPathStream;
            DriverRegistrationPathStream << L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" << DriverServiceName.toWString();

            UNICODE_STRING DriverRegistrationPath;
            RtlInitUnicodeString(&DriverRegistrationPath, DriverRegistrationPathStream.str().c_str());

            OBJECT_ATTRIBUTES DriverRegistrationAttribute;
            InitializeObjectAttributes(&DriverRegistrationAttribute, &DriverRegistrationPath, OBJ_CASE_INSENSITIVE, nullptr, nullptr)

            DWORD DriverServiceType = SERVICE_KERNEL_DRIVER;

            UNICODE_STRING DriverErrorControlString, DriverImagePathString, DriverServiceTypeString, DriverStartTypeString;
            RtlInitUnicodeString(&DriverErrorControlString, L"ErrorControl");
            RtlInitUnicodeString(&DriverImagePathString, L"ImagePath");
            RtlInitUnicodeString(&DriverServiceTypeString, L"Type");
            RtlInitUnicodeString(&DriverStartTypeString, L"Start");

            HANDLE DriverRegistrationHandle;
            if (!NT_SUCCESS(NtCreateKey(&DriverRegistrationHandle, KEY_ALL_ACCESS, &DriverRegistrationAttribute, 0, nullptr, 0, nullptr))) {
                FreeLibrary(ModuleNtDll);
                throw Exception(String(u"KeDriver::doLoadNt() NtCreateKey"));
            }
            NtSetValueKey(DriverRegistrationHandle, &DriverErrorControlString, 0, REG_DWORD, &DriverErrorControl, sizeof(DWORD));
            NtSetValueKey(DriverRegistrationHandle, &DriverImagePathString, 0, REG_EXPAND_SZ, (void*) DriverImagePathStream.str().c_str(), sizeof(wchar_t) * (DriverImagePathStream.str().size() + 1));
            NtSetValueKey(DriverRegistrationHandle, &DriverStartTypeString, 0, REG_DWORD, &DriverStartType, sizeof(DWORD));
            NtSetValueKey(DriverRegistrationHandle, &DriverServiceTypeString, 0, REG_DWORD, &DriverServiceType, sizeof(DWORD));
            NtClose(DriverRegistrationHandle);

            RtlAdjustPrivilege(SeLoadDriverPrivilege, TRUE, FALSE, nullptr);

            if (!NT_SUCCESS(NtLoadDriver(&DriverRegistrationPath))) {
                FreeLibrary(ModuleNtDll);
                throw Exception(String(u"KeDriver::doLoadNt() NtLoadDriver"));
            }
            FreeLibrary(ModuleNtDll);
        }

        void doLoadSC() {
            SC_HANDLE ServiceManagerHandle = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
            if (!ServiceManagerHandle) throw Exception(String(u"KeDriver::doLoadSC() OpenSCManagerW"));
            SC_HANDLE DriverServiceHandle = CreateServiceW(ServiceManagerHandle, DriverServiceName.toWString().c_str(), DriverServiceName.toWString().c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, DriverStartType, DriverErrorControl, DriverFilePath.toWString().c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
            if (!DriverServiceHandle) {
                CloseServiceHandle(ServiceManagerHandle);
                throw Exception(String(u"KeDriver::doLoadSC() CreateServiceW"));
            }

            if (!StartServiceW(DriverServiceHandle, 0, nullptr)) {
                CloseServiceHandle(DriverServiceHandle);
                CloseServiceHandle(ServiceManagerHandle);
                throw Exception(String(u"KeDriver::doLoadSC() StartServiceW"));
            }

            CloseServiceHandle(DriverServiceHandle);
            CloseServiceHandle(ServiceManagerHandle);
        }

        void doUnloadNt() {
            HMODULE ModuleNtDll = LoadLibraryW(L"ntdll.dll");
            NtUnloadDriverType NtUnloadDriver = (NtUnloadDriverType) GetProcAddress(ModuleNtDll, "NtUnloadDriver");
            RtlAdjustPrivilegeType RtlAdjustPrivilege = (RtlAdjustPrivilegeType) GetProcAddress(ModuleNtDll, "RtlAdjustPrivilege");
            RtlInitUnicodeStringType RtlInitUnicodeString = (RtlInitUnicodeStringType) GetProcAddress(ModuleNtDll, "RtlInitUnicodeString");

            std::basic_stringstream<wchar_t> DriverRegistrationPathStream;
            DriverRegistrationPathStream << L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" << DriverServiceName.toWString();

            UNICODE_STRING DriverRegistrationPath;
            RtlInitUnicodeString(&DriverRegistrationPath, DriverRegistrationPathStream.str().c_str());

            RtlAdjustPrivilege(SeLoadDriverPrivilege, TRUE, FALSE, nullptr);

            if (!NT_SUCCESS(NtUnloadDriver(&DriverRegistrationPath))) {
                FreeLibrary(ModuleNtDll);
                throw Exception(String(u"KeDriver::doUnloadNt() NtUnloadDriver"));
            }
            if (!NT_SUCCESS(SHDeleteKeyW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\eLibraryDriver"))) {
                FreeLibrary(ModuleNtDll);
                throw Exception(String(u"KeDriver::doUnloadNt() SHDeleteKeyW"));
            }
            FreeLibrary(ModuleNtDll);
        }

        void doUnloadSC() {
            SC_HANDLE ServiceManagerHandle = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
            if (!ServiceManagerHandle) throw Exception(String(u"KeDriver::doUnloadSC() OpenSCManagerW"));
            SC_HANDLE DriverServiceHandle = OpenServiceW(ServiceManagerHandle, DriverServiceName.toWString().c_str(), SERVICE_ALL_ACCESS);
            if (!DriverServiceHandle) {
                CloseServiceHandle(ServiceManagerHandle);
                throw Exception(String(u"KeDriver::doUnloadSC() OpenServiceW"));
            }
            SERVICE_STATUS DriverServiceStatus;
            ControlService(DriverServiceHandle, SERVICE_CONTROL_STOP, &DriverServiceStatus);

            if (!DeleteService(DriverServiceHandle)) {
                CloseServiceHandle(DriverServiceHandle);
                CloseServiceHandle(ServiceManagerHandle);
                throw Exception(String(u"KeDriver::doUnloadSC() DeleteService"));
            }
            CloseServiceHandle(DriverServiceHandle);
            CloseServiceHandle(ServiceManagerHandle);
        }
    };
}

#endif
