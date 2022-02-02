#include <vector>

template <typename T>
class array_sset {
 public:
  void insert(const T& value) {
    if (count(value) == 0) {
      auto it = std::lower_bound(data_.begin(), data_.end(), value);
      data_.insert(it, value);
    }
  }
  size_t count(const T& key) {
    if (std::binary_search(data_.begin(), data_.end(), key)) return 1;
    return 0;
  }
  void erase(const T& key) {
    if (count(key) == 0) return;
    auto pr = std::equal_range(data_.begin(), data_.end(), key);
    data_.erase(pr.first, pr.second);
  }
  typename std::vector<T>::iterator begin() { return data_.begin(); }
  typename std::vector<T>::iterator end() { return data_.end(); }

 private:
  std::vector<T> data_;
};
