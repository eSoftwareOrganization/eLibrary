#pragma once

#include <Core/Global.hpp>

#include <format>
#include <string>
#include <typeinfo>

namespace eLibrary::Core {
    class String;

    class Object {
    public:
        virtual ~Object() noexcept = default;

        virtual uintmax_t hashCode() const noexcept {
            return (uintmax_t) this;
        }

        virtual String toString() const noexcept;
    };

    template<typename T>
    concept ObjectDerived = std::derived_from<T, Object>;

    class Objects final : public Object {
    public:
        constexpr Objects() noexcept = delete;

        template<Comparable T>
        static intmax_t doCompare(const T &Object1, const T &Object2) {
            return Object1.doCompare(Object2);
        }

        template<Comparable T>
        static intmax_t doCompare(const T *Object1, const T *Object2) {
            return Object1->doCompare(*Object2);
        }

        template<typename T> requires std::is_object<T>::value
        static T *getAddress(T &ObjectSource) noexcept {
            return (T*) &((char&) ObjectSource);
        }

        template<typename T> requires (!std::is_object<T>::value)
        static T *getAddress(T &ObjectSource) noexcept {
            return &ObjectSource;
        }

        template<Arithmetic T1, Arithmetic T2>
        static auto getMaximum(T1 Object1, T2 Object2) noexcept {
            return Object1 >= Object2 ? Object1 : Object2;
        }

        template<Comparable T>
        static T getMaximum(const T &Object1, const T &Object2) {
            return doCompare(Object1, Object2) >= 0 ? Object1 : Object2;
        }

        template<Arithmetic T1, Arithmetic T2>
        static auto getMinimum(T1 Object1, T2 Object2) noexcept {
            return Object1 <= Object2 ? Object1 : Object2;
        }

        template<Comparable T>
        static T getMinimum(const T &Object1, const T &Object2) {
            return doCompare(Object1, Object2) <= 0 ? Object1 : Object2;
        }
    };
}

template<eLibrary::Core::ObjectDerived ObjectT, typename CharacterT>
struct std::formatter<ObjectT, CharacterT> : public std::formatter<string, CharacterT> {
public:
    template<typename ContextT>
    typename ContextT::iterator format(const ObjectT &ObjectSource, ContextT &ObjectContext) const {
        return std::formatter<std::string, CharacterT>::format(ObjectSource.toString().toU8String(), ObjectContext);
    }
};
