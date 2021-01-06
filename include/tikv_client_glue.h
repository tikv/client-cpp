#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace rust {
inline namespace cxxbridge1 {
// #include "rust/cxx.h"

#ifndef CXXBRIDGE1_PANIC
#define CXXBRIDGE1_PANIC
template <typename Exception>
void panic [[noreturn]] (const char *msg);
#endif // CXXBRIDGE1_PANIC

#ifndef CXXBRIDGE1_RUST_BOX
#define CXXBRIDGE1_RUST_BOX
template <typename T>
class Box final {
public:
  using value_type = T;
  using const_pointer =
      typename std::add_pointer<typename std::add_const<T>::type>::type;
  using pointer = typename std::add_pointer<T>::type;

  Box() = delete;
  Box(const Box &);
  Box(Box &&) noexcept;
  ~Box() noexcept;

  explicit Box(const T &);
  explicit Box(T &&);

  Box &operator=(const Box &);
  Box &operator=(Box &&) noexcept;

  const T *operator->() const noexcept;
  const T &operator*() const noexcept;
  T *operator->() noexcept;
  T &operator*() noexcept;

  template <typename... Fields>
  static Box in_place(Fields &&...);

  static Box from_raw(T *) noexcept;

  T *into_raw() noexcept;

private:
  class uninit;
  class allocation;
  Box(uninit) noexcept;
  void drop() noexcept;
  T *ptr;
};

template <typename T>
class Box<T>::uninit {};

template <typename T>
class Box<T>::allocation {
  static T *alloc() noexcept;
  static void dealloc(T *) noexcept;

public:
  allocation() noexcept : ptr(alloc()) {}
  ~allocation() noexcept {
    if (this->ptr) {
      dealloc(this->ptr);
    }
  }
  T *ptr;
};

template <typename T>
Box<T>::Box(const Box &other) : Box(*other) {}

template <typename T>
Box<T>::Box(Box &&other) noexcept : ptr(other.ptr) {
  other.ptr = nullptr;
}

template <typename T>
Box<T>::Box(const T &val) {
  allocation alloc;
  ::new (alloc.ptr) T(val);
  this->ptr = alloc.ptr;
  alloc.ptr = nullptr;
}

template <typename T>
Box<T>::Box(T &&val) {
  allocation alloc;
  ::new (alloc.ptr) T(std::move(val));
  this->ptr = alloc.ptr;
  alloc.ptr = nullptr;
}

template <typename T>
Box<T>::~Box() noexcept {
  if (this->ptr) {
    this->drop();
  }
}

template <typename T>
Box<T> &Box<T>::operator=(const Box &other) {
  if (this->ptr) {
    **this = *other;
  } else {
    allocation alloc;
    ::new (alloc.ptr) T(*other);
    this->ptr = alloc.ptr;
    alloc.ptr = nullptr;
  }
  return *this;
}

template <typename T>
Box<T> &Box<T>::operator=(Box &&other) noexcept {
  if (this->ptr) {
    this->drop();
  }
  this->ptr = other.ptr;
  other.ptr = nullptr;
  return *this;
}

template <typename T>
const T *Box<T>::operator->() const noexcept {
  return this->ptr;
}

template <typename T>
const T &Box<T>::operator*() const noexcept {
  return *this->ptr;
}

template <typename T>
T *Box<T>::operator->() noexcept {
  return this->ptr;
}

template <typename T>
T &Box<T>::operator*() noexcept {
  return *this->ptr;
}

template <typename T>
template <typename... Fields>
Box<T> Box<T>::in_place(Fields &&... fields) {
  allocation alloc;
  auto ptr = alloc.ptr;
  ::new (ptr) T{std::forward<Fields>(fields)...};
  alloc.ptr = nullptr;
  return from_raw(ptr);
}

template <typename T>
Box<T> Box<T>::from_raw(T *raw) noexcept {
  Box box = uninit{};
  box.ptr = raw;
  return box;
}

template <typename T>
T *Box<T>::into_raw() noexcept {
  T *raw = this->ptr;
  this->ptr = nullptr;
  return raw;
}

template <typename T>
Box<T>::Box(uninit) noexcept {}
#endif // CXXBRIDGE1_RUST_BOX

#ifndef CXXBRIDGE1_RUST_BITCOPY
#define CXXBRIDGE1_RUST_BITCOPY
struct unsafe_bitcopy_t final {
  explicit unsafe_bitcopy_t() = default;
};

constexpr unsafe_bitcopy_t unsafe_bitcopy{};
#endif // CXXBRIDGE1_RUST_BITCOPY

#ifndef CXXBRIDGE1_RUST_VEC
#define CXXBRIDGE1_RUST_VEC
template <typename T>
class Vec final {
public:
  using value_type = T;

  Vec() noexcept;
  Vec(std::initializer_list<T>);
  Vec(const Vec &);
  Vec(Vec &&) noexcept;
  ~Vec() noexcept;

  Vec &operator=(Vec &&) noexcept;
  Vec &operator=(const Vec &);

  std::size_t size() const noexcept;
  bool empty() const noexcept;
  const T *data() const noexcept;
  T *data() noexcept;
  std::size_t capacity() const noexcept;

  const T &operator[](std::size_t n) const noexcept;
  const T &at(std::size_t n) const;
  const T &front() const;
  const T &back() const;

  T &operator[](std::size_t n) noexcept;
  T &at(std::size_t n);
  T &front();
  T &back();

  void reserve(std::size_t new_cap);
  void push_back(const T &value);
  void push_back(T &&value);
  template <typename... Args>
  void emplace_back(Args &&... args);

  class iterator;
  iterator begin() noexcept;
  iterator end() noexcept;

  using const_iterator = typename Vec<const T>::iterator;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  Vec(unsafe_bitcopy_t, const Vec &) noexcept;

private:
  static std::size_t stride() noexcept;
  void reserve_total(std::size_t cap) noexcept;
  void set_len(std::size_t len) noexcept;
  void drop() noexcept;

  std::array<std::uintptr_t, 3> repr;
};

template <typename T>
class Vec<T>::iterator final {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = typename std::add_pointer<T>::type;
  using reference = typename std::add_lvalue_reference<T>::type;

  reference operator*() const noexcept;
  pointer operator->() const noexcept;
  reference operator[](difference_type) const noexcept;

  iterator &operator++() noexcept;
  iterator operator++(int) noexcept;
  iterator &operator--() noexcept;
  iterator operator--(int) noexcept;

  iterator &operator+=(difference_type) noexcept;
  iterator &operator-=(difference_type) noexcept;
  iterator operator+(difference_type) const noexcept;
  iterator operator-(difference_type) const noexcept;
  difference_type operator-(const iterator &) const noexcept;

  bool operator==(const iterator &) const noexcept;
  bool operator!=(const iterator &) const noexcept;
  bool operator<(const iterator &) const noexcept;
  bool operator>(const iterator &) const noexcept;
  bool operator<=(const iterator &) const noexcept;
  bool operator>=(const iterator &) const noexcept;

private:
  friend class Vec;
  friend class Vec<typename std::remove_const<T>::type>;
  void *pos;
  std::size_t stride;
};

template <typename T>
Vec<T>::Vec(std::initializer_list<T> init) : Vec{} {
  this->reserve_total(init.size());
  std::move(init.begin(), init.end(), std::back_inserter(*this));
}

template <typename T>
Vec<T>::Vec(const Vec &other) : Vec() {
  this->reserve_total(other.size());
  std::copy(other.begin(), other.end(), std::back_inserter(*this));
}

template <typename T>
Vec<T>::Vec(Vec &&other) noexcept : repr(other.repr) {
  new (&other) Vec();
}

template <typename T>
Vec<T>::~Vec() noexcept {
  this->drop();
}

template <typename T>
Vec<T> &Vec<T>::operator=(Vec &&other) noexcept {
  if (this != &other) {
    this->drop();
    this->repr = other.repr;
    new (&other) Vec();
  }
  return *this;
}

template <typename T>
Vec<T> &Vec<T>::operator=(const Vec &other) {
  if (this != &other) {
    this->drop();
    new (this) Vec(other);
  }
  return *this;
}

template <typename T>
bool Vec<T>::empty() const noexcept {
  return size() == 0;
}

template <typename T>
T *Vec<T>::data() noexcept {
  return const_cast<T *>(const_cast<const Vec<T> *>(this)->data());
}

template <typename T>
const T &Vec<T>::operator[](std::size_t n) const noexcept {
  auto data = reinterpret_cast<const char *>(this->data());
  return *reinterpret_cast<const T *>(data + n * this->stride());
}

template <typename T>
const T &Vec<T>::at(std::size_t n) const {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Vec index out of range");
  }
  return (*this)[n];
}

