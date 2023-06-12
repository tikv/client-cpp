// Copyright 2021 TiKV Project Authors. Licensed under Apache-2.0.

#include "tikv_client.h"
#include <iostream>

int main() {
    auto client = tikv_client::RawKVClient({"127.0.0.1:2379"});

    const std::uint32_t kTimeoutMs = 10;
    client.put("k1", "v1", kTimeoutMs);

    auto val = client.get("k1",kTimeoutMs);
    if (val) {
        std::cout << "get key: \n(k1:" << *val << ")" << std::endl;
    } else {
        std::cout << "key not found" << std::endl;
    }

    client.batch_put({{"k2","v2"},{"k3","v3"},{"k4","v4"},{"k5","v5"}}, kTimeoutMs);

    const std::uint32_t kLimit = 20;
    // scan [k1,k6), limit 20, timeout 10ms
    auto kv_pairs = client.scan("k1","k6", kLimit ,kTimeoutMs);
    std::cout<<"scan[\"k1\",\"k6\"):"<<std::endl;
    for (auto iter = kv_pairs.begin(); iter != kv_pairs.end(); ++iter) {
        std::cout << "(" << iter->key << ": " << iter->value  << ") ";
    }
    std::cout << std::endl;

    // delete [k3,k5), so [k1,k6) should be [k1,k3) + [k5,k6)
    std::cout<<"scan[\"k1\",\"k6\") after delete:"<<std::endl;
    client.del_range("k3","k5",kTimeoutMs);
    kv_pairs = client.scan("k1","k6", kLimit ,kTimeoutMs);
    for (auto iter = kv_pairs.begin(); iter != kv_pairs.end(); ++iter) {
        std::cout << "(" << iter->key << ": " << iter->value  << ") ";
    }
    std::cout << std::endl;

    return 0;
}
