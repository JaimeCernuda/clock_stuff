// Server Code
#include <thallium.hpp>
#include <iostream>

namespace tl = thallium;

int main() {
  tl::engine serverEngine("tcp", THALLIUM_SERVER_MODE);
  std::cout << "Server running at " << serverEngine.self() << std::endl;

  serverEngine.enable_remote_shutdown();
  serverEngine.push_prefinalize_callback([&]() {
    std::cout << "Finalizing operations before shutdown." << std::endl;
  });

  serverEngine.define("get_time", [](const tl::request &req) {
    req.respond(std::chrono::system_clock::now().time_since_epoch().count());
  });

  serverEngine.wait_for_finalize();

  std::cout << "Server is shutting down." << std::endl;
}
