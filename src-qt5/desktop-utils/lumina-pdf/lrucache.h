#ifndef LUMINA_PDF_LRUCACHE_H
#define LUMINA_PDF_LRUCACHE_H

#include <list>
#include <optional>
#include <tuple>
#include <unordered_map>

namespace LuminaPDF {
template <class Value, class Key = int> class LRUCache {
public:
  LRUCache() = default;
  ~LRUCache() = default;

  void setCacheSize(size_t _max) {
    max = _max;
    hashmap.reserve(max);
    return;
  }

  std::optional<Value> get(const Key k) {
    auto itr = hashmap.find(k);

    if (itr == hashmap.end()) {
      return std::nullopt;
    }

    Value v = std::get<1>(*std::get<1>(*itr));

    values.erase(std::get<1>(*itr));
    values.push_front(std::make_tuple(k, v));
    hashmap[k] = values.begin();
    return v;
  }

  void push(const Key k, const Value v) {
    while (hashmap.size() >= max) {
      Key _k = std::get<0>(values.back());
      values.pop_back();
      hashmap.erase(_k);
    }

    values.push_front(std::make_tuple(k, v));
    hashmap[k] = values.begin();
  }

  bool contains(const Key k){
    auto itr = hashmap.find(k);
    return ( itr!=hashmap.end() );
  }

private:
  size_t max;
  std::list<std::tuple<Key, Value>> values;
  std::unordered_map<Key, typename std::list<std::tuple<Key, Value>>::iterator>
      hashmap;
};
} // namespace LuminaPDF

#endif // LUIMINA_PDF_LRUCACHE_H
