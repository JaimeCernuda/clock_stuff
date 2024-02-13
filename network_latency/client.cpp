// Client Code for Timing, Printing, Calculations, and Triggering Shutdown
#include <thallium.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <iomanip> // For std::setw

namespace tl = thallium;

std::pair<std::string, std::ratio<1>> determineTimeUnit() {
  using namespace std::chrono;
  if (std::ratio_equal<system_clock::duration::period, nano>::value) {
    return {"ns", nano{}};
  } else if (std::ratio_equal<system_clock::duration::period, micro>::value) {
    return {"us", micro{}};
  } else if (std::ratio_equal<system_clock::duration::period, milli>::value) {
    return {"ms", milli{}};
  }
  return {"s", std::ratio<1>{}};
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <server address> <num tests> <sleep seconds>" << std::endl;
        return 1;
    }

    std::string serverAddress = argv[1];
    int numTests = std::stoi(argv[2]);
    int sleepSeconds = std::stoi(argv[3]);

    tl::engine clientEngine("tcp", THALLIUM_CLIENT_MODE);
    tl::endpoint server = clientEngine.lookup(serverAddress);
    tl::remote_procedure getTime = clientEngine.define("get_time");

    std::vector<long long> rtts, offsets;

  auto [unit, ratio] = determineTimeUnit();

  std::cout << "System clock precision: " << unit << std::endl;

  std::cout << std::left << std::setw(10) <<
    "Attempt" << std::setw(20) <<
    "Timestamp1" << std::setw(20) <<
    "Timestamp2" << std::setw(20) <<
    "Timestamp3" << std::setw(10) <<
    "RTT (" << unit << ")" << std::setw(10) <<
    "Offset (" << unit << ")" << std::endl;

  typedef std::chrono::duration<long long, decltype(ratio)> local_time;
  for (int i = 0; i < numTests; ++i) {
    auto t1 = std::chrono::system_clock::now();
    long long t2_raw = getTime.on(server)(); // Assuming this returns time in the same unit as Clock
    auto t2 = std::chrono::system_clock::time_point(local_time(t2_raw));
    auto t3 = std::chrono::system_clock::now();

    // Use the detected ratio for duration calculation
    auto rttDuration = std::chrono::duration_cast<local_time>(t3 - t1);
    auto offsetDuration = std::chrono::duration_cast<local_time>(t2 - t1 - rttDuration / 2);

    double rtt = rttDuration.count();
    double offset = offsetDuration.count();

    rtts.push_back(rtt);
    offsets.push_back(offset);

    std::cout << std::left << std::setw(10) << i + 1
              << std::setw(20) << std::chrono::duration_cast<local_time>(t1.time_since_epoch()).count()
              << std::setw(20) << std::chrono::duration_cast<local_time>(t2.time_since_epoch()).count()
              << std::setw(20) << std::chrono::duration_cast<local_time>(t3.time_since_epoch()).count()
              << std::setw(15) << rtt
              << std::setw(15) << offset
              << std::endl;

    if (i < numTests - 1) {
      std::this_thread::sleep_for(std::chrono::seconds(sleepSeconds));
    }
  }

    long long sumRtt = 0, sumOffset = 0;
    for (int i = 0; i < numTests; ++i) {
        sumRtt += rtts[i];
        sumOffset += offsets[i];
    }
    double avgRtt = double(sumRtt) / numTests;
    double avgOffset = double(sumOffset) / numTests;

    std::cout << "Average RTT: " << avgRtt << " ms, Average Offset: " << avgOffset << " ms" << std::endl;

    clientEngine.shutdown_remote_engine(server);
    std::cout << "Requested remote server shutdown." << std::endl;

    return 0;
}
