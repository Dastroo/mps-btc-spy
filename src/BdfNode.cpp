//
// Created by dawid on 28.11.2021.
//

#include <string>
#include <sstream>
#include <thread>
#include <atomic>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "../include/BdfNode.h"


std::string BdfNode::get_mempool() {
    Json::Value jor;
    jor["jsonrpc"] = 1.0;
    jor["id"] = "curltest";
    jor["method"] = "getrawmempool";
    return request(jor);
}

std::string BdfNode::get_transaction(const std::string &transaction_id) {
    Json::Value params(Json::arrayValue);
    params.append(transaction_id);
    params.append(true);
    // jor -> json object request
    Json::Value jor;
    jor["jsonrpc"] = 1.0;
    jor["id"] = "curltest";
    jor["method"] = "getrawtransaction";
    jor["params"] = params;

    return request(jor);
}

std::string BdfNode::request(const Json::Value &params) {
    if (!connection_exist())
        return {};

    try {
        curlpp::Easy request;
        //  -G url
        std::string url = "http://" + ip + "/";
        request.setOpt<curlpp::options::Url>(url);

        //  -d parameters
        request.setOpt<curlpp::options::PostFields>(writer.write(params));

        //  -H headers
        std::list<std::string> headers;
        headers.emplace_back("token: " + bdf_token);
        headers.emplace_back("content-type: text/json");
        request.setOpt<curlpp::options::HttpHeader>(headers);

        request.setOpt<curlpp::options::ConnectTimeout>(connect_timeout);

        //  get result as string
        std::ostringstream os;
        request.setOpt<curlpp::options::WriteStream>(&os);

        request.perform();

        return os.str();
    } catch (curlpp::RuntimeError &e) {
        std::cerr << "BdfNode::request -> " << e.what() << std::endl;
        check_connection_to_node();
        return {};
    } catch (curlpp::LogicError &e) {
        std::cerr << "BdfNode::request -> " << e.what() << std::endl;
        check_connection_to_node();
        return {};
    }
}

bool BdfNode::ping() {
    try {
        curlpp::Easy request;
        //  -G url
        std::string url = "http://" + ip + "/";
        request.setOpt<curlpp::options::Url>(url);

        //  -d parameters
        Json::Value jor;
        jor["jsonrpc"] = 1.0;
        jor["id"] = "curltest";
        jor["method"] = "apiPing";
        request.setOpt<curlpp::options::PostFields>(writer.write(jor));

        //  -H headers
        std::list<std::string> headers;
        headers.emplace_back("token: " + bdf_token);
        headers.emplace_back("content-type: text/json");
        request.setOpt<curlpp::options::HttpHeader>(headers);

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

void BdfNode::check_connection_to_node() {
    connection = false;
    std::thread t(&BdfNode::check_for_connection, this);
    t.detach();
}

void BdfNode::check_for_connection() {
    Timer<std::chrono::seconds> t;
    std::cout << "thread(&BdfNode::check_for_connection()) -> started" << std::endl;
    t.start();
    while (!ping() && t.duration() <= 300)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    connection = true;
    std::cout << "thread(&BdfNode::check_for_connection()) -> stopped after: " << t.duration() << 's' << std::endl;
}