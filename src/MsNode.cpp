//
// Created by dawid on 28.11.2021.
//

#include "../include/MsNode.h"

std::string MsNode::get_mempool() {
    return request("/api/mempool/txids");
}

std::string MsNode::get_transaction(const std::string &transaction_id) {
    std::string response = request("/api/tx/" + transaction_id);
    Json::Value json_res = Json::Value(response);
    json_res = normalize_transaction_data(json_res);
    return writer.write(json_res);
}

[[nodiscard]] std::string MsNode::request(const std::string &endpoint) {
    if (!connection_exist())
        return {};

    try {
        curlpp::Easy request;
        //  -G url
        std::string url = "https://mempool.space" + endpoint;
        request.setOpt<curlpp::options::Url>(url);

        request.setOpt<curlpp::options::ConnectTimeout>(connect_timeout);

        //  get result as string
        std::ostringstream os;
        request.setOpt<curlpp::options::WriteStream>(&os);

        request.perform();

        return os.str();
    } catch (curlpp::RuntimeError &e) {
        std::cerr << "MsNode::request -> " << e.what() << std::endl;
        check_connection_to_node();
        return {};
    } catch (curlpp::LogicError &e) {
        std::cerr << "MsNode::request -> " << e.what() << std::endl;
        check_connection_to_node();
        return {};
    }
}

[[nodiscard]] bool MsNode::ping() const {
    try {
        curlpp::Easy request;
        //  -G url
        std::string url = "https://mempool.space/api/v1/difficulty-adjustment";
        request.setOpt<curlpp::options::Url>(url);

        request.setOpt<curlpp::options::ConnectTimeout>(connect_timeout);

        //  get result as string
        std::ostringstream os;
        request.setOpt<curlpp::options::WriteStream>(&os);

        request.perform();

        return true;
    } catch (curlpp::RuntimeError &e) {
        return false;
    } catch (curlpp::LogicError &e) {
        return false;
    }
}

[[nodiscard]] Json::Value MsNode::normalize_transaction_data(const Json::Value &raw_transaction) const {
    Json::Value vin(Json::arrayValue);
    for (auto &param: raw_transaction["vin"]) {
        Json::Value jv;
        jv["txid"] = param["txid"];
        jv["vout"] = param["vout"];
        jv["address"] = param["prevout"]["scriptpubkey_address"];
        jv["value"] = param["prevout"]["value"].asDouble() / SAT_TO_BTC;
        vin.append(jv);
    }

    Json::Value vout(Json::arrayValue);
    for (auto &param: raw_transaction["vout"]) {
        Json::Value jv;
        jv["scriptPubKey"]["addresses"].append(param["scriptpubkey_address"]);
        jv["value"] = param["value"].asDouble() / SAT_TO_BTC;
        vout.append(jv);
    }

    Json::Value res;
    res["result"]["vin"] = vin;
    res["result"]["vout"] = vout;

    return res;
}

void MsNode::check_connection_to_node() {
    connection = false;
    std::thread t(&MsNode::check_for_connection, this);
    t.detach();
}

void MsNode::check_for_connection() {
    Timer<std::chrono::seconds> t;
    std::cout << "thread(&MsNode::check_for_connection()) -> started" << std::endl;
    t.start();
    while (!ping() && t.duration() <= 300)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    connection = true;
    std::cout << "thread(&MsNode::check_for_connection()) -> stopped after: " << t.duration() << 's' << std::endl;
}