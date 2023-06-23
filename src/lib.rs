// Copyright 2021 TiKV Project Authors. Licensed under Apache-2.0.

use core::panic;
use std::ops;

use anyhow::Result;
use cxx::{CxxString, CxxVector};
use futures::executor::block_on;

use self::ffi::*;

#[cxx::bridge]
mod ffi {
    struct Key {
        key: Vec<u8>,
    }

    #[namespace = "ffi"]
    struct KvPair {
        key: Vec<u8>,
        value: Vec<u8>,
    }

    struct OptionalValue {
        is_none: bool,
        value: Vec<u8>,
    }

    enum Bound {
        Included,
        Excluded,
        Unbounded,
    }

    #[namespace = "tikv_client_glue"]
    extern "Rust" {
        type TransactionClient;
        type Transaction;
        type RawKVClient;

        fn raw_client_new(pd_endpoints: &CxxVector<CxxString>) -> Result<Box<RawKVClient>>;

        fn raw_get(client: &RawKVClient, key: &CxxString, timeout_ms: u32)
            -> Result<OptionalValue>;

        fn raw_put(
            cli: &RawKVClient,
            key: &CxxString,
            val: &CxxString,
            timeout_ms: u32,
        ) -> Result<()>;

        fn raw_scan(
            cli: &RawKVClient,
            start: &CxxString,
            end: &CxxString,
            limit: u32,
            timeout_ms: u32,
        ) -> Result<Vec<KvPair>>;

        fn raw_delete(cli: &RawKVClient, key: &CxxString, timeout_ms: u32) -> Result<()>;

        fn raw_delete_range(
            cli: &RawKVClient,
            startKey: &CxxString,
            endKey: &CxxString,
            timeout_ms: u32,
        ) -> Result<()>;

        fn raw_batch_put(
            cli: &RawKVClient,
            pairs: &CxxVector<KvPair>,
            timeout_ms: u32,
        ) -> Result<()>;

        fn transaction_client_new(
            pd_endpoints: &CxxVector<CxxString>,
        ) -> Result<Box<TransactionClient>>;

        fn transaction_client_begin(client: &TransactionClient) -> Result<Box<Transaction>>;

        fn transaction_client_begin_pessimistic(
            client: &TransactionClient,
        ) -> Result<Box<Transaction>>;

        fn transaction_get(transaction: &Transaction, key: &CxxString) -> Result<OptionalValue>;

        fn transaction_get_for_update(
            transaction: &mut Transaction,
            key: &CxxString,
        ) -> Result<OptionalValue>;

        fn transaction_batch_get(
            transaction: &mut Transaction,
            keys: &CxxVector<CxxString>,
        ) -> Result<Vec<KvPair>>;

        fn transaction_batch_get_for_update(
            transaction: &mut Transaction,
            keys: &CxxVector<CxxString>,
        ) -> Result<Vec<KvPair>>;

        fn transaction_scan(
            transaction: &mut Transaction,
            start: &CxxString,
            start_bound: Bound,
            end: &CxxString,
            end_bound: Bound,
            limit: u32,
        ) -> Result<Vec<KvPair>>;

        fn transaction_scan_keys(
            transaction: &mut Transaction,
            start: &CxxString,
            start_bound: Bound,
            end: &CxxString,
            end_bound: Bound,
            limit: u32,
        ) -> Result<Vec<Key>>;

        fn transaction_put(
            transaction: &mut Transaction,
            key: &CxxString,
            val: &CxxString,
        ) -> Result<()>;

        fn transaction_delete(transaction: &mut Transaction, key: &CxxString) -> Result<()>;

        fn transaction_commit(transaction: &mut Transaction) -> Result<()>;
    }
}

#[repr(transparent)]
struct TransactionClient {
    inner: tikv_client::TransactionClient,
}

struct RawKVClient {
    inner: tikv_client::RawClient,
}

#[repr(transparent)]
struct Transaction {
    inner: tikv_client::Transaction,
}

