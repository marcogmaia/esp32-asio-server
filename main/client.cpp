#include "client.h"

#include <asio.hpp>
#include <iostream>
#include <memory>
#include <utility>

namespace {

using asio::ip::tcp;

}

void Session::DoRead() {
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(data_, kMaxLen),
                          [this, self](std::error_code ec, std::size_t length) {
                            if (!ec) {
                              DoWrite(length);
                            }
                          });
}

void Session::DoWrite(std::size_t length) {
  auto self(shared_from_this());
  asio::async_write(socket_,
                    asio::buffer(data_, length),
                    [this, self](std::error_code ec, std::size_t /*length*/) {
                      if (!ec) {
                        DoRead();
                      }
                    });
}

void Server::DoAccept() {
  acceptor_.async_accept([this](std::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<Session>(std::move(socket))->Start();
    }
    DoAccept();
  });
}

int main(int argc, char* argv[]) {
  try {
    asio::io_context io_context;
    constexpr uint16_t port = 54321;

    Server s(&io_context, port);

    io_context.run();
  } catch (std::exception& e) {
    ESP_LOGE("Asio", "Exception: %s", e.what());
  }

  return 0;
}