template <typename T>
const T &Vec<T>::front() const {
  return (*this)[0];
}

template <typename T>
const T &Vec<T>::back() const {
  return (*this)[this->size() - 1];
}

template <typename T>
T &Vec<T>::operator[](std::size_t n) noexcept {
  auto data = reinterpret_cast<char *>(this->data());
  return *reinterpret_cast<T *>(data + n * this->stride());
}

template <typename T>
T &Vec<T>::at(std::size_t n) {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Vec index out of range");
  }
  return (*this)[n];
}

template <typename T>
T &Vec<T>::front() {
  return (*this)[0];
}

template <typename T>
T &Vec<T>::back() {
  return (*this)[this->size() - 1];
}

template <typename T>
void Vec<T>::reserve(std::size_t new_cap) {
  this->reserve_total(new_cap);
}

template <typename T>
void Vec<T>::push_back(const T &value) {
  this->emplace_back(value);
}

template <typename T>
void Vec<T>::push_back(T &&value) {
  this->emplace_back(std::move(value));
}

template <typename T>
template <typename... Args>
void Vec<T>::emplace_back(Args &&... args) {
  auto size = this->size();
  this->reserve_total(size + 1);
  ::new (reinterpret_cast<T *>(reinterpret_cast<char *>(this->data()) +
                               size * this->stride()))
      T(std::forward<Args>(args)...);
  this->set_len(size + 1);
}

