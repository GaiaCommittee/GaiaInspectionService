#include "InspectionReader.hpp"

namespace Gaia::InspectionService
{
    /// Query the value text of the variable with the given name.
    std::optional<std::string> InspectionReader::QueryText(const std::string &name)
    {
        return Connection->get(VariableNamePrefix + name);
    }

    /// Query all available units list.
    std::unordered_set<std::string> InspectionReader::QueryUnits()
    {
        std::unordered_set<std::string> units;
        Connection->smembers("inspections", std::inserter(units, units.end()));
        return units;
    }

    /// Query all available variables.
    std::unordered_set<std::string> InspectionReader::QueryVariables()
    {
        std::unordered_set<std::string> items;
        if (UnitName != "*")
        {
            Connection->smembers("inspections/" + UnitName, std::inserter(items, items.end()));
        }
        else
        {
            long long cursor = 0;
            do
            {
                cursor = Connection->scan(cursor, "inspections/*", std::inserter(items, items.end()));
            }while (cursor != 0);
        }
        return items;
    }

    /// Rebind this reader to another specific unit.
    void InspectionReader::BindUnit(const std::string &unit_name)
    {
        UnitName = unit_name;
        VariableNamePrefix = "inspections/" + UnitName + "/";
    }
}