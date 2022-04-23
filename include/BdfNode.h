//
// Created by dawid on 28.11.2021.
//

#pragma once


#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>

#include <utility>

#include "Timer.h"

/// Bdf -> Blue Dragonfly
class BdfNode {
    std::string ip;
    std::string bdf_token;
    Json::FastWriter writer;
    //  in seconds
    int connect_timeout = 3;

    std::atomic<bool> connection = true;

public:
    explicit BdfNode(std::string ip, std::string bdf_token) : ip(std::move(ip)), bdf_token(std::move(bdf_token)) {};

    ~BdfNode() = default;

    [[nodiscard]] bool connection_exist() const {
        return connection;
    }

    void set_connection_timeout(int seconds) {
        connect_timeout = seconds;
    }

    std::string get_mempool();

    std::string get_transaction(const std::string &transaction_id);

private:
    /**
     * @brief https://developer.bitcoin.org/reference/rpc/
     * @param params set of parameters in json format, more details in example from link above
     * @returns bdf node response
     */
    std::string request(const Json::Value &params);

    bool ping();

    void check_connection_to_node();

    /**
     * @brief meant to be used in a thread, pings node for 5min
     */
    void check_for_connection();
};