fn raw_client_new(pd_endpoints: &CxxVector<CxxString>) -> Result<Box<RawKVClient>> {
    env_logger::init();

    let pd_endpoints = pd_endpoints
        .iter()
        .map(|str| str.to_str().map(ToOwned::to_owned))
        .collect::<std::result::Result<Vec<_>, _>>()?;

    Ok(Box::new(RawKVClient {
        inner: block_on(tikv_client::RawClient::new(pd_endpoints))?,
    }))
}

fn transaction_client_new(pd_endpoints: &CxxVector<CxxString>) -> Result<Box<TransactionClient>> {
    env_logger::init();

    let pd_endpoints = pd_endpoints
        .iter()
        .map(|str| str.to_str().map(ToOwned::to_owned))
        .collect::<std::result::Result<Vec<_>, _>>()?;

    Ok(Box::new(TransactionClient {
        inner: block_on(tikv_client::TransactionClient::new(pd_endpoints))?,
    }))
}

fn transaction_client_begin(client: &TransactionClient) -> Result<Box<Transaction>> {
    Ok(Box::new(Transaction {
        inner: block_on(client.inner.begin_optimistic())?,
    }))
}

fn transaction_client_begin_pessimistic(client: &TransactionClient) -> Result<Box<Transaction>> {
    Ok(Box::new(Transaction {
        inner: block_on(client.inner.begin_pessimistic())?,
    }))
}

fn raw_get(cli: &RawKVClient, key: &CxxString, timeout_ms: u32) -> Result<OptionalValue> {
    match block_on(cli.inner.get(key.as_bytes().to_vec()))? {
        Some(value) => Ok(OptionalValue {
            is_none: false,
            value,
        }),
        None => Ok(OptionalValue {
            is_none: true,
            value: Vec::new(),
        }),
    }
}

// TODO(smityz): implement timeout
fn raw_put(cli: &RawKVClient, key: &CxxString, val: &CxxString, timeout_ms: u32) -> Result<()> {
    block_on(
        cli.inner
            .put(key.as_bytes().to_vec(), val.as_bytes().to_vec()),
    )?;
    Ok(())
}

fn raw_scan(
    cli: &RawKVClient,
    start: &CxxString,
    end: &CxxString,
    limit: u32,
    timeout_ms: u32,
) -> Result<Vec<KvPair>> {
    let rg = to_bound_range(start, Bound::Included, end, Bound::Excluded);
    let pairs = block_on(cli.inner.scan(rg, limit))?
        .into_iter()
        .map(|tikv_client::KvPair(key, value)| KvPair {
            key: key.into(),
            value,
        })
        .collect();
    Ok(pairs)
}

fn raw_delete(cli: &RawKVClient, key: &CxxString, timeout_ms: u32) -> Result<()> {
    block_on(cli.inner.delete(key.as_bytes().to_vec()))?;
    Ok(())
}

fn raw_delete_range(
    cli: &RawKVClient,
    start_key: &CxxString,
    end_key: &CxxString,
    timeout_ms: u32,
) -> Result<()> {
    let rg = to_bound_range(start_key, Bound::Included, end_key, Bound::Excluded);
    block_on(cli.inner.delete_range(rg))?;
    Ok(())
}

fn raw_batch_put(cli: &RawKVClient, pairs: &CxxVector<KvPair>, timeout_ms: u32) -> Result<()> {
    let tikv_pairs: Vec<tikv_client::KvPair> = pairs
        .iter()
        .map(|KvPair { key, value }| -> tikv_client::KvPair {
            tikv_client::KvPair(key.to_vec().into(), value.to_vec())
        })
        .collect();
    block_on(cli.inner.batch_put(tikv_pairs))?;
    Ok(())
}

fn transaction_get(transaction: &Transaction, key: &CxxString) -> Result<OptionalValue> {
    match block_on(transaction.inner.get(key.as_bytes().to_vec()))? {
        Some(value) => Ok(OptionalValue {
            is_none: false,
            value,
        }),
        None => Ok(OptionalValue {
            is_none: true,
            value: Vec::new(),
        }),
    }
}

