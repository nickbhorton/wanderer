#include <cstdint>
#include <fstream>
#include <iostream>

#include "fit_decode.hpp"
#include "fit_mesg_broadcaster.hpp"

#include "fit_defaults.h"

double semicircles_to_degrees(int32_t val)
{
    double constexpr semicircle_to_degree = 180.0 / 2147483648.0;
    return val * semicircle_to_degree;
}

class CoordsListener : public fit::RecordMesgListener
{
public:
    void OnMesg(fit::RecordMesg& mesg) override
    {
        if (mesg.IsPositionLatValid()) {
            std::cout << mesg.GetPositionLat() << " ";
        } else {
            std::cout << "null ";
        }
        if (mesg.IsPositionLongValid()) {
            std::cout << mesg.GetPositionLong() << " ";
        } else {
            std::cout << "null ";
        }
        if (mesg.IsAltitudeValid()) {
            std::cout << mesg.GetAltitude() << " ";
        } else if (mesg.IsEnhancedAltitudeValid()) {
            std::cout << mesg.GetEnhancedAltitude() << " ";
        } else {
            std::cout << "null ";
        }
        std::cout << "\n";
    }
};

int main(int argc, char* argv[])
{
    fit::Decode decode;
    fit::MesgBroadcaster mesgBroadcaster;
    CoordsListener coords_listener;
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

    mesgBroadcaster.AddListener((fit::RecordMesgListener&)coords_listener);

    try {
        decode.Read(&file, &mesgBroadcaster, &mesgBroadcaster, &dev_listener);
    } catch (const fit::RuntimeException& e) {
        std::cerr << "Exception decoding file: %s\n" << e.what() << "\n";
        std::exit(1);
    } catch (...) {
        std::cerr << "Exception decoding file\n";
        std::exit(1);
    }
}
