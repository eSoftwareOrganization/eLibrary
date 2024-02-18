#pragma once

#ifndef eLibraryHeaderExtensionGeoIP
#define eLibraryHeaderExtensionGeoIP

#if eLibraryFeature(ExtensionGeoIP)

#include <IO/IOException.hpp>

#include <maxminddb.h>

namespace eLibrary::Extension::GeoIP {
    using namespace eLibrary::Core;

    class MaxMindDatabase final {
    private:
        ::MMDB_s DatabaseObject;

        template<typename ...Ts>
        static String doResolve(::MMDB_lookup_result_s DatabaseResult, Ts&& ...DatabaseParameter) {
            if (!DatabaseResult.found_entry) doThrowChecked(IO::IOException(u"MaxMindDatabase::doResolve(MMDB_lookup_result_s) DatabaseResult.found_entry"_S));
            MMDB_entry_data_s DatabaseEntryData;
            ::MMDB_get_value(&DatabaseResult.entry, &DatabaseEntryData, Objects::doForward<Ts>(DatabaseParameter)...);
            if (!DatabaseEntryData.has_data || DatabaseEntryData.type != MMDB_DATA_TYPE_UTF8_STRING) doThrowChecked(IO::IOException(u"MaxMindDatabase::doResolve(MMDB_lookup_result_s) MMDB_get_value"_S));
            return ::std::string(DatabaseEntryData.utf8_string, DatabaseEntryData.data_size);
        }
    public:
        MaxMindDatabase(const String &DatabaseName, uint32_t DatabaseMode) {
            if (::MMDB_open(DatabaseName.toU8String().c_str(), DatabaseMode, &DatabaseObject) != MMDB_SUCCESS)
                doThrowChecked(IO::IOException(u"MaxMindDatabase::doOpen(const String&) MMDB_open"_S));
        }

        ~MaxMindDatabase() {
            ::MMDB_close(&DatabaseObject);
        }

        auto doLookup(const sockaddr *DatabaseAddress) const {
            return doResolve(::MMDB_lookup_sockaddr(&DatabaseObject, DatabaseAddress, nullptr));
        }

        auto doLookup(const String &DatabaseAddress) const {
            return doResolve(::MMDB_lookup_string(&DatabaseObject, DatabaseAddress.toU8String().c_str(), nullptr, nullptr));
        }
    };
}

#endif

#endif
