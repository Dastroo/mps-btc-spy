//
// Created by dawid on 30.11.2021.
//

#include "../include/Mempool.h"

bool Mempool::bdfnode_available() {
    return bdf_node.connection_exist();
}

bool Mempool::msnode_available() {
    return ms_node.connection_exist();
}

/***
 * @returns Json::Value().empty() on error or Json::Value on success
 */
Json::Value Mempool::get_mempool() {
    Json::Value mempool;
    if (bdf_node.connection_exist()) {
        reader.parse(bdf_node.get_mempool(), mempool, false);
    } else if (ms_node.connection_exist())
        reader.parse(ms_node.get_mempool(), mempool, false);

    return mempool;
}

/***
 * @returns Json::Value().empty() on error or Json::Value on success
 */
Json::Value Mempool::get_transaction(const std::string &transaction_id) {
    Json::Value transaction;
    if (bdf_node.connection_exist())
        reader.parse(bdf_node.get_transaction(transaction_id), transaction, false);
    else if (ms_node.connection_exist())
        reader.parse(ms_node.get_transaction(transaction_id), transaction, false);

    return transaction;
}