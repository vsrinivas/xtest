#include <functional>

class PWQ {
 public:
  static void Run(std::function<void()> fn);
  static void Flush();
};
