#include <cstdint>
#include <fstream>
#include <iostream>

#include "fit.hpp"
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
    std::vector<std::tuple<FIT_SINT32, FIT_SINT32, FIT_FLOAT32>> positions;
    int unreadable{};
    void OnMesg(fit::RecordMesg& mesg) override
    {
        if (mesg.IsPositionLatValid() && mesg.IsPositionLongValid() &&
            (mesg.IsAltitudeValid() || mesg.IsEnhancedAltitudeValid())) {
            positions.push_back(std::tuple<FIT_SINT32, FIT_SINT32, FIT_FLOAT32>(
                {mesg.GetPositionLat(),
                 mesg.GetPositionLong(),
                 mesg.IsAltitudeValid() ? mesg.GetAltitude()
                                        : mesg.GetEnhancedAltitude()}
            ));

        } else {
            unreadable++;
        }
    };
};

int main(int argc, char* argv[])
{
    fit::Decode decode;
    fit::MesgBroadcaster mesgBroadcaster;
    CoordsListener coords_listener;
    DefaultDeveloperFieldDescriptionListener dev_listener;
    std::fstream ifile;
    std::fstream ofile;

    if (argc != 3) {
        std::cerr << "Usage: fit_coords <input_filename> <output_filename>\n";
        return -1;
    }

    ifile.open(argv[1], std::ios::in | std::ios::binary);
    if (!ifile.is_open()) {
        std::cerr << "Error opening file " << argv[1] << "\n";
        std::exit(1);
    }
    ofile.open(argv[2], std::ios::out | std::ios::binary);
    if (!ofile.is_open()) {
        std::cerr << "Error opening file " << argv[2] << "\n";
        std::exit(1);
    }

    if (!decode.CheckIntegrity(ifile)) {
        std::cerr << "FIT file integrity failed.\nAttempting to decode...\n";
    }

    mesgBroadcaster.AddListener((fit::RecordMesgListener&)coords_listener);

    try {
        decode.Read(&ifile, &mesgBroadcaster, &mesgBroadcaster, &dev_listener);
    } catch (const fit::RuntimeException& e) {
        std::cerr << "Exception decoding file: %s\n" << e.what() << "\n";
        std::exit(1);
    } catch (...) {
        std::cerr << "Exception decoding file\n";
        std::exit(1);
    }
    std::cout << coords_listener.positions.size() << " / "
              << coords_listener.unreadable << "\n";
    for (auto const& [lat, longi, alt] : coords_listener.positions) {
        ofile.write((const char*)&lat, sizeof(lat));
        ofile.write((const char*)&longi, sizeof(longi));
        ofile.write((const char*)&alt, sizeof(alt));
    }
}