fn transaction_get_for_update(
    transaction: &mut Transaction,
    key: &CxxString,
) -> Result<OptionalValue> {
    match block_on(transaction.inner.get_for_update(key.as_bytes().to_vec()))? {
        Some(value) => Ok(OptionalValue {
            is_none: false,
            value,
        }),
        None => Ok(OptionalValue {
            is_none: true,
            value: Vec::new(),
        }),
    }
}

fn transaction_batch_get(
    transaction: &mut Transaction,
    keys: &CxxVector<CxxString>,
) -> Result<Vec<KvPair>> {
    let keys = keys.iter().map(|key| key.as_bytes().to_vec());
    let kv_pairs = block_on(transaction.inner.batch_get(keys))?
        .map(|tikv_client::KvPair(key, value)| KvPair {
            key: key.into(),
            value,
        })
        .collect();
    Ok(kv_pairs)
}

fn transaction_batch_get_for_update(
    _transaction: &mut Transaction,
    _keys: &CxxVector<CxxString>,
) -> Result<Vec<KvPair>> {
    // let keys = keys.iter().map(|key| key.as_bytes().to_vec());
    // let kv_pairs = block_on(transaction.inner.batch_get_for_update(keys))?
    //     .map(|tikv_client::KvPair(key, value)| KvPair {
    //         key: key.into(),
    //         value,
    //     })
    //     .collect();
    // Ok(kv_pairs)
    unimplemented!("batch_get_for_update is not working properly so far.")
}

fn transaction_scan(
    transaction: &mut Transaction,
    start: &CxxString,
    start_bound: Bound,
    end: &CxxString,
    end_bound: Bound,
    limit: u32,
) -> Result<Vec<KvPair>> {
    let range = to_bound_range(start, start_bound, end, end_bound);
    let kv_pairs = block_on(transaction.inner.scan(range, limit))?
        .map(|tikv_client::KvPair(key, value)| KvPair {
            key: key.into(),
            value,
        })
        .collect();
    Ok(kv_pairs)
}

fn transaction_scan_keys(
    transaction: &mut Transaction,
    start: &CxxString,
    start_bound: Bound,
    end: &CxxString,
    end_bound: Bound,
    limit: u32,
) -> Result<Vec<Key>> {
    let range = to_bound_range(start, start_bound, end, end_bound);
    let keys = block_on(transaction.inner.scan_keys(range, limit))?
        .map(|key| Key { key: key.into() })
        .collect();
    Ok(keys)
}

fn transaction_put(transaction: &mut Transaction, key: &CxxString, val: &CxxString) -> Result<()> {
    block_on(
        transaction
            .inner
            .put(key.as_bytes().to_vec(), val.as_bytes().to_vec()),
    )?;
    Ok(())
}

fn transaction_delete(transaction: &mut Transaction, key: &CxxString) -> Result<()> {
    block_on(transaction.inner.delete(key.as_bytes().to_vec()))?;
    Ok(())
}

fn transaction_commit(transaction: &mut Transaction) -> Result<()> {
    block_on(transaction.inner.commit())?;
    Ok(())
}

fn to_bound_range(
    start: &CxxString,
    start_bound: Bound,
    end: &CxxString,
    end_bound: Bound,
) -> tikv_client::BoundRange {
    let start_bound = match start_bound {
        Bound::Included => ops::Bound::Included(start.as_bytes().to_vec()),
        Bound::Excluded => ops::Bound::Excluded(start.as_bytes().to_vec()),
        Bound::Unbounded => ops::Bound::Unbounded,
        _ => panic!("unexpected bound"),
    };
    let end_bound = match end_bound {
        Bound::Included => ops::Bound::Included(end.as_bytes().to_vec()),
        Bound::Excluded => ops::Bound::Excluded(end.as_bytes().to_vec()),
        Bound::Unbounded => ops::Bound::Unbounded,
        _ => panic!("unexpected bound"),
    };
    tikv_client::BoundRange::from((start_bound, end_bound))
}
