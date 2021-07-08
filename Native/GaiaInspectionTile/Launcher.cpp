#include <iostream>
#include <thread>
#include <boost/program_options.hpp>
#include <GaiaInspectionReader/GaiaInspectionReader.hpp>

#include <QApplication>
#include "TileWindow.hpp"

int main(int arguments_count, char** arguments)
{
    using namespace Gaia::InspectionService;
    using namespace Gaia::InspectionTile;
    using namespace boost::program_options;

    options_description options("Options");

    options.add_options()
            ("help,?", "show help message.")
            ("host,h", value<std::string>()->default_value("127.0.0.1"),
             "IP address of the Redis server.")
            ("port,p", value<unsigned int>()->default_value(6379),
             "Port of the Redis server.")
            ("unit,u", value<std::string>(),
             "name of the unit to watch")
            ("variable,v", value<std::string>(), "name of the variable to watch.")
            ("frequency,f", value<unsigned int>(), "query frequency, aka. query times per second.")
            ("list,l", "list all inspection variables.");

    variables_map variables;
    store(parse_command_line(arguments_count, arguments, options), variables);
    notify(variables);

    if (variables.count("help"))
    {
        std::cout << options << std::endl;
        return 0;
    }

    auto reader = std::make_unique<InspectionReader>("*", variables["port"].as<unsigned int>(),
                            variables["host"].as<std::string>());

    if (variables.count("list"))
    {
        std::cout << "All inspected variables:" << std::endl;
        auto inspected_variables = reader->QueryVariables();
        for (const auto& inspected_variable : inspected_variables)
        {
            std::cout << "\t" << inspected_variable << std::endl;
        }
        return 0;
    }

    std::string unit_name;
    if (!variables.count("unit"))
    {
        std::cout << "Input unit name: ";
        std::cin >> unit_name;
    }
    else
    {
        unit_name = variables["unit"].as<std::string>();
    }
    std::string variable_name;
    if (!variables.count("variable"))
    {
        std::cout << "Input variable name: ";
        std::cin >> variable_name;
    }
    else
    {
        variable_name = variables["variable"].as<std::string>();
    }

    unsigned int frequency = 1;
    if (variables.count("frequency"))
    {
        frequency = variables["frequency"].as<unsigned int>();
    }

    reader->BindUnit(unit_name);

    QApplication application(arguments_count, arguments);

    TileWindow window(std::move(reader), variable_name, frequency);
    window.show();

    return QApplication::exec();
}