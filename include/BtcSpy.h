//
// Created by dawid on 11.10.2021.
//

#pragma once

#include <string>
#include <sstream>
#include <chrono>

#include <log_helper/Log.h>
#include <mps_utils/NotificationsAPI.h>
#include <algorithm>

#include "Timer.h"
#include "Mempool.h"

class BtcSpy {
    const std::string TAG = "BtcSpy";

    enum transaction_type {
        BUY,
        SELL
    };

    std::string service_name = "btc_spy";
    std::string whale_address;

    std::vector<std::string> checked_transaction_ids;
    std::atomic<bool> running = true;
    std::atomic<bool> logging = true;

    Mempool mempool;
    mps::NotificationsAPI notificationsAPI;

    //  just for statistics
    int transaction_count = 0, vin_count = 0, vout_count = 0;

public:
    explicit BtcSpy(const std::string &ip, const std::string &bdf_token) : mempool(ip, bdf_token) {
        //  GET BDF_TOKEN FROM SERVER WIDE CONFIG FILE
        Json::Value value;
        std::ifstream is(mps::SvrDir::usr().append("config.json"));
        Json::Reader().parse(is, value, false);
        if (!value.isMember("btc_whale_address"))
            throw std::invalid_argument("in config.json: btc_whale_address does not exist");
        whale_address = value["btc_whale_address"].asString();
    };

    ~BtcSpy() = default;

    void connect() {
        int sec = 0;
        while (running && !notificationsAPI.connect(service_name))
            std::this_thread::sleep_for(std::chrono::seconds(sec >= 60 ? sec : sec++));
    }

    void run() {
        Timer<std::chrono::milliseconds> timer;

        while (running) {
            timer.start();

            //  get all transaction ids from mempool
            Json::Value transaction_ids = mempool.get_mempool();
            if (transaction_ids.empty() || transaction_ids["result"].empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }

            scan_transactions(transaction_ids["result"]);

            display_stats(timer.duration());
            transaction_count = 0, vin_count = 0, vout_count = 0;

            delay_to_sec(timer.duration());

            timer.stop();
        }
    }

    void stop() {
        running = false;
        notificationsAPI.disconnect(service_name);
    }

private:
    void scan_transactions(const Json::Value &transaction_ids) {
        std::vector<std::string> new_transactions;
        new_transactions.reserve(transaction_ids.size());

        //  iterate through all transaction_ids from btc memory pool
        for (auto &transaction_id: transaction_ids) {
            if (is_checked(transaction_id)) {
                new_transactions.emplace_back(transaction_id.asString());
                continue;
            }

            ++transaction_count;

            //  get transaction info
            Json::Value transaction = mempool.get_transaction(transaction_id.asString());
            if (transaction.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            scan_transaction(transaction);

            //  save transaction col_android_id
            new_transactions.emplace_back(transaction_id.asString());

            if (!running)
                break;
        }
        checked_transaction_ids = std::move(new_transactions);
        std::sort(checked_transaction_ids.begin(), checked_transaction_ids.end());
    }

    void scan_transaction(const Json::Value &transaction) {
        //  CHECK IF WHALE SOLD
        for (auto &vin: transaction["result"]["vin"]) {
            //  only mempool.space RESPONSE includes the prevout's address
            if (vin.isMember("address")) {
                if (vin["address"].asString() != whale_address)
                    continue;
                send_notifications(SELL, vin["value"].asDouble());
                Log::i(TAG, "notification sent");
                return;
            }

            std::string transaction_id = vin["txid"].asString();
            Json::Value vin_transaction = mempool.get_transaction(transaction_id);
            if (vin_transaction.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }

            //  for statistics
            ++vin_count;

            //  SCANNING VOUT POINTED BY VIN
            int vout_n = vin["vout"].asInt();
            Json::Value vout = vin_transaction["result"]["vout"][vout_n];
            if (arr_contains_string(vout["scriptPubKey"]["addresses"], whale_address)) {
                double value = get_sell_amount(transaction["result"]["vout"]);

                send_notifications(SELL, value);
                Log::i(TAG, "notification sent");
                return;
            }
            if (!running)
                break;
        }

        //  CHECK IF WHALE BOUGHT
        for (auto &vout: transaction["result"]["vout"]) {
            ++vout_count;
            //  SCANNING VOUTS
            if (arr_contains_string(vout["scriptPubKey"]["addresses"], whale_address)) {
                double value = get_buy_amount(transaction["result"]["vout"]);
                Log::i(TAG, "notification sent");
                send_notifications(BUY, value);
            }
        }
    }

    double get_sell_amount(const Json::Value &vouts) {
        double btc = 0;
        for (auto &vout: vouts) {
            ++vout_count;
            //  SCANNING VOUTS
            if (!arr_contains_string(vout["scriptPubKey"]["addresses"], whale_address))
                btc += vout["value"].asDouble();
        }
        return btc;
    }

    double get_buy_amount(const Json::Value &vouts) {
        double btc = 0;
        for (auto &vout: vouts) {
            ++vout_count;
            //  SCANNING VOUTS
            if (arr_contains_string(vout["scriptPubKey"]["addresses"], whale_address))
                btc += vout["value"].asDouble();
        }
        return btc;
    }

    static bool arr_contains_string(const Json::Value &vout, const std::string &address) {
        return std::any_of(vout.begin(), vout.end(),
                           [&](const Json::Value &vout_address) { return vout_address.asString() == address; });
    }

    bool is_checked(const Json::Value &transaction_id) {
        return std::binary_search(checked_transaction_ids.begin(), checked_transaction_ids.end(),
                                  transaction_id.asString());
    }

    void display_stats(long duration) {
        if (!logging)
            return;
        Log::t(TAG,
               "\n\tnumber of new transactions: " + std::to_string(transaction_count) +
               "\n\tnumber of checked transactions: " + std::to_string(checked_transaction_ids.size()) +
               "\n\tnumber of vin's checked: " + std::to_string(vin_count) +
               "\n\tnumber of vout's checked: " + std::to_string(vout_count) +
               "\n\tduration: " + std::to_string(duration) + "ms");
    }

    void send_notifications(transaction_type alert, double amount) {
        if (amount < 50.0f)    // don't notify if whale moves less than x btc
            return;
        if (alert == BUY)
            notificationsAPI.notify_clients(service_name, "BUY", "whale bought " + std::to_string(amount) + " BTC");
        else if (alert == SELL)
            notificationsAPI.notify_clients(service_name, "SELL", "whale sold " + std::to_string(amount) + " BTC");
    }

    static void delay_to_sec(long duration) {
        if (duration >= 1000)
            return;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - duration));
    }
};