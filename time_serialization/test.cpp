#include <cereal/archives/binary.hpp>
#include <cereal/types/chrono.hpp>
#include <sstream>
#include <chrono>
#include <iostream>
#include <fstream>

// Serialize a time_point to a string
template <typename Clock>
std::string serializeTimePoint(const std::chrono::time_point<Clock>& now) {
  std::stringstream ss;
  cereal::BinaryOutputArchive archive(ss);
  archive(now);
  return ss.str();
}

// Deserialize a time_point from a string
template <typename Clock>
std::chrono::time_point<Clock> deserializeTimePoint(const std::string& data) {
  std::stringstream ss(data);
  std::chrono::time_point<Clock> now;
  cereal::BinaryInputArchive archive(ss);
  archive(now);
  return now;
}

int main() {
  auto nowSystem = std::chrono::system_clock::now();
  auto serializedSystem = serializeTimePoint(nowSystem);
  auto nowSystemDeserialized = deserializeTimePoint<std::chrono::system_clock>(serializedSystem);

  if (nowSystem == nowSystemDeserialized) {
    std::cout << "Deserialization successful for system_clock, time points match." << std::endl;
  } else {
    std::cout << "Deserialization failed for system_clock, time points do not match." << std::endl;
  }

  std::string filename = "timepoint.dat";
  std::ofstream os(filename, std::ios::binary);
  os << serializedSystem;
  os.close();

  std::ifstream is(filename, std::ios::binary);
  std::string receivedData((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
  is.close();

  auto now2 = deserializeTimePoint<std::chrono::system_clock>(receivedData);

  if (nowSystem == now2) {
    std::cout << "Deserialization successful with file." << std::endl;
  } else {
    std::cout << "Deserialization failed with file." << std::endl;
  }

  // Example for steady_clock
  auto nowSteady = std::chrono::steady_clock::now();
  auto serializedSteady = serializeTimePoint(nowSteady);
  auto nowSteadyDeserialized = deserializeTimePoint<std::chrono::steady_clock>(serializedSteady);

  return 0;
}
