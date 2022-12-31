#pragma once

#ifdef eLibraryCore_WindowsNT

#include <Windows.h>
#include <map>
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

    class NtModule final {
    private:
        mutable std::map<String, FARPROC> ModuleFunctionMapping;
        HMODULE ModuleHandle;
    public:
        NtModule(const String &ModuleName) {
            if (!(ModuleHandle = LoadLibraryW(ModuleName.toWString().c_str()))) throw Exception(String(u"NtModule::NtModule() LoadLibraryW"));
        }

        ~NtModule() noexcept {
            if (ModuleHandle) {
                FreeLibrary(ModuleHandle);
                ModuleHandle = nullptr;
            }
        }

        FARPROC getFunction(const String &FunctionName) const {
            if (ModuleFunctionMapping.contains(FunctionName)) return ModuleFunctionMapping[FunctionName];
            FARPROC FunctionObject = GetProcAddress(ModuleHandle, FunctionName.toU8String().c_str());
            if (!FunctionObject) throw Exception(String(u"NtModule::getFunction() GetProcAddress"));
            return ModuleFunctionMapping[FunctionName] = FunctionObject;
        }
    };

    class NtService final {
    private:
        SC_HANDLE ServiceHandle;
        String ServiceName;
        String ServicePath;
        SERVICE_ERROR_TYPE ServiceErrorControl;
        SERVICE_LOAD_TYPE ServiceStartType;
        SERVICE_NODE_TYPE ServiceType;
    public:
        NtService(SC_HANDLE Handle, const String &Name, const String &Path, DWORD ErrorControl, DWORD StartType, DWORD Type) : ServiceHandle(Handle), ServiceName(Name), ServicePath(Path), ServiceErrorControl((SERVICE_ERROR_TYPE) ErrorControl), ServiceStartType((SERVICE_LOAD_TYPE) StartType), ServiceType((SERVICE_NODE_TYPE) Type) {}

        ~NtService() noexcept {
            if (ServiceHandle) {
                CloseServiceHandle(ServiceHandle);
                ServiceHandle = nullptr;
            }
        }

        void doControl(DWORD ServiceControl) const {
            if (!ControlService(ServiceHandle, ServiceControl, nullptr))
                throw Exception(String(u"NtService::doControl(DWORD) ControlService"));
        }

        void doDelete() const {
            if (!DeleteService(ServiceHandle))
                throw Exception(String(u"NtService::doDelete() DeleteService"));
        }

        void doStart(DWORD ServiceArgumentCount, void **ServiceArgumentList) const {
            if (!StartServiceW(ServiceHandle, ServiceArgumentCount, (const wchar_t**) ServiceArgumentList))
                throw Exception(String(u"NtService::doStart(DWORD, void**) StartServiceW"));
        }
    };

    class NtServiceManager final {
    private:
        SC_HANDLE ManagerHandle;
    public:
        NtServiceManager(const String &ManagerMachineName, const String &ManagerDatabaseName) {
            if (!(ManagerHandle = OpenSCManagerW(ManagerMachineName.isNull() ? nullptr : ManagerMachineName.toWString().c_str(), ManagerDatabaseName.isNull() ? nullptr : ManagerDatabaseName.toWString().c_str(), SC_MANAGER_ALL_ACCESS)))
                throw Exception(String(u"NtServiceManager::NtServiceManager(const String&, const String&) OpenSCManagerW"));
        }

        ~NtServiceManager() noexcept {
            if (ManagerHandle) {
                CloseServiceHandle(ManagerHandle);
                ManagerHandle = nullptr;
            }
        }

        SC_HANDLE doCreateService(const String &ServiceName, const String &ServiceFilePath, DWORD ServiceType, DWORD ServiceStartType, DWORD ServiceErrorControl) const {
            SC_HANDLE ServiceHandle = CreateServiceW(ManagerHandle, ServiceName.toWString().c_str(), ServiceName.toWString().c_str(), SERVICE_ALL_ACCESS, ServiceType, ServiceStartType, ServiceErrorControl, ServiceFilePath.toWString().c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
            if (!ServiceHandle) throw Exception(String(u"NtServiceManager::doCreateService(const String&, const String&, DWORD, DWORD, DWORD) CreateServiceW"));
            return ServiceHandle;
        }

        NtService doOpenService(const String &ServiceName) const {
            SC_HANDLE ServiceHandle = OpenServiceW(ManagerHandle, ServiceName.toWString().c_str(), SERVICE_ALL_ACCESS);
            if (!ServiceHandle) throw Exception(String(u"NtServiceManager::doOpenService(const String&) OpenServiceW"));
            QUERY_SERVICE_CONFIGW ServiceConfiguration;
            if (!QueryServiceConfigW(ServiceHandle, &ServiceConfiguration, sizeof(QUERY_SERVICE_CONFIGW), nullptr))
                throw Exception(String(u"NtServiceManager::doOpenService(const String&) QueryServiceConfigW"));
            return NtService(ServiceHandle, ServiceName, String(ServiceConfiguration.lpBinaryPathName), ServiceConfiguration.dwErrorControl, ServiceConfiguration.dwServiceType, ServiceConfiguration.dwStartType);
        }
    };

    class NtDriver final {
    private:
        DWORD DriverErrorControl;
        HANDLE DriverFileHandle;
        String DriverFilePath;
        String DriverServiceName;
        DWORD DriverStartType;
    public:
        NtDriver(const String &FilePath, const String &Name, DWORD ErrorControl, DWORD StartType) noexcept : DriverErrorControl(ErrorControl), DriverFileHandle(nullptr), DriverFilePath(FilePath), DriverServiceName(Name), DriverStartType(StartType) {}

        ~NtDriver() noexcept {
            if (DriverFileHandle && DriverFileHandle != INVALID_HANDLE_VALUE) CloseHandle(DriverFileHandle);
        }

        void doCancel() const {
            if (!CancelIo(DriverFileHandle)) throw Exception(String(u"NtDriver::doCancel() CancelIo"));
        }

        void doClose() {
            if (!DriverFileHandle || DriverFileHandle == INVALID_HANDLE_VALUE) throw Exception(String(u"NtDriver::doClose() DriverFileHandle"));
            CloseHandle(DriverFileHandle);
        }

        void doControl(DWORD DriverControlCode, void *DriverControlInputBuffer, DWORD DriverControlInputSize, void *DriverControlOutputBuffer, DWORD DriverControlOutputBufferSize, DWORD *DriverControlOutputSize) {
            if (!DeviceIoControl(DriverFileHandle, DriverControlCode, DriverControlInputBuffer, DriverControlInputSize, DriverControlOutputBuffer, DriverControlOutputBufferSize, DriverControlOutputSize, nullptr))
                throw Exception(String(u"NtDriver::doControl(DWORD, void*, DWORD, void*, DWORD, DWORD*) DeviceIoControl"));
        }

        void doLoadNt() {
            NtModule ModuleNtDll(String(u"ntdll.dll"));
            NtCloseType NtClose = (NtCloseType) ModuleNtDll.getFunction(String(u"NtClose"));
            NtCreateKeyType NtCreateKey = (NtCreateKeyType) ModuleNtDll.getFunction(String(u"NtCreateKey"));
            NtLoadDriverType NtLoadDriver = (NtLoadDriverType) ModuleNtDll.getFunction(String(u"NtLoadDriver"));
            NtSetValueKeyType NtSetValueKey = (NtSetValueKeyType) ModuleNtDll.getFunction(String(u"NtSetValueKey"));
            RtlAdjustPrivilegeType RtlAdjustPrivilege = (RtlAdjustPrivilegeType) ModuleNtDll.getFunction(String(u"RtlAdjustPrivilege"));
            RtlInitUnicodeStringType RtlInitUnicodeString = (RtlInitUnicodeStringType) ModuleNtDll.getFunction(String(u"RtlInitUnicodeString"));

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
            if (!NT_SUCCESS(NtCreateKey(&DriverRegistrationHandle, KEY_ALL_ACCESS, &DriverRegistrationAttribute, 0, nullptr, 0, nullptr)))
                throw Exception(String(u"NtDriver::doLoadNt() NtCreateKey"));
            NtSetValueKey(DriverRegistrationHandle, &DriverErrorControlString, 0, REG_DWORD, &DriverErrorControl, sizeof(DWORD));
            NtSetValueKey(DriverRegistrationHandle, &DriverImagePathString, 0, REG_EXPAND_SZ, (void*) DriverImagePathStream.str().c_str(), sizeof(wchar_t) * (DriverImagePathStream.str().size() + 1));
            NtSetValueKey(DriverRegistrationHandle, &DriverStartTypeString, 0, REG_DWORD, &DriverStartType, sizeof(DWORD));
            NtSetValueKey(DriverRegistrationHandle, &DriverServiceTypeString, 0, REG_DWORD, &DriverServiceType, sizeof(DWORD));
            NtClose(DriverRegistrationHandle);

            RtlAdjustPrivilege(SeLoadDriverPrivilege, TRUE, FALSE, nullptr);

            if (!NT_SUCCESS(NtLoadDriver(&DriverRegistrationPath)))
                throw Exception(String(u"NtDriver::doLoadNt() NtLoadDriver"));
        }

        void doLoadSC(const NtServiceManager &ServiceManager) const {
            SC_HANDLE DriverServiceHandle = ServiceManager.doCreateService(DriverServiceName, DriverFilePath, SERVICE_KERNEL_DRIVER, DriverStartType, DriverErrorControl);

            if (!StartServiceW(DriverServiceHandle, 0, nullptr)) {
                CloseServiceHandle(DriverServiceHandle);
                throw Exception(String(u"NtDriver::doLoadSC(const NtServiceManager&) StartServiceW"));
            }

            CloseServiceHandle(DriverServiceHandle);
        }

        void doOpen(const String &DriverControlPath) {
            DriverFileHandle = CreateFileW(DriverControlPath.toWString().c_str(), FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (DriverFileHandle == INVALID_HANDLE_VALUE) throw Exception(String(u"NtDriver::doOpen(const String&) CreateFileW"));
        }

        void doRead(void *DriverControlBuffer, DWORD DriverControlBufferSize, DWORD *DriverControlSize) const {
            if (!ReadFile(DriverFileHandle, DriverControlBuffer, DriverControlBufferSize, DriverControlSize, nullptr))
                throw Exception(String(u"NtDriver::doRead(void*, DWORD, DWORD) ReadFile"));
        }

        void doUnloadNt() const {
            NtModule ModuleNtDll(String(u"ntdll.dll"));
            NtUnloadDriverType NtUnloadDriver = (NtUnloadDriverType) ModuleNtDll.getFunction(String(u"NtUnloadDriver"));
            RtlAdjustPrivilegeType RtlAdjustPrivilege = (RtlAdjustPrivilegeType) ModuleNtDll.getFunction(String(u"RtlAdjustPrivilege"));
            RtlInitUnicodeStringType RtlInitUnicodeString = (RtlInitUnicodeStringType) ModuleNtDll.getFunction(String(u"RtlInitUnicodeString"));

            std::basic_stringstream<wchar_t> DriverRegistrationPathStream;
            DriverRegistrationPathStream << L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" << DriverServiceName.toWString();

            UNICODE_STRING DriverRegistrationPath;
            RtlInitUnicodeString(&DriverRegistrationPath, DriverRegistrationPathStream.str().c_str());

            RtlAdjustPrivilege(SeLoadDriverPrivilege, TRUE, FALSE, nullptr);

            if (!NT_SUCCESS(NtUnloadDriver(&DriverRegistrationPath)))
                throw Exception(String(u"NtDriver::doUnloadNt() NtUnloadDriver"));
            if (!NT_SUCCESS(SHDeleteKeyW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\eLibraryDriver")))
                throw Exception(String(u"NtDriver::doUnloadNt() SHDeleteKeyW"));
        }

        void doUnloadSC(const NtServiceManager &ServiceManager) const {
            NtService DriverService = ServiceManager.doOpenService(DriverServiceName);
            DriverService.doControl(SERVICE_CONTROL_STOP);
            DriverService.doDelete();
        }

        void doWrite(void *DriverControlBuffer, DWORD DriverControlBufferSize, DWORD *DriverControlSize) const {
            if (!WriteFile(DriverFileHandle, DriverControlBuffer, DriverControlBufferSize, DriverControlSize, nullptr))
                throw Exception(String(u"NtDriver::doWrite(void*, DWORD, DWORD) WriteFile"));
        }
    };
}

#endif
