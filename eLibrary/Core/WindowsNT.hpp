#pragma once

#if eLibrarySystem(Windows) && !eLibraryCompiler(MSVC)

#include <Core/Exception.hpp>

#include <Windows.h>
#include <set>
#include <shlwapi.h>
#include <string>
#include <sstream>
#include <winternl.h>

extern "C" {
NTAPI NTSYSAPI NTSTATUS NtCreateKey(OUT HANDLE *KeyHandle, IN ACCESS_MASK DesiredAccess, IN OBJECT_ATTRIBUTES *ObjectAttributes, unsigned long TitleIndex, IN OPTIONAL UNICODE_STRING *Class, IN unsigned long CreateOptions, OUT OPTIONAL unsigned long *Disposition);
NTAPI NTSYSAPI NTSTATUS NtLoadDriver(IN UNICODE_STRING *DriverServiceName);
NTAPI NTSYSAPI NTSTATUS NtOpenProcess(OUT HANDLE *ProcessHandle, IN ACCESS_MASK DesiredAccess, IN OBJECT_ATTRIBUTES *ObjectAttributes, IN CLIENT_ID *ClientId);
NTAPI NTSYSAPI NTSTATUS NtReadFile(IN HANDLE FileHandle, IN OPTIONAL HANDLE Event, IN OPTIONAL PIO_APC_ROUTINE ApcRoutine, IN OPTIONAL void *ApcContext, OUT IO_STATUS_BLOCK *IoStatusBlock, OUT void *Buffer, IN unsigned long Length, IN OPTIONAL LARGE_INTEGER *ByteOffset, IN OPTIONAL unsigned long *Key);
NTAPI NTSYSAPI NTSTATUS NtResumeProcess(IN HANDLE Process);
NTAPI NTSYSAPI NTSTATUS NtSetValueKey(IN HANDLE KeyHandle, IN UNICODE_STRING *ValueName, IN OPTIONAL unsigned long TitleIndex, IN unsigned long Type, IN OPTIONAL void *Data, IN unsigned long DataSize);
NTAPI NTSYSAPI NTSTATUS NtSuspendProcess(IN HANDLE Process);
NTAPI NTSYSAPI NTSTATUS NtTerminateProcess(IN OPTIONAL HANDLE ProcessHandle, IN NTSTATUS ExitStatus);
NTAPI NTSYSAPI NTSTATUS NtUnloadDriver(IN UNICODE_STRING *DriverServiceName);
NTAPI NTSYSAPI NTSTATUS NtWriteFile(IN HANDLE FileHandle, IN OPTIONAL HANDLE Event, IN OPTIONAL PIO_APC_ROUTINE ApcRoutine, IN OPTIONAL void *ApcContext, OUT IO_STATUS_BLOCK *IoStatusBlock, IN void *Buffer, IN unsigned long Length, IN OPTIONAL LARGE_INTEGER *ByteOffset, IN OPTIONAL unsigned long *Key);
NTAPI NTSYSAPI NTSTATUS RtlAdjustPrivilege(IN unsigned long Privilege, IN BOOLEAN Enable, IN BOOLEAN CurrentThread, OUT OPTIONAL BOOLEAN *Enabled);
}

namespace eLibrary::Core {
    #define SeLoadDriverPrivilege 0xa

    class WindowsException final : public Exception {
    public:
        explicit WindowsException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };

    class NtFile final : public Object {
    private:
        HANDLE FileHandle;

        constexpr NtFile(HANDLE FileHandleSource) noexcept : FileHandle(FileHandleSource) {}