template <typename T>
typename Vec<T>::iterator::reference
Vec<T>::iterator::operator*() const noexcept {
  return *static_cast<T *>(this->pos);
}

template <typename T>
typename Vec<T>::iterator::pointer
Vec<T>::iterator::operator->() const noexcept {
  return static_cast<T *>(this->pos);
}

template <typename T>
typename Vec<T>::iterator::reference Vec<T>::iterator::operator[](
    typename Vec<T>::iterator::difference_type n) const noexcept {
  auto pos = static_cast<char *>(this->pos) + this->stride * n;
  return *static_cast<T *>(pos);
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator++() noexcept {
  this->pos = static_cast<char *>(this->pos) + this->stride;
  return *this;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator++(int) noexcept {
  auto ret = iterator(*this);
  this->pos = static_cast<char *>(this->pos) + this->stride;
  return ret;
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator--() noexcept {
  this->pos = static_cast<char *>(this->pos) - this->stride;
  return *this;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator--(int) noexcept {
  auto ret = iterator(*this);
  this->pos = static_cast<char *>(this->pos) - this->stride;
  return ret;
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator+=(
    typename Vec<T>::iterator::difference_type n) noexcept {
  this->pos = static_cast<char *>(this->pos) + this->stride * n;
  return *this;
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator-=(
    typename Vec<T>::iterator::difference_type n) noexcept {
  this->pos = static_cast<char *>(this->pos) - this->stride * n;
  return *this;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator+(
    typename Vec<T>::iterator::difference_type n) const noexcept {
  auto ret = iterator(*this);
  ret.pos = static_cast<char *>(this->pos) + this->stride * n;
  return ret;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator-(
    typename Vec<T>::iterator::difference_type n) const noexcept {
  auto ret = iterator(*this);
  ret.pos = static_cast<char *>(this->pos) - this->stride * n;
  return ret;
}

template <typename T>
typename Vec<T>::iterator::difference_type
Vec<T>::iterator::operator-(const iterator &other) const noexcept {
  auto diff = std::distance(static_cast<char *>(other.pos),
                            static_cast<char *>(this->pos));
  return diff / this->stride;
}

template <typename T>
bool Vec<T>::iterator::operator==(const iterator &other) const noexcept {
  return this->pos == other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator!=(const iterator &other) const noexcept {
  return this->pos != other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator>(const iterator &other) const noexcept {
  return this->pos > other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator<(const iterator &other) const noexcept {
  return this->pos < other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator>=(const iterator &other) const noexcept {
  return this->pos >= other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator<=(const iterator &other) const noexcept {
  return this->pos <= other.pos;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::begin() noexcept {
  iterator it;
  it.pos = const_cast<typename std::remove_const<T>::type *>(this->data());
  it.stride = this->stride();
  return it;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::end() noexcept {
  iterator it = this->begin();
  it.pos = static_cast<char *>(it.pos) + it.stride * this->size();
  return it;
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::begin() const noexcept {
  return this->cbegin();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::end() const noexcept {
  return this->cend();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::cbegin() const noexcept {
  const_iterator it;
  it.pos = const_cast<typename std::remove_const<T>::type *>(this->data());
  it.stride = this->stride();
  return it;
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::cend() const noexcept {
  const_iterator it = this->cbegin();
  it.pos = static_cast<char *>(it.pos) + it.stride * this->size();
  return it;
}

template <typename T>
Vec<T>::Vec(unsafe_bitcopy_t, const Vec &bits) noexcept : repr(bits.repr) {}
#endif // CXXBRIDGE1_RUST_VEC
} // namespace cxxbridge1
} // namespace rust

struct Key;
struct KvPair;
struct OptionalValue;
enum class Bound : ::std::uint8_t;
namespace tikv_client_glue {
  struct TransactionClient;
  struct Transaction;
}

#ifndef CXXBRIDGE1_STRUCT_Key
#define CXXBRIDGE1_STRUCT_Key
struct Key final {
  ::rust::Vec<::std::uint8_t> key;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_Key

#ifndef CXXBRIDGE1_STRUCT_KvPair
#define CXXBRIDGE1_STRUCT_KvPair
struct KvPair final {
  ::rust::Vec<::std::uint8_t> key;
  ::rust::Vec<::std::uint8_t> value;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_KvPair

#ifndef CXXBRIDGE1_STRUCT_OptionalValue
#define CXXBRIDGE1_STRUCT_OptionalValue
struct OptionalValue final {
  bool is_none;
  ::rust::Vec<::std::uint8_t> value;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_OptionalValue

#ifndef CXXBRIDGE1_ENUM_Bound
#define CXXBRIDGE1_ENUM_Bound
enum class Bound : ::std::uint8_t {
  Included = 0,
  Excluded = 1,
  Unbounded = 2,
};
#endif // CXXBRIDGE1_ENUM_Bound

namespace tikv_client_glue {
::rust::Box<::tikv_client_glue::TransactionClient> transaction_client_new(const ::std::vector<::std::string> &pd_endpoints);

::rust::Box<::tikv_client_glue::Transaction> transaction_client_begin(const ::tikv_client_glue::TransactionClient &client);

::rust::Box<::tikv_client_glue::Transaction> transaction_client_begin_pessimistic(const ::tikv_client_glue::TransactionClient &client);

::OptionalValue transaction_get(const ::tikv_client_glue::Transaction &transaction, const ::std::string &key);

::OptionalValue transaction_get_for_update(::tikv_client_glue::Transaction &transaction, const ::std::string &key);

::rust::Vec<::KvPair> transaction_batch_get(::tikv_client_glue::Transaction &transaction, const ::std::vector<::std::string> &keys);

::rust::Vec<::KvPair> transaction_batch_get_for_update(::tikv_client_glue::Transaction &transaction, const ::std::vector<::std::string> &keys);

::rust::Vec<::KvPair> transaction_scan(::tikv_client_glue::Transaction &transaction, const ::std::string &start, ::Bound start_bound, const ::std::string &end, ::Bound end_bound, ::std::uint32_t limit);

::rust::Vec<::Key> transaction_scan_keys(::tikv_client_glue::Transaction &transaction, const ::std::string &start, ::Bound start_bound, const ::std::string &end, ::Bound end_bound, ::std::uint32_t limit);

void transaction_put(::tikv_client_glue::Transaction &transaction, const ::std::string &key, const ::std::string &val);

void transaction_delete(::tikv_client_glue::Transaction &transaction, const ::std::string &key);

void transaction_commit(::tikv_client_glue::Transaction &transaction);
} // namespace tikv_client_glue
