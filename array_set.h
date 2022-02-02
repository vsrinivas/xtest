#include <vector>

template <typename T>
class array_set {
 public:
  void insert(const T& value) {
    if (count(value) == 0) {
      data_.push_back(value);
    }
  }
  size_t count(const T& key) {
    for (typename std::vector<T>::iterator it = data_.begin(); it != data_.end(); ++it) {
      if (*it == key) {
        return 1;
      }
    }
    return 0;
  }
  void erase(const T& key) {
    for (typename std::vector<T>::iterator it = data_.begin(); it != data_.end(); ++it) {
      if (*it == key) {
	data_.erase(it);
	return;
      }
    }
  }
  typename std::vector<T>::iterator begin() { return data_.begin(); }
  typename std::vector<T>::iterator end() { return data_.end(); }

 private:
  std::vector<T> data_;
};