        doDisableCopyAssignConstruct(NtFile)
    public:
        enum class NtFileAccess {
            AccessAll = FILE_ALL_ACCESS,
            AccessAttributeExtendRead = FILE_READ_EA,
            AccessAttributeExtendWrite = FILE_WRITE_EA,
            AccessAttributeRead = FILE_READ_ATTRIBUTES,
            AccessAttributeWrite = FILE_WRITE_ATTRIBUTES,
            AccessDataAppend = FILE_APPEND_DATA,
            AccessDataRead = FILE_READ_DATA,
            AccessDataWrite = FILE_WRITE_DATA,
            AccessDelete = DELETE,
            AccessRead = FILE_READ_ACCESS,
            AccessSynchronize = SYNCHRONIZE,
            AccessWrite = FILE_WRITE_ACCESS
        };

        enum class NtFileAttribute {
            AttributeCompressed = FILE_ATTRIBUTE_COMPRESSED,
            AttributeHidden = FILE_ATTRIBUTE_HIDDEN,
            AttributeNormal = FILE_ATTRIBUTE_NORMAL,
            AttributeOffline = FILE_ATTRIBUTE_OFFLINE,
            AttributeReadonly = FILE_ATTRIBUTE_READONLY,
            AttributeSystem = FILE_ATTRIBUTE_SYSTEM,
            AttributeTemporary = FILE_ATTRIBUTE_TEMPORARY
        };

        enum class NtFileDisposition {
            DispositionCreate = FILE_CREATE,
            DispositionOpen = FILE_OPEN,
            DispositionOpenIf = FILE_OPEN_IF,
            DispositionOverwrite = FILE_OVERWRITE,
            DispositionOverwriteIf = FILE_OVERWRITE_IF,
            DispositionSupersede = FILE_SUPERSEDE
        };

        enum class NtFileOption {
            OptionDeleteOnClose = FILE_DELETE_ON_CLOSE,
            OptionNoCompression = FILE_NO_COMPRESSION,
            OptionSequential = FILE_SEQUENTIAL_ONLY,
            OptionWriteThrough = FILE_WRITE_THROUGH
        };

        enum class NtFileShare {
            ShareDelete = FILE_SHARE_DELETE,
            ShareNone = 0,
            ShareRead = FILE_SHARE_READ,
            ShareWrite = FILE_SHARE_WRITE
        };

        ~NtFile() noexcept {
            if (FileHandle) {
                NtClose(FileHandle);
                FileHandle = nullptr;
            }
        }

        static NtFile doCreate(const String &FilePath, bool FilePathCaseSenstive, const NtFileAccess &FileAccess, const NtFileAttribute &FileAttribute, const NtFileDisposition &FileDisposition, const NtFileOption &FileOption, const NtFileShare &FileShare) {
            HANDLE FileHandle;
            IO_STATUS_BLOCK FileStatusBlock;
            OBJECT_ATTRIBUTES FileObjectAttribute;
            UNICODE_STRING FilePathString;
            RtlInitUnicodeString(&FilePathString, FilePath.toWString().c_str());
            InitializeObjectAttributes(&FileObjectAttribute, &FilePathString, FilePathCaseSenstive ? 0 : OBJ_CASE_INSENSITIVE, nullptr, nullptr)
            if (!NT_SUCCESS(NtCreateFile(&FileHandle, (ACCESS_MASK) FileAccess, &FileObjectAttribute, &FileStatusBlock, nullptr, (ULONG) FileAttribute, (ULONG) FileShare, (ULONG) FileDisposition, (ULONG) FileOption, nullptr, 0)))
                throw WindowsException(String(u"NtFile::doCreate(const String&, bool, const NtFileAccess&, const NtFileAttribute&, const NtFileDisposition&, const NtFileOption&, const NtFileShare&) NtCreateFile"));
            return {FileHandle};
        }

