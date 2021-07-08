#include <iostream>
#include <thread>
#include <memory>
#include <boost/program_options.hpp>
#include <GaiaInspectionReader/GaiaInspectionReader.hpp>
#include <QApplication>
#include "ChartWindow.hpp"

int main(int arguments_count, char** arguments)
{
    using namespace Gaia::InspectionService;
    using namespace Gaia::InspectionChart;
    using namespace boost::program_options;

    options_description options("Options");

    options.add_options()
            ("help,?", "show help message.")
            ("ip,i", value<std::string>()->default_value("127.0.0.1"),
             "IP address of the Redis server.")
            ("port,p", value<unsigned int>()->default_value(6379),
             "Port of the Redis server.")
            ("unit,u", value<std::string>()->default_value(""),
             "name of the unit to watch")
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
            variables["ip"].as<std::string>());

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

    reader->BindUnit(unit_name);

    QApplication application(arguments_count, arguments);

    ChartWindow window(std::move(reader));
    window.show();

    return QApplication::exec();
}