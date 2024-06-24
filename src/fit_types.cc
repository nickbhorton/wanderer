#include "fit_decode.hpp"
#include "fit_mesg_broadcaster.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <tuple>

#include "fit_defaults.h"

double semicircles_to_degrees(int32_t val)
{
    double constexpr semicircle_to_degree = 180.0 / 2147483648.0;
    return val * semicircle_to_degree;
}

class TypeNameListener : public fit::MesgListener
{
public:
    std::vector<std::tuple<std::string, int>> mesg_name_count{};

    void OnMesg(fit::Mesg& mesg) override
    {
        bool found{false};
        for (auto& [name, count] : mesg_name_count) {
            if (name == mesg.GetName()) {
                count++;
                found = true;
            }
        }
        if (!found) {
            mesg_name_count.push_back({mesg.GetName(), 1});
        }
    }
};

int main(int argc, char* argv[])
{
    fit::Decode decode;
    fit::MesgBroadcaster mesgBroadcaster;
    TypeNameListener type_listener;
    DefaultDeveloperFieldDescriptionListener dev_listener;
    std::fstream file;

    if (argc != 2) {
        printf("Usage: fit_types <filename>\n");
        return -1;
    }

    file.open(argv[1], std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        printf("Error opening file %s\n", argv[1]);
        return -1;
    }

    if (!decode.CheckIntegrity(file)) {
        printf("FIT file integrity failed.\nAttempting to decode...\n");
    }

    mesgBroadcaster.AddListener((fit::MesgListener&)type_listener);

    try {
        decode.Read(&file, &mesgBroadcaster, &mesgBroadcaster, &dev_listener);
    } catch (const fit::RuntimeException& e) {
        std::cerr << "Exception decoding file: %s\n" << e.what() << "\n";
        std::exit(1);
    } catch (...) {
        std::cerr << "Exception decoding file\n";
        std::exit(1);
    }
    for (auto const& [name, count] : type_listener.mesg_name_count) {
        std::cout << name << ": " << count << "\n";
    }
}
