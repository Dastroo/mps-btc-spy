//
// Created by dawid on 28.11.2021.
//

#pragma once

#include <string>
#include <sstream>
#include <thread>
#include <atomic>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>

#include "Timer.h"

/// ms -> Mempool.space
class MsNode {
    Json::FastWriter writer;
    const double SAT_TO_BTC = 100000000;
    //  in seconds
    int connect_timeout = 3;

    std::atomic<bool> connection = true;

public:
    MsNode() = default;

    ~MsNode() = default;

    [[nodiscard]] inline bool connection_exist() const {
        return connection;
    }

    inline void set_connection_timeout(int seconds) {
        connect_timeout = seconds;
    }

    /**
     * @brief request transaction ids from mempool.space using their api
     * @return Json::Value().empty() on error or Json::Value on success
     */
    std::string get_mempool();

    /**
     * @brief request transaction from mempool.space using their api
     * @example example at Server/example_responses/mempool_response.json
     * @return Json::Value().empty() on error or Json::Value on success
     */
    std::string get_transaction(const std::string &transaction_id);

private:
    /**
     * @brief https://mempool.space/api
     * @param endpoint method from above mentioned api
     * @returns mempool.space response
     */
    [[nodiscard]] std::string request(const std::string &endpoint);

    [[nodiscard]] bool ping() const;

    /**
     * @brief takes a mempool.space "GET transaction" response and turns it into
     * something closer to a bitcoin node "getrawtransaction" response
     * @returns project normalized response
     */
    [[nodiscard]] Json::Value normalize_transaction_data(const Json::Value &raw_transaction) const;

    void check_connection_to_node();

    /**
     * @brief meant to be used in a thread, pings node for 5min
     */
    void check_for_connection();
};


