
#pragma once

#include <cstdint>
#include <string>

namespace ehb
{
    class DatabaseGuid
    {
    public:
        DatabaseGuid();
        explicit DatabaseGuid(const char* str);
        explicit DatabaseGuid(uint32_t guid);

        bool isValid() const;
        bool isValidSlow() const; // this does a lookup in the actual SiegeNode database which is why it's marked as slow
        uint32_t value() const;
        void setValue(uint32_t guid);

        std::string toString() const;
        bool fromString(const char* str);

    private:
        uint32_t& rawValue() { return (*(uint32_t*)_guid); }
        const uint32_t& rawValue() const { return (*(const uint32_t*)_guid); }

        friend bool operator==(DatabaseGuid const& guid0, DatabaseGuid const& guid1);
        friend bool operator!=(DatabaseGuid const& guid0, DatabaseGuid const& guid1);
        friend bool operator<(DatabaseGuid const& guid0, DatabaseGuid const& guid1);

        uint8_t _guid[4];
    };

    inline DatabaseGuid::DatabaseGuid()
    {
        rawValue() = 0;
    }

    inline DatabaseGuid::DatabaseGuid(uint32_t guid)
    {
        setValue(guid);
    }

    inline bool DatabaseGuid::isValid() const
    {
        return rawValue() != 0;
    }

    inline uint32_t DatabaseGuid::value() const
    {
        return ((_guid[0] << 24) + (_guid[1] << 16) + (_guid[2] << 8) + _guid[3]);
    }

    inline void DatabaseGuid::setValue(uint32_t guid)
    {
        _guid[0] = (uint8_t)((guid & 0xff000000) >> 24);
        _guid[1] = (uint8_t)((guid & 0x00ff0000) >> 16);
        _guid[2] = (uint8_t)((guid & 0x0000ff00) >> 8);
        _guid[3] = (uint8_t)((guid & 0x000000ff));
    }

    // Comparison
    inline bool operator==(DatabaseGuid const& guid0, DatabaseGuid const& guid1)
    {
        return (guid0.rawValue() == guid1.rawValue());
    }

    inline bool operator!=(DatabaseGuid const& guid0, DatabaseGuid const& guid1)
    {
        return (guid0.rawValue() != guid1.rawValue());
    }

    inline bool operator<(DatabaseGuid const& guid0, DatabaseGuid const& guid1)
    {
        return (guid0.rawValue() < guid1.rawValue());
    }

    extern const DatabaseGuid UNDEFINED_GUID;
} // namespace ehb