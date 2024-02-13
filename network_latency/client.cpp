// Client Code for Timing, Printing, Calculations, and Triggering Shutdown
#include <thallium.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <ratio>
#include <iomanip> // For std::setw
#include <numeric>

namespace tl = thallium;

std::string determineTimeUnit() {
  using namespace std::chrono;
  if (std::ratio_equal<system_clock::duration::period, std::nano>::value) {
    return "ns";
  } else if (std::ratio_equal<system_clock::duration::period, std::micro>::value) {
    return "us";
  } else if (std::ratio_equal<system_clock::duration::period, std::milli>::value) {
    return "ms";
  }
  return "s";
}

typedef std::chrono::system_clock::duration local_time;
void calculateAndPrint(std::vector<local_time>& times, std::string marker, float range=1.5) {
  if (times.empty()) return;

  std::vector<long long> values;
  std::transform(times.begin(), times.end(), std::back_inserter(values), [](const local_time& time) {
    return time.count();
  });

  // Sorting to easily find median and potentially use for IQR based approach
  std::sort(values.begin(), values.end());
  long long median = values[values.size() / 2];

  // Simple outlier detection based on a multiplier of the median
  const long long outlierThreshold = median * range;
  std::vector<long long> filteredValues, outliers;

  for (auto value : values) {
    if (value <= outlierThreshold) {
      filteredValues.push_back(value);
    } else {
      outliers.push_back(value);
    }
  }

  // Calculating average of filtered values
  double avg = std::accumulate(filteredValues.begin(), filteredValues.end(), 0LL) / double(filteredValues.size());

  // Printing average and outliers
  std::cout << "Average " << marker <<": " << avg << std::endl;
  std::cout << "Discarded measurements: ";
  for (auto outlier : outliers) {
    std::cout << outlier << " ";
  }
  std::cout << std::endl;
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


  auto unit = determineTimeUnit();
  std::cout << "System clock precision: " << unit << std::endl;

  std::cout << std::left << std::setw(10) <<
    "Attempt" << std::setw(20) <<
    "Timestamp1" << std::setw(20) <<
    "Timestamp2" << std::setw(20) <<
    "Timestamp3" << std::setw(10) <<
    "RTT (" << unit << ")" << std::setw(10) <<
    "Offset (" << unit << ")" << std::endl;

  typedef std::chrono::system_clock::duration local_time;
  std::vector<local_time> rtts, offsets;

  for (int i = 0; i < numTests; ++i) {
    auto t1 = std::chrono::system_clock::now();
    int64_t t2_raw = getTime.on(server)(); // Assuming this returns time in the same unit as Clock
    auto t2 = std::chrono::system_clock::time_point(local_time(t2_raw));
    auto t3 = std::chrono::system_clock::now();

    // Use the detected ratio for duration calculation
    local_time rttDuration = std::chrono::duration_cast<local_time>(t3 - t1);
    local_time offsetDuration = std::chrono::duration_cast<local_time>(t2 - t1 - rttDuration / 2);

    auto rtt = rttDuration.count();
    auto offset = offsetDuration.count();

    rtts.push_back(rttDuration);
    offsets.push_back(offsetDuration);

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

  calculateAndPrint(rtts, "RTT");
//  calculateAndPrint(offsets, "Offset");

    std::cout << "Average Offset: " << std::reduce(offsets.begin(), offsets.end()).count() << " ms" << std::endl;

    clientEngine.shutdown_remote_engine(server);
    std::cout << "Requested remote server shutdown." << std::endl;

    return 0;
}
