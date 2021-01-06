// Copyright 2021 TiKV Project Authors. Licensed under Apache-2.0.

#include "tikv_client.h"

using namespace std;
using ::rust::cxxbridge1::Box;

namespace tikv_client {

KvPair::KvPair(std::string &&key, std::string &&value)
    : key(std::move(key))
    , value(std::move(value))
{}

TransactionClient::TransactionClient(const std::vector<std::string> &pd_endpoints):
    _client(tikv_client_glue::transaction_client_new(pd_endpoints)) {}

Transaction TransactionClient::begin() {
    return Transaction(transaction_client_begin(*_client));
}

Transaction TransactionClient::begin_pessimistic() {
    return Transaction(transaction_client_begin_pessimistic(*_client));
}


Transaction::Transaction(Box<tikv_client_glue::Transaction> txn) : _txn(std::move(txn)) {}

std::optional<std::string> Transaction::get(const std::string &key) {
    auto val = transaction_get(*_txn, key);
    if (val.is_none) {
        return std::nullopt;
    } else {
        return std::string{val.value.begin(), val.value.end()};
    }
}

std::optional<std::string> Transaction::get_for_update(const std::string &key) {
    auto val = transaction_get_for_update(*_txn, key);
    if (val.is_none) {
        return std::nullopt;
    } else {
        return std::string{val.value.begin(), val.value.end()};
    }
}

std::vector<KvPair> Transaction::batch_get(const std::vector<std::string> &keys) {
    auto kv_pairs = transaction_batch_get(*_txn, keys);
    std::vector<KvPair> result;
    result.reserve(kv_pairs.size());
    for (auto iter = kv_pairs.begin(); iter != kv_pairs.end(); ++iter) {
        result.emplace_back(
            std::string{(iter->key).begin(), (iter->key).end()},
            std::string{(iter->value).begin(), (iter->value).end()}
        );
    }
    return result;
}

std::vector<KvPair> Transaction::batch_get_for_update(const std::vector<std::string> &keys) {
    auto kv_pairs = transaction_batch_get_for_update(*_txn, keys);
    std::vector<KvPair> result;
    result.reserve(kv_pairs.size());
    for (auto iter = kv_pairs.begin(); iter != kv_pairs.end(); ++iter) {
        result.emplace_back(
            std::string{(iter->key).begin(), (iter->key).end()},
            std::string{(iter->value).begin(), (iter->value).end()}
        );
    }
    return result;
}


std::vector<KvPair> Transaction::scan(const std::string &start, Bound start_bound, const std::string &end, Bound end_bound, std::uint32_t limit) {
    auto kv_pairs = transaction_scan(*_txn, start, start_bound, end, end_bound, limit);
    std::vector<KvPair> result;
    result.reserve(kv_pairs.size());
    for (auto iter = kv_pairs.begin(); iter != kv_pairs.end(); ++iter) {
        result.emplace_back(
            std::string{(iter->key).begin(), (iter->key).end()},
            std::string{(iter->value).begin(), (iter->value).end()}
        );
    }
    return result;
}

std::vector<std::string> Transaction::scan_keys(const std::string &start, Bound start_bound, const std::string &end, Bound end_bound, std::uint32_t limit) {
    auto keys = transaction_scan_keys(*_txn, start, start_bound, end, end_bound, limit);
    std::vector<std::string> result;
    result.reserve(keys.size());
    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
        result.emplace_back(std::string{(iter->key).begin(), (iter->key).end()});
    }
    return result;
}

void Transaction::put(const std::string &key, const std::string &value) {
    transaction_put(*_txn, key, value);
}

void Transaction::batch_put(const std::vector<KvPair> &kvs) {
    for (auto iter = kvs.begin(); iter != kvs.end(); ++iter) {
        transaction_put(*_txn, iter->key, iter->value);
    } 
}

void Transaction::remove(const std::string &key) {
    transaction_delete(*_txn, key);
}

void Transaction::commit() {
    transaction_commit(*_txn);
}

}
