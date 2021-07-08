#include "InspectionReader.hpp"

#include <utility>

#include <utility>

namespace Gaia::InspectionService
{
    /// Establish a connection to the Redis server and bind the given name.
    InspectionReader::InspectionReader(const std::string &unit_name, unsigned int port, const std::string &ip)
        : InspectionReader(unit_name,
                           std::make_shared<sw::redis::Redis>("tcp://" + ip + ":" + std::to_string(port)))
    {}

    /// Reuse the connection to a Redis server and bind the given unit name.
    InspectionReader::InspectionReader(const std::string &unit_name, std::shared_ptr<sw::redis::Redis> connection)
        : UnitName(unit_name), Connection(std::move(connection)), VariableNamePrefix("inspections/" + unit_name + "/")
    {}

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