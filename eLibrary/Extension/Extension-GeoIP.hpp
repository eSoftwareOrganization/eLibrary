#pragma once

#ifndef eLibraryHeaderExtensionGeoIP
#define eLibraryHeaderExtensionGeoIP

#if eLibraryFeature(ExtensionGeoIP)

#warning Experimental(Extension::GeoIP)

#include <IO/Exception.hpp>

#include <maxminddb/maxminddb.h>

namespace eLibrary::Extension::GeoIP {
    using namespace eLibrary::Core;

    class MaxMindDatabase final {
    private:
        ::MMDB_s DatabaseObject;

        template<typename ...Ts>
        static String doResolve(::MMDB_lookup_result_s DatabaseResult, Ts&& ...DatabaseParameter) {
            if (!DatabaseResult.found_entry) throw IO::IOException(String(u"MaxMindDatabase::doResolve(MMDB_lookup_result_s) DatabaseResult.found_entry"));
            MMDB_entry_data_s DatabaseEntryData;
            ::MMDB_get_value(&DatabaseResult.entry, &DatabaseEntryData, Objects::doForward<Ts>(DatabaseParameter)...);
            if (!DatabaseEntryData.has_data || DatabaseEntryData.type != MMDB_DATA_TYPE_UTF8_STRING) throw IO::IOException(String(u"MaxMindDatabase::doResolve(MMDB_lookup_result_s) MMDB_get_value"));
            return ::std::string(DatabaseEntryData.utf8_string, DatabaseEntryData.data_size);
        }
    public:
        MaxMindDatabase(const String &DatabaseName, uint32_t DatabaseMode) {
            if (::MMDB_open(DatabaseName.toU8String().c_str(), DatabaseMode, &DatabaseObject) != MMDB_SUCCESS)
                throw IO::IOException(String(u"MaxMindDatabase::doOpen(const String&) MMDB_open"));
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
