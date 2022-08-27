//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.h"

#include <exception>
#include <iostream>
#include <memory>
#include <utility>

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include "board_configs.h"

namespace {

using asio::ip::tcp;
using Json = nlohmann::json;

}  // namespace

void Server::DoAccept() {
  acceptor_.async_accept(socket_, [this](std::error_code ec) {
    if (!ec) {
      std::make_shared<Session>(std::move(socket_))->Start();
    }
    DoAccept();
  });
}

Server::Server(asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context) {
  DoAccept();
}

void Session::DoRead() {
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(data_, kMaxLen),
                          [this, self](std::error_code ec, std::size_t length) {
                            if (!ec) {
                              std::string str(data_, length);
                              Json json;
                              json["str"] = str;
                              Blink();
                              ESP_LOGI("Asio", "Message received: %s", json.dump(2).c_str());
                              DoWrite(length);
                            }
                          });
}

void Session::DoWrite(std::size_t length) {
  auto self(shared_from_this());
  asio::async_write(socket_,
                    asio::buffer(data_, length),
                    [this, self, length](std::error_code ec, std::size_t /*length*/) {
                      if (!ec) {
                        std::string message(data_, length);
                        std::string_view m(data_, length);
                        if (!message.empty()) {
                          ESP_LOGI("Asio", "Message written: %s", m.data());
                        }
                        DoRead();
                      }
                    });
}