        static NtFile doOpen(const String &FilePath, bool FilePathCaseSenstive, const NtFileAccess &FileAccess, const NtFileOption &FileOption, const NtFileShare &FileShare) {
            HANDLE FileHandle;
            IO_STATUS_BLOCK FileStatusBlock;
            OBJECT_ATTRIBUTES FileObjectAttribute;
            UNICODE_STRING FilePathString;
            RtlInitUnicodeString(&FilePathString, FilePath.toWString().c_str());
            InitializeObjectAttributes(&FileObjectAttribute, &FilePathString, FilePathCaseSenstive ? 0 : OBJ_CASE_INSENSITIVE, nullptr, nullptr)
            if (!NT_SUCCESS(NtOpenFile(&FileHandle, (ACCESS_MASK) FileAccess, &FileObjectAttribute, &FileStatusBlock, (ULONG) FileShare, (ULONG) FileOption)))
                throw WindowsException(String(u"NtFile::doOpen(const String&, bool, const NtFileAccess&, const NtFileOption&, const NtFileShare&) NtOpenFile"));
            return {FileHandle};
        }

        void doRead(void *FileBuffer, unsigned long FileBufferSize) const {
            IO_STATUS_BLOCK FileStatusBlock;
            if (!NT_SUCCESS(NtReadFile(FileHandle, nullptr, nullptr, nullptr, &FileStatusBlock, FileBuffer, FileBufferSize, nullptr, nullptr)))
                throw WindowsException(String(u"NtFile::doRead(void*, unsigned long) NtReadFile"));
        }

        void doWrite(void *FileBuffer, unsigned long FileBufferSize) const {
            IO_STATUS_BLOCK FileStatusBlock;
            if (!NT_SUCCESS(NtWriteFile(FileHandle, nullptr, nullptr, nullptr, &FileStatusBlock, FileBuffer, FileBufferSize, nullptr, nullptr)))
                throw WindowsException(String(u"NtFile::doWrite(void*, unsigned long) NtWriteFile"));
        }
    };

    class NtProcess final : public Object {
    private:
        HANDLE ProcessHandle;

        NtProcess(HANDLE ProcessHandleSource) : ProcessHandle(ProcessHandleSource) {
            if (!ProcessHandleSource) throw WindowsException(String(u"NtProcess::NtProcess(HANDLE) ProcessHandleSource"));
        }

        doDisableCopyAssignConstruct(NtProcess)
    public:
        ~NtProcess() noexcept {
            if (ProcessHandle) {
                NtClose(ProcessHandle);
                ProcessHandle = nullptr;
            }
        }

        static NtProcess doOpen(DWORD ProcessID) {
            CLIENT_ID ProcessClientID{ULongToHandle(ProcessID), nullptr};
            HANDLE ProcessHandle;
            OBJECT_ATTRIBUTES ProcessObjectAttribute;
            InitializeObjectAttributes(&ProcessObjectAttribute, nullptr, 0, nullptr, nullptr)
            if (!NT_SUCCESS(NtOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ProcessObjectAttribute, &ProcessClientID))) throw WindowsException(String(u"NtProcess::doOpen(DWORD) NtOpenProcess"));
            return {ProcessHandle};
        }

        void doResume() {
            if (!NT_SUCCESS(NtResumeProcess(ProcessHandle))) throw WindowsException(String(u"NtProcess::doResume() NtResumeProcess"));
        }

        void doSuspend() {
            if (!NT_SUCCESS(NtSuspendProcess(ProcessHandle))) throw WindowsException(String(u"NtProcess::doSuspend() NtSuspendProcess"));
        }

