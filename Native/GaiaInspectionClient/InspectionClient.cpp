#include "InspectionClient.hpp"

#include <utility>

namespace Gaia::InspectionService
{
    /// Establish a connection to the Redis server and bind the given name.
    InspectionClient::InspectionClient(const std::string &unit_name, unsigned int port, const std::string &ip) :
        InspectionClient(unit_name, std::make_shared<sw::redis::Redis>("tcp://" + ip + ":" + std::to_string(port)))
    {}

    /// Reuse the connection to a Redis server and bind the given unit name.
    InspectionClient::InspectionClient(const std::string& unit_name,
                                       std::shared_ptr<sw::redis::Redis> connection) :
        UnitName(unit_name), Connection(std::move(connection)),
        VariableNamePrefix("inspections/" + unit_name + "/")
    {
        if (!Connection) throw std::runtime_error("Connection to Redis is null.");

        Connection->sadd("inspections", UnitName);
    }

    /// Destructor which will remove the keys of the registered variables.
    InspectionClient::~InspectionClient()
    {
        for (const auto& [name, probe_information] : Probes)
        {
            Connection->del(VariableNamePrefix + name);
        }
        Connection->del("inspections/" + UnitName);
        Connection->srem("inspections", UnitName);
    }

    /// Add a variable probe into the update list.
    void InspectionClient::AddProbe(const std::string &name, InspectionClient::InspectionProbe probe)
    {
        std::unique_lock lock(ProbesMutex);
        Probes[name] = std::make_tuple(std::move(probe), "");
        Connection->sadd("inspections/" + UnitName, name);
    }

    /// Remove a variable probe from the update list.
    void InspectionClient::RemoveProbe(const std::string &name)
    {
        std::unique_lock lock(ProbesMutex);
        auto finder = Probes.find(name);
        if (finder != Probes.end())
        {
            Probes.erase(finder);
            Connection->sadd("inspections/" + UnitName, name);
        }
    }

    /// Update the probe with the given name.
    void InspectionClient::UpdateProbe(const std::string &name, bool force_mode)
    {
        std::shared_lock lock(ProbesMutex);
        auto finder = Probes.find(name);
        if (finder == Probes.end()) return;
        const auto& [probe, last_value] = finder->second;
        if (probe)
        {
            auto new_value = probe();
            if (!force_mode && new_value == last_value)
            {
                return;
            }
            std::get<1>(finder->second) = new_value;
            Connection->set(VariableNamePrefix + name, new_value);
        }
    }

    /// Update the value of a inspected value.
    void InspectionClient::UpdateValue(const std::string &name, const std::string& value)
    {
        Connection->set(VariableNamePrefix + name, value);
        Connection->sadd("inspections/" + UnitName, name);
        std::shared_lock lock(ProbesMutex);
        auto finder = Probes.find(name);
        if (finder != Probes.end())
        {
            std::get<1>(finder->second) = value;
        }
    }

    /// Delete the key of the variable with the given name from the Redis, and remove the probe for this variable.
    void InspectionClient::RemoveValue(const std::string &name)
    {
        Connection->del(VariableNamePrefix + name);
        Connection->srem("inspections/" + UnitName, name);
        std::unique_lock lock(ProbesMutex);
        auto finder = Probes.find(name);
        if (finder != Probes.end())
        {
            Probes.erase(finder);
        }
    }

    /// Update all probes.
    void InspectionClient::Update(bool force_mode)
    {
        std::shared_lock lock(ProbesMutex);

        for (auto& [name, probe_information] : Probes)
        {
            const auto& [probe, last_value] = probe_information;
            if (!probe) continue;
            const auto& new_value = probe();
            if (!force_mode && new_value == last_value)
            {
                continue;
            }
            std::get<1>(probe_information) = new_value;
            Connection->set(VariableNamePrefix + name, new_value);
        }
    }
}