#pragma once

#include <memory>

#include <esp_log.h>
#include <asio.hpp>

class Session : public std::enable_shared_from_this<Session> {
  using tcp = asio::ip::tcp;

 public:
  Session(tcp::socket socket) : socket_(std::move(socket)) {}

  void Start() {
    ESP_LOGI("Asio", "Session started.");
    DoRead();
  }

 private:
  void DoRead();

  void DoWrite(std::size_t length);

  tcp::socket socket_;
  static constexpr int kMaxLen = 1024;
  char data_[kMaxLen];
};

class Server {
  using tcp = asio::ip::tcp;

 public:
  Server(asio::io_context* io_context, short port)
      : acceptor_(*io_context, tcp::endpoint(tcp::v4(), port)) {
    DoAccept();
  }

 private:
  void DoAccept();

  tcp::acceptor acceptor_;
};