        void doTerminate(NTSTATUS ProcessStatus) const {
            if (!NT_SUCCESS(NtTerminateProcess(ProcessHandle, ProcessStatus))) throw WindowsException(String(u"NtProcess::doTerminate(NTSTATUS) NtTerminateProcess"));
        }
    };

    class NtSecurityBuffer final : public Object {
    private:
        uintmax_t BufferSize;
        uint8_t *BufferContainer;

        doDisableCopyAssignConstruct(NtSecurityBuffer)
    public:
        NtSecurityBuffer(uintmax_t BufferSizeSource) : BufferSize(BufferSizeSource) {
            BufferContainer = (uint8_t*) VirtualAlloc(0, BufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!VirtualLock(BufferContainer, BufferSize)) {
                VirtualFree(BufferContainer, 0, MEM_RELEASE);
                throw WindowsException(String(u"NtSecurityBuffer::NtSecurityBuffer(uintmax_t) VirtualLock"));
            }
        }

        ~NtSecurityBuffer() {
            if (BufferContainer && BufferSize) {
                RtlSecureZeroMemory(BufferContainer, BufferSize);
                VirtualUnlock(BufferContainer, BufferSize);
                VirtualFree(BufferContainer, BufferSize, MEM_RELEASE);
                BufferSize = 0;
                BufferContainer = nullptr;
            }
        }

        explicit operator uint8_t*() const noexcept {
            return BufferContainer;
        }

        uintmax_t getBufferSize() const noexcept {
            return BufferSize;
        }
    };

    class NtService final : public Object {
    public:
        enum class NtServiceErrorControl {
            ControlCritical = SERVICE_ERROR_CRITICAL,
            ControlIgnore = SERVICE_ERROR_IGNORE,
            ControlNormal = SERVICE_ERROR_NORMAL,
            ControlSevere = SERVICE_ERROR_SEVERE
        };

        enum class NtServiceStartType {
            TypeAuto = SERVICE_AUTO_START,
            TypeBoot = SERVICE_BOOT_START,
            TypeDemand = SERVICE_DEMAND_START,
            TypeDisabled = SERVICE_DISABLED,
            TypeSystem = SERVICE_SYSTEM_START
        };

        enum class NtServiceType {
            TypeKernelDriver = SERVICE_KERNEL_DRIVER
        };
    private:
        SC_HANDLE ServiceHandle;
        String ServiceName;
        String ServicePath;
        NtServiceErrorControl ServiceErrorControl;
        NtServiceStartType ServiceStartType;
        NtServiceType ServiceType;
        std::set<String> ServiceDependencySet;

        doDisableCopyAssignConstruct(NtService)

        NtService(SC_HANDLE ServiceHandleSource, const String &ServiceNameSource, const String &ServicePathSource, const NtServiceErrorControl &ServiceErrorControlSource, const NtServiceStartType &ServiceStartTypeSource, const NtServiceType &ServiceTypeSource) : ServiceHandle(ServiceHandleSource), ServiceName(ServiceNameSource), ServicePath(ServicePathSource), ServiceErrorControl(ServiceErrorControlSource), ServiceStartType(ServiceStartTypeSource), ServiceType(ServiceTypeSource) {
            if (!ServiceHandleSource) throw WindowsException(String(u"NtService::NtService(SC_HANDLE, const String&, const String&, const String&, const NtServiceErrorControl&, const NtServiceStartType&, const NtServiceType&) ServiceHandleSource"));
        }

        friend class NtServiceManager;
    public:
        ~NtService() noexcept {
            if (ServiceHandle) {
                CloseServiceHandle(ServiceHandle);
                ServiceHandle = nullptr;
            }
        }

        void addDependency(const NtService &ServiceDependency) noexcept {
            ServiceDependencySet.insert(ServiceDependency.ServiceName);
        }

        void doControl(DWORD ServiceControl) const {
            if (!ControlService(ServiceHandle, ServiceControl, nullptr))
                throw WindowsException(String(u"NtService::doControl(DWORD) ControlService"));
        }

        void doDelete() const {
            if (!DeleteService(ServiceHandle))
                throw WindowsException(String(u"NtService::doDelete() DeleteService"));
        }

        void doStart(DWORD ServiceArgumentCount, void **ServiceArgumentList) const {
            if (!StartServiceW(ServiceHandle, ServiceArgumentCount, (const wchar_t**) ServiceArgumentList))
                throw WindowsException(String(u"NtService::doStart(DWORD, void**) StartServiceW"));
        }

        NtServiceErrorControl getServiceErrorControl() const noexcept {
            return ServiceErrorControl;
        }

        String getServiceName() const noexcept {
            return ServiceName;
        }

        String getServicePath() const noexcept {
            return ServicePath;
        }

        NtServiceStartType getServiceStartType() const noexcept {
            return ServiceStartType;
        }

        NtServiceType getServiceType() const noexcept {
            return ServiceType;
        }

        DWORD getServiceState() const {
            SERVICE_STATUS ServiceStatus;
            if (!QueryServiceStatus(ServiceHandle, &ServiceStatus))
                throw WindowsException(String(u"NtService::getServiceState() QueryServiceStatus"));
            return ServiceStatus.dwCurrentState;
        }

        void removeDependency(const NtService &ServiceDependency) {
            if (!ServiceDependencySet.contains(ServiceDependency.ServiceName))
                throw WindowsException(String(u"NtService::removeDependency(const NtService&) ServiceDependency"));
            ServiceDependencySet.erase(ServiceDependency.ServiceName);
        }

        void setServiceErrorControl(const NtServiceErrorControl &ServiceErrorControlSource) {
            ServiceErrorControl = ServiceErrorControlSource;
            updateServiceConfiguration();
        }

        void setServicePath(const String &ServicePathSource) {
            ServicePath.doAssign(ServicePathSource);
            updateServiceConfiguration();
        }

        void setServiceStartType(const NtServiceStartType &ServiceStartTypeSource) {
            ServiceStartType = ServiceStartTypeSource;
            updateServiceConfiguration();
        }

        void setServiceType(const NtServiceType &ServiceTypeSource) {
            ServiceType = ServiceTypeSource;
            updateServiceConfiguration();
        }

        void updateServiceConfiguration() const {
            std::basic_stringstream<wchar_t> ServiceDependencyStream;
            for (const auto &ServiceDependency : ServiceDependencySet)
                ServiceDependencyStream << ServiceDependency.toWString() << L'\0';
            ServiceDependencyStream << L'\0';
            if (!ChangeServiceConfigW(ServiceHandle, (DWORD) ServiceType, (DWORD) ServiceStartType, (DWORD) ServiceErrorControl, ServicePath.toWString().c_str(), nullptr, nullptr, ServiceDependencyStream.str().c_str(), nullptr, nullptr, ServiceName.toWString().c_str()))
                throw WindowsException(String(u"NtServcie::updateServiceConfiguration() ChangeServiceConfigW"));
        }
    };

    class NtServiceManager final : public Object {
    private:
        SC_HANDLE ManagerHandle;

        doDisableCopyAssignConstruct(NtServiceManager)
    public:
        NtServiceManager(const String &ManagerMachineName, const String &ManagerDatabaseName) {
            if (!(ManagerHandle = OpenSCManagerW(ManagerMachineName.isNull() ? nullptr : ManagerMachineName.toWString().c_str(), ManagerDatabaseName.isNull() ? nullptr : ManagerDatabaseName.toWString().c_str(), SC_MANAGER_ALL_ACCESS)))
                throw WindowsException(String(u"NtServiceManager::NtServiceManager(const String&, const String&) OpenSCManagerW"));
        }

        ~NtServiceManager() noexcept {
            if (ManagerHandle) {
                CloseServiceHandle(ManagerHandle);
                ManagerHandle = nullptr;
            }
        }

        NtService doCreateService(const String &ServiceName, const String &ServicePath, const NtService::NtServiceType &ServiceType, const NtService::NtServiceStartType &ServiceStartType, const NtService::NtServiceErrorControl &ServiceErrorControl) const {
            SC_HANDLE ServiceHandle = CreateServiceW(ManagerHandle, ServiceName.toWString().c_str(), ServiceName.toWString().c_str(), SERVICE_ALL_ACCESS, (DWORD) ServiceType, (DWORD) ServiceStartType, (DWORD) ServiceErrorControl, ServicePath.toWString().c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
            if (!ServiceHandle) throw WindowsException(String(u"NtServiceManager::doCreateService(const String&, const String&, const NtService::NtServiceType&, const NtService::NtServiceStartType&, const NtService::NtServiceErrorControl&) CreateServiceW"));
            return {ServiceHandle, ServiceName, ServicePath, ServiceErrorControl, ServiceStartType, ServiceType};
        }

        NtService doOpenService(const String &ServiceName) const {
            SC_HANDLE ServiceHandle = OpenServiceW(ManagerHandle, ServiceName.toWString().c_str(), SERVICE_ALL_ACCESS);
            if (!ServiceHandle) throw WindowsException(String(u"NtServiceManager::doOpenService(const String&) OpenServiceW"));
            QUERY_SERVICE_CONFIGW ServiceConfiguration;
            if (!QueryServiceConfigW(ServiceHandle, &ServiceConfiguration, sizeof(QUERY_SERVICE_CONFIGW), nullptr))
                throw WindowsException(String(u"NtServiceManager::doOpenService(const String&) QueryServiceConfigW"));
            return {ServiceHandle, ServiceName, String(ServiceConfiguration.lpBinaryPathName), (NtService::NtServiceErrorControl) ServiceConfiguration.dwErrorControl, (NtService::NtServiceStartType) ServiceConfiguration.dwStartType, (NtService::NtServiceType) ServiceConfiguration.dwServiceType};
        }
    };

    class NtDriver final : public Object {
    public:
        constexpr NtDriver() noexcept = delete;

        static void doLoadNt(const String &DriverServiceName, const String &DriverFilePath, DWORD DriverErrorControl, DWORD DriverStartType) {
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
                throw WindowsException(String(u"NtDriver::doLoadNt(const String&, const String&, DWORD, DWORD) NtCreateKey"));
            NtSetValueKey(DriverRegistrationHandle, &DriverErrorControlString, 0, REG_DWORD, &DriverErrorControl, sizeof(DWORD));
            NtSetValueKey(DriverRegistrationHandle, &DriverImagePathString, 0, REG_EXPAND_SZ, (void*) DriverImagePathStream.str().c_str(), sizeof(wchar_t) * (DriverImagePathStream.str().size() + 1));
            NtSetValueKey(DriverRegistrationHandle, &DriverStartTypeString, 0, REG_DWORD, &DriverStartType, sizeof(DWORD));
            NtSetValueKey(DriverRegistrationHandle, &DriverServiceTypeString, 0, REG_DWORD, &DriverServiceType, sizeof(DWORD));
            NtClose(DriverRegistrationHandle);

            RtlAdjustPrivilege(SeLoadDriverPrivilege, TRUE, FALSE, nullptr);

            if (!NT_SUCCESS(NtLoadDriver(&DriverRegistrationPath)))
                throw WindowsException(String(u"NtDriver::doLoadNt(const String&, const String&, DWORD, DWORD) NtLoadDriver"));
        }

        void doUnloadNt(const String &DriverServiceName) const {
            std::basic_stringstream<wchar_t> DriverRegistrationPathStream;
            DriverRegistrationPathStream << L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" << DriverServiceName.toWString();

            UNICODE_STRING DriverRegistrationPath;
            RtlInitUnicodeString(&DriverRegistrationPath, DriverRegistrationPathStream.str().c_str());

            RtlAdjustPrivilege(SeLoadDriverPrivilege, TRUE, FALSE, nullptr);

            if (!NT_SUCCESS(NtUnloadDriver(&DriverRegistrationPath)))
                throw WindowsException(String(u"NtDriver::doUnloadNt(const String&) NtUnloadDriver"));
            if (!NT_SUCCESS(SHDeleteKeyW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\eLibraryDriver")))
                throw WindowsException(String(u"NtDriver::doUnloadNt(const String&) SHDeleteKeyW"));
        }
    };
}

#endif
