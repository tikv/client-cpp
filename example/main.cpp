// Copyright 2021 TiKV Project Authors. Licensed under Apache-2.0.

#include "tikv_client.h"
#include <iostream>

int main() {
    auto client = tikv_client::TransactionClient({"127.0.0.1:2379"});
    auto txn = client.begin();

    txn.put("k1", "v2");

    auto val = txn.get("k1");
    if (val) {
        std::cout << "get key k1:" << *val << std::endl;
    } else {
        std::cout << "key not found" << std::endl;
    }

    auto kv_pairs = txn.scan("k1", Bound::Included, "", Bound::Unbounded, 10);
    for (auto iter = kv_pairs.begin(); iter != kv_pairs.end(); ++iter) {
        std::cout << "scan:" << iter->key << ": " << iter->value << std::endl;
    }

    txn.commit();

    return 0;
}
