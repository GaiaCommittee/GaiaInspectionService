#include <GaiaInspectionClient/GaiaInspectionClient.hpp>

#include <thread>

int main()
{
    using namespace Gaia;

    int increased_value = 0;
    int decreased_value = 0;

    InspectionService::InspectionWriter client("inspect_test");

    client.AddProbe(TEXT(increased_value),
                    [&increased_value]{return std::to_string(increased_value);});
    client.AddProbe(TEXT(decreased_value),
                    [&decreased_value]{return std::to_string(decreased_value);});
    int times = 30;
    while (times--)
    {
        ++increased_value;
        --decreased_value;

        client.UpdateValue("increased_value", increased_value);

        client.Update();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}