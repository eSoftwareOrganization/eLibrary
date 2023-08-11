#pragma once

#include <Core/Global.hpp>

#include <format>
#include <string>

namespace eLibrary::Core {
    class String;

    class Object {
    public:
        virtual ~Object() noexcept = default;

        virtual uintmax_t hashCode() const noexcept {
            return (uintmax_t) this;
        }

        eLibraryAPI virtual String toString() const noexcept;
    };

    template<typename T>
    concept ObjectDerived = std::derived_from<T, Object>;

    class Objects final : public Object {
    private:
        template<typename K, typename V>
        struct ObjectEntry final {
            K EntryKey;
            V EntryValue;

            constexpr ObjectEntry(const K &EntryKeySource, const V &EntryValueSource) : EntryKey(EntryKeySource), EntryValue(EntryValueSource) {}
        };
    public:
        constexpr Objects() noexcept = delete;

        template<typename F, typename ...Fs>
        static void doCall(F FunctionCurrent, Fs ...FunctionList) {
            FunctionCurrent();
            if constexpr (sizeof...(Fs)) doCall(FunctionList...);
        }

        template<Comparable T>
        static intmax_t doCompare(const T &Object1, const T &Object2) {
            return Object1.doCompare(Object2);
        }

        template<Comparable T>
        static intmax_t doCompare(const T *Object1, const T *Object2) {
            return Object1->doCompare(*Object2);
        }

        template<typename T>
        static constexpr T &&doForward(typename std::remove_reference<T>::type &ObjectSource) noexcept {
            return static_cast<T&&>(ObjectSource);
        }

        template<typename T>
        static constexpr T &&doForward(typename std::remove_reference<T>::type &&ObjectSource) noexcept {
            return static_cast<T&&>(ObjectSource);
        }

        template<typename K, typename F>
        static void doMatchExecute(const K &ObjectKeyTarget, F ObjectRoutineDefault, std::initializer_list<ObjectEntry<K, F>> ObjectEntryList) {
            for (const auto &ObjectEntryCurrent : ObjectEntryList)
                if (!doCompare(ObjectEntryCurrent.EntryKey, ObjectKeyTarget)) {
                    ObjectEntryCurrent.EntryValue();
                    return;
                }
            ObjectRoutineDefault();
        }

        template<typename K, typename V>
        static auto doMatchValue(const K &ObjectKeyTarget, std::initializer_list<ObjectEntry<K, V>> ObjectEntryList) {
            for (const auto &ObjectEntryCurrent : ObjectEntryList)
                if (!doCompare(ObjectEntryCurrent.EntryKey, ObjectKeyTarget)) return ObjectEntryCurrent.EntryValue;
            std::unreachable();
        }

        template<typename K, typename V>
        static auto doMatchValue(const K &ObjectKeyTarget, const V &ObjectValueDefault, std::initializer_list<ObjectEntry<K, V>> ObjectEntryList) {
            for (const auto &ObjectEntryCurrent : ObjectEntryList)
                if (!doCompare(ObjectEntryCurrent.EntryKey, ObjectKeyTarget)) return ObjectEntryCurrent.EntryValue;
            return ObjectValueDefault;
        }

        template<typename T>
        static constexpr auto doMove(T &&ObjectSource) noexcept {
            return static_cast<typename std::remove_reference<T>::type&&>(ObjectSource);
        }

        template<typename T>
        static void doSwap(T &Object1, T &Object2) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
            T ObjectBuffer = doMove(Object1);
            Object1 = doMove(Object2);
            Object2 = doMove(ObjectBuffer);
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
struct std::formatter<ObjectT, CharacterT> : public std::formatter<std::string, CharacterT> {
public:
    template<typename ContextT>
    auto format(const ObjectT &ObjectSource, ContextT &ObjectContext) const {
        return std::formatter<std::string, CharacterT>::format(ObjectSource.toString().toU8String(), ObjectContext);
    }
};
