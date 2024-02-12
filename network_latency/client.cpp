// Client Code for Timing, Printing, Calculations, and Triggering Shutdown
#include <thallium.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <thread>

namespace tl = thallium;

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

    for (int i = 0; i < numTests; ++i) {
        auto t1 = std::chrono::system_clock::now();
        long long t2 = getTime.on(server)();
        auto t3 = std::chrono::system_clock::now();

        auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1).count();
        auto offset = t2 - std::chrono::duration_cast<std::chrono::milliseconds>(t1.time_since_epoch()).count() - rtt / 2;

        rtts.push_back(rtt);
        offsets.push_back(offset);

        std::cout << "Attempt " << i + 1 << ": RTT = " << rtt << " ms, Offset = " << offset << " ms" << std::endl;

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
