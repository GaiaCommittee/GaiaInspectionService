#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <sw/redis++/redis++.h>
#include <functional>
#include <shared_mutex>

#ifndef TEXT
#define TEXT(Expression) #Expression
#endif

namespace Gaia::InspectionService
{
    /**
     * @brief Client for register and synchronize values to the Redis server.
     */
    class InspectionClient
    {
    protected:
        /// Name prefix for variables in Redis.
        const std::string VariableNamePrefix;
    public:
        /// Unit name of this client.
        const std::string UnitName;

        /**
         * @brief Establish a connection to the Redis server and bind the given name.
         * @param unit_name Name for the unit, will effect the variables name prefix.
         * @param port Port of the Redis server.
         * @param ip IP address of the Redis server.
         */
        explicit InspectionClient(const std::string& unit_name,
                         unsigned int port = 6379, const std::string& ip = "127.0.0.1");
        /**
         * @brief Reuse the connection to a Redis server and bind the given unit name.
         * @param unit_name Name for the unit, will effect the variables name prefix.
         * @param connection Connection to the Redis server.
         */
        InspectionClient(const std::string&  unit_name, std::shared_ptr<sw::redis::Redis> connection);

        /// Destructor which will remove the keys of the reigstered variables.
        virtual ~InspectionClient();

    protected:
        /**
         * @brief A probe is a lambda function to get the value to update into the Redis server.
         * @return To store in the Redis, the result should be values converted to std::string using std::to_string(...).
         */
        using InspectionProbe = std::function<std::string()>;

        /// Connection to the Redis.
        std::shared_ptr<sw::redis::Redis> Connection;

        /// Mutex for probes.
        std::shared_mutex ProbesMutex;
        /// Registered probes.
        std::unordered_map<std::string, std::tuple<InspectionProbe, std::string>> Probes;

    public:
        /**
         * @brief Add a variable probe into the update list.
         * @param name Name of the variable.
         * @param probe Probe for the variable.
         * @details
         *  Added probe will be used in function Update().
         *  Previous probe with the same name will be replaced silently.
         */
        void AddProbe(const std::string& name, InspectionProbe probe);
        /**
         * @brief Remove a variable probe from the update list.
         * @param name Name of the variable.
         * @details This function will do nothing if probe with the given name does not exist.
         */
        void RemoveProbe(const std::string& name);
        /**
         * @brief Update the probe with the given name.
         * @param name Name of the variable to update.
         * @param force_mode If true, the value will be sent to Redis, ignoring the previous value cached.
         * @details
         *  Normally, this function will check the cached previous value,
         *  if the current value has not changed, the value will not be sent to Redis.
         */
        void UpdateProbe(const std::string& name, bool force_mode = false);
        /**
         * @brief Directly update the value of a inspected value,
         *        and it will update the last value for its probe if the probe exists.
         * @param name Name of the variable.
         * @param value Value to update.
         */
        void UpdateValue(const std::string& name, const std::string& value);

        /**
         * @brief Directly update the value of a inspected value,
         *        and it will update the last value for its probe if the probe exists.
         * @tparam ValueType Type of the given value.
         * @param name Name of the variable.
         * @param value Value to pass to std::to_string(...) and then used to update.
         */
        template <typename ValueType>
        void UpdateValue(const std::string& name, const ValueType& value)
        {
            UpdateValue(name, std::to_string(value));
        }

        /**
         * @brief Delete the key of the variable with the given name from the Redis,
         *        and remove the probe for this variable.
         * @param name Name of the variable.
         */
        void RemoveValue(const std::string& name);

    public:
        /**
         * @brief Update all probes.
         * @param force_mode If true, all values will be sent to Redis, ignoring the previous value.
         * @details
         *  Normally, this function will check the cached previous value,
         *  if the current value has not changed, the value will not be sent to Redis.
         */
        void Update(bool force_mode = false);
    };
}