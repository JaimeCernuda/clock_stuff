#include <chrono>

int main() {
  for (int i = 0; i < 10; ++i) {
    auto now = std::chrono::steady_clock::now();
  }
  return 0;
}