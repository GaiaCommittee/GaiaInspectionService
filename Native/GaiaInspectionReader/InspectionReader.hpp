#pragma once

#include <string>
#include <memory>
#include <sw/redis++/redis++.h>
#include <unordered_set>
#include <optional>
#include <boost/lexical_cast.hpp>

namespace Gaia::InspectionService
{
    class InspectionReader
    {
    protected:
        /// Name prefix for variables in Redis.
        std::string VariableNamePrefix;
        /// Unit name of this client.
        std::string UnitName {"*"};

    public:
        /**
         * @brief Establish a connection to the Redis server and bind the given name.
         * @param unit_name Name for the unit, will effect the variables name prefix.
         * @param port Port of the Redis server.
         * @param ip IP address of the Redis server.
         */
        explicit InspectionReader(const std::string& unit_name = "*",
                                  unsigned int port = 6379, const std::string& ip = "127.0.0.1");
        /**
         * @brief Reuse the connection to a Redis server and bind the given unit name.
         * @param unit_name Name for the unit, will effect the variables name prefix.
         * @param connection Connection to the Redis server.
         */
        InspectionReader(const std::string& unit_name, std::shared_ptr<sw::redis::Redis> connection);

    protected:
        /// Connection to the Redis.
        std::shared_ptr<sw::redis::Redis> Connection;

    public:
        /// Query all available units list.
        std::unordered_set<std::string> QueryUnits();

        /**
         * @brief Rebind this reader to another specific unit.
         * @param unit_name Name of the unit to bind.
         */
        void BindUnit(const std::string& unit_name);

        /**
         * @brief Query all available variables.
         * @pre This reader is bound to a unit.
         * @details
         *  If bound unit name is "*", then all variables will be listed in the format of "unit/item"
         */
        std::unordered_set<std::string> QueryVariables();

        /**
         * @brief Query the string value of a variable with the given name.
         * @param name Name of the variable to query.
         * @pre This reader is bound to a unit.
         * @return Optional value text of this variable.
         */
        std::optional<std::string> QueryText(const std::string& name);

        /**
         * @brief Query the value of an inspected variable with the given name.
         * @tparam ValueType Type of the value to convert to.
         * @param name Name of the variable.
         * @param default_value Default value to return if the variable with the given name does not exist.
         * @pre This reader is bound to a unit.
         * @return Optional value of the variable, std::nullopt when the variable does not exist.
         */
        template <typename ValueType>
        std::optional<ValueType> QueryValue(const std::string& name)
        {
            auto result = QueryText(name);
            if (result)
            {
                return boost::lexical_cast<ValueType>(*result);
            }
            return std::nullopt;
        }

        /**
         * @brief Query the value of an inspected variable with the given name.
         * @tparam ValueType Type of the value to convert to.
         * @param name Name of the variable.
         * @param default_value Default value to return if the variable with the given name does not exist.
         * @pre This reader is bound to a unit.
         * @return Value of the variable or default value.
         */
        template <typename ValueType>
        ValueType QueryValue(const std::string& name, const ValueType& default_value)
        {
            auto value = QueryValue<ValueType>(name);
            if (value)
            {
                return *value;
            }
            return default_value;
        }
    };
}
