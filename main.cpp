#include <csignal>
#include <fstream>

#include <jsoncpp/json/reader.h>
#include <mps_utils/SvrDir.h>

#include "include/BtcSpy.h"

int main() {
    Log::init(mps::SvrDir::var().append("logs/btc-spy/log"));
    Log::set_level(Log::INFO);

    //  GET BDF_TOKEN FROM SERVER WIDE CONFIG FILE
    Json::Value value;
    std::ifstream is(mps::SvrDir::usr().append("config.json"));
    Json::Reader().parse(is, value, false);
    std::string bdf_token = value["bdf_token"].asString();
    std::string bdf_node_ip = value["bdf_node_ip"].asString();

    static BtcSpy btc_spy(bdf_node_ip, bdf_token);

    //  SET A WAY TO GRACEFULLY STOP THIS PROCESS
    std::signal(SIGTERM, [](int signum) {
        btc_spy.stop();
        Log::release();

        exit(signum);
    });

    btc_spy.connect();
    btc_spy.run();

    return 0;
}
