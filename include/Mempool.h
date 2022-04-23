//
// Created by dawid on 11.10.2021.
//

#pragma once

#include <thread>
#include <atomic>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <jsoncpp/json/reader.h>

#include "MsNode.h"
#include "BdfNode.h"

class Mempool {
    MsNode ms_node;
    BdfNode bdf_node;
    Json::Reader reader;

public:
    explicit Mempool(const std::string &ip, const std::string &bdf_token) : bdf_node(ip, bdf_token) {};

    ~Mempool() = default;

    bool bdfnode_available();

    bool msnode_available();

    /***
     * @returns Json::Value().empty() on error or Json::Value on success
     */
    Json::Value get_mempool();

    /***
     * @returns Json::Value().empty() on error or Json::Value on success
     */
    Json::Value get_transaction(const std::string &transaction_id);
};