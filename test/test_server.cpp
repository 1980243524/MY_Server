#include <chatserver.h>
#include <iostream>
auto main() -> int {
  std::vector<std::thread> ths;
  try {
    boost::asio::io_context io;
    ChatServer server(io, 8080);
    server.Init("root", "593509663", "serverdb", 3306, 5);
    server.Start();
    for (int i = 0; i < 5; i++) {
      ths.emplace_back([&] { io.run(); });
    }
    io.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}