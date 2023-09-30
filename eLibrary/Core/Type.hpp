#pragma once

#ifndef eLibraryHeaderCoreType
#define eLibraryHeaderCoreType

#include <Core/Memory.hpp>

#include <map>
#include <string>

namespace eLibrary::Core {
    class TypeBase : public Object {
    protected:
        std::string TypeName;

        doDisableCopyAssignConstruct(TypeBase)
    public:
        TypeBase(const std::string &TypeNameSource) noexcept : TypeName(TypeNameSource) {}

        std::string getTypeName() const noexcept {
            return TypeName;
        }

        virtual bool isClass() const noexcept {
            return false;
        }

        virtual bool isEnumeration() const noexcept {
            return false;
        }
    };

    class TypePropertyBase : public TypeBase {
    private:
        const TypeBase *PropertyValueType;
    public:
        TypePropertyBase(const std::string &TypeNameSource, const TypeBase *PropertyValueTypeSource) noexcept : TypeBase(TypeNameSource), PropertyValueType(PropertyValueTypeSource) {}

        const TypeBase *getValueType() const noexcept {
            return PropertyValueType;
        }
    };

    class TypePropertyClass final : public TypePropertyBase {
    public:
        TypePropertyClass(const std::string &TypeNameSource, const TypeBase *PropertyValueTypeSource) noexcept : TypePropertyBase(TypeNameSource, PropertyValueTypeSource) {}
    };

    class TypePropertyEnumeration final : public TypePropertyBase {
    public:
        TypePropertyEnumeration(const std::string &TypeNameSource, const TypeBase *PropertyValueTypeSource) noexcept : TypePropertyBase(TypeNameSource, PropertyValueTypeSource) {}
    };

    class TypeClass final : public TypeBase {
    private:
        std::string ClassParent;
        std::map<std::string, const TypePropertyBase*> ClassPropertyMap;

        doDisableCopyAssignConstruct(TypeClass)
    public:
        TypeClass(const std::string &TypeNameSource, const std::string &ClassParentSource) noexcept : TypeBase(TypeNameSource), ClassParent(ClassParentSource) {}

        TypeClass *doRegisterProperty(const TypePropertyBase *PropertySource) {
            if (!PropertySource || !PropertySource->isClass()) return this;
            ClassPropertyMap[PropertySource->getTypeName()] = PropertySource;
            return this;
        }

        eLibraryAPI const TypeClass *getClassParent() const noexcept;

        const TypePropertyBase *getClassProperty(const std::string &PropertyName) noexcept {
            auto *PropertyResult = ClassPropertyMap[PropertyName];
            if (!PropertyResult->isClass()) return nullptr;
            return (const TypePropertyClass*) PropertyResult;
        }

        bool isClass() const noexcept override {
            return true;
        }
    };

    class TypeEnumeration final : public TypeBase {
    private:
        doDisableCopyAssignConstruct(TypeEnumeration)

        std::map<std::string, const TypePropertyBase*> EnumerationPropertyMap;
    public:
        TypeEnumeration(const std::string &TypeNameSource, ...) noexcept : TypeBase(TypeNameSource) {}

        TypeEnumeration *doRegisterProperty(const TypePropertyBase *PropertySource) {
            if (!PropertySource || !PropertySource->isEnumeration()) return this;
            EnumerationPropertyMap[PropertySource->getTypeName()] = PropertySource;
            return this;
        }

        const TypePropertyEnumeration *getEnumerationProperty(const std::string &PropertyName) {
            auto *PropertyResult = EnumerationPropertyMap[PropertyName];
            if (!PropertyResult->isEnumeration()) return nullptr;
            return (const TypePropertyEnumeration*) PropertyResult;
        }

        bool isEnumeration() const noexcept override {
            return true;
        }
    };

    class TypeManager final : public Object {
    private:
        std::map<std::string, const TypeBase*> ManagerMap;

        TypeManager() noexcept = default;

        doDisableCopyAssignConstruct(TypeManager)
    public:
        ~TypeManager() noexcept {
            for (auto &TypeEntry : ManagerMap) MemoryAllocator::deleteObject(TypeEntry.second);
            ManagerMap.clear();
        }

        const TypeClass *doQueryClass(const std::string &TypeName) {
            auto *TypeResult = ManagerMap[TypeName];
            if (!TypeResult->isClass()) return nullptr;
            return (TypeClass*) TypeResult;
        }

        TypeManager *doRegisterBase(const TypeBase *TypeSource) {
            if (!TypeSource) return this;
            ManagerMap[TypeSource->getTypeName()] = TypeSource;
            return this;
        }

        TypeManager *doRegisterClass(const TypeBase *TypeSource) {
            if (!TypeSource || !TypeSource->isClass()) return this;
            ManagerMap[TypeSource->getTypeName()] = TypeSource;
            return this;
        }

        TypeManager *doRegisterEnumeration(const TypeBase *TypeSource) {
            if (!TypeSource || !TypeSource->isEnumeration()) return this;
            ManagerMap[TypeSource->getTypeName()] = TypeSource;
            return this;
        }

        const char *getClassName() const noexcept override {
            return "TypeManager";
        }

        static TypeManager *getInstance() {
            static TypeManager ManagerInstance;
            return &ManagerInstance;
        }
    };

    const TypeClass *TypeClass::getClassParent() const noexcept {
        return TypeManager::getInstance()->doQueryClass(ClassParent);
    }
}

#endif
