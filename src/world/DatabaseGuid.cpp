
#include "DatabaseGuid.hpp"

#include "DatabaseGuid.hpp"

namespace ehb
{
    const DatabaseGuid UNDEFINED_GUID;

    DatabaseGuid::DatabaseGuid(const char* str)
    {
        fromString(str);
    }

    bool DatabaseGuid::isValidSlow() const
    {
        if (!isValid())
        {
            return (false);
        }

        return false;
    }

    std::string DatabaseGuid::toString() const
    {
        return "DatabaseGuid::toString() - NYI";
    }

    bool DatabaseGuid::fromString(const char* str)
    {
        return false;
    }
} // namespace ehb

//////////////////////////////////////////////////////////////////////////////
