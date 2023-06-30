// Copyright 2021 TiKV Project Authors. Licensed under Apache-2.0.

#include "tikv_client.h"

using namespace std;
using ::rust::cxxbridge1::Box;

namespace tikv_client {

KvPair::KvPair(std::string &&key, std::string &&value)
    : key(std::move(key))
    , value(std::move(value))
{}

ffi::KvPair KvPair::to_ffi() {
    ffi::KvPair f_pair;
    f_pair.key.reserve(key.size());
    for (const auto &c : this->key) {
        f_pair.key.emplace_back(static_cast<std::uint8_t>(c));
    }
    f_pair.value.reserve(value.size());
    for (const auto &c : this->value) {
        f_pair.value.emplace_back(static_cast<std::uint8_t>(c));
    }
    return f_pair;
}

TransactionClient::TransactionClient(const std::vector<std::string> &pd_endpoints):
    _client(tikv_client_glue::transaction_client_new(pd_endpoints)) {}

RawKVClient::RawKVClient(const std::vector<std::string> &pd_endpoints):
    _client(tikv_client_glue::raw_client_new(pd_endpoints)) {}

std::optional<std::string> RawKVClient::get(const std::string &key, const std::uint64_t timeout) {
    auto val = tikv_client_glue::raw_get(*_client,key,timeout);
    if (val.is_none) {
        return std::nullopt;
    } else {
        return std::string{val.value.begin(), val.value.end()};
    }
}

void RawKVClient::put(const std::string &key, const std::string &value, const std::uint64_t timeout) {
    tikv_client_glue::raw_put(*_client,key,value,timeout);
}

void RawKVClient::batch_put(const std::vector<KvPair> &kv_pairs, const std::uint64_t timeout) {
    std::vector<ffi::KvPair> pairs;
    pairs.reserve(kv_pairs.size());
    for (auto pair: kv_pairs) {
        pairs.emplace_back(pair.to_ffi());
    }
    tikv_client_glue::raw_batch_put(*_client,pairs,timeout);
}

std::vector<KvPair> RawKVClient::scan(const std::string &startKey, const std::string &endKey, std::uint32_t limit, const std::uint64_t timeout){
    auto kv_pairs = tikv_client_glue::raw_scan(*_client,startKey,endKey,limit,timeout);  
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

void RawKVClient::remove(const std::string &key, const std::uint64_t timeout) {
    tikv_client_glue::raw_delete(*_client,key,timeout);
}

void RawKVClient::remove_range(const std::string &start_key,const std::string &end_key, const std::uint64_t timeout) {
    tikv_client_glue::raw_delete_range(*_client,start_key,end_key,timeout);
}

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
