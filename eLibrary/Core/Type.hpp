#pragma once

#ifndef eLibraryHeaderCoreType
#define eLibraryHeaderCoreType

#include <Core/Memory.hpp>

#include <map>
#include <string>

namespace eLibrary::Core {
    class TypeBase : public Object, public NonCopyable {
    protected:
        std::string TypeName;
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
    public:
        TypeClass(const std::string &TypeNameSource, const std::string &ClassParentSource) noexcept : TypeBase(TypeNameSource), ClassParent(ClassParentSource) {}

        TypeClass *doRegisterProperty(const TypePropertyBase *PropertySource) {
            if (!PropertySource || !PropertySource->isClass()) return this;
            ClassPropertyMap[PropertySource->getTypeName()] = PropertySource;
            return this;
        }

        std::string getClassParent() const noexcept {
            return ClassParent;
        }

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

    class TypeManager final : public Object, public NonCopyable {
    private:
        std::map<std::string, const TypeBase*> ManagerMap;

        TypeManager() noexcept = default;
    public:
        ~TypeManager() noexcept {
            for (auto &TypeEntry : ManagerMap) MemoryAllocator::deleteObject(TypeEntry.second);
            ManagerMap.clear();
        }

        template<typename T>
        const TypeBase *doQueryType() {}

        const TypeBase *doQueryType(const std::string &TypeName) noexcept {
            return ManagerMap[TypeName];
        }

        TypeManager *doRegisterType(const TypeBase *TypeSource) noexcept {
            if (!TypeSource) return this;
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
}

#endif
