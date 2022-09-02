
#include "client.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>

#include <esp_log.h>
#include <fmt/format.h>
#include <asio.hpp>

#include "board_configs.h"
#include "uart.h"

namespace mmrr::client {

namespace {

class Message {
 public:
  static constexpr int kMaxBodyLen = 512;
  static constexpr int kHeaderLen  = 4;

  Message() : body_length_(0) {}

  const std::byte* data() const { return data_.data(); }

  std::byte* data() { return data_.data(); }

  std::size_t length() const { return kHeaderLen + body_length_; }

  const std::byte* body() const { return data_.data() + kHeaderLen; }

  std::byte* body() { return data_.data() + kHeaderLen; }

  std::size_t body_length() const { return body_length_; }

  void body_length(std::size_t new_length) {
    body_length_ = new_length;
    if (body_length_ > kMaxBodyLen)
      body_length_ = kMaxBodyLen;
  }

  bool DecodeHeader() {
    std::array<std::byte, kHeaderLen + 1> header{std::byte{0}};
    std::copy_n(data_.cbegin(), kHeaderLen, header.begin());
    body_length_ = std::atoi(reinterpret_cast<char*>(header.data()));
    if (body_length_ > kMaxBodyLen) {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void EncodeHeader() {
    std::array<std::byte, kHeaderLen + 1> header{std::byte{0}};
    std::sprintf(reinterpret_cast<char*>(header.data()), "%4d", static_cast<int>(body_length_));
    std::copy_n(header.begin(), kHeaderLen, data_.begin());
  }

 private:
  std::array<std::byte, kHeaderLen + kMaxBodyLen> data_;
  std::size_t body_length_;
};

constexpr const char* kTag = "Client";

using asio::ip::tcp;

using ChatMessageQueue = std::deque<Message>;

class Client {
 public:
  Client(asio::io_context* io_context, const tcp::resolver::results_type& endpoints)
      : io_context_(*io_context), socket_(*io_context) {
    DoConnect(endpoints);
  }

  void Write(const Message& msg) {
    asio::post(io_context_, [this, msg]() {
      bool is_write_in_progress = !write_msgs_.empty();
      write_msgs_.push_back(msg);
      if (!is_write_in_progress) {
        DoWrite();
      }
    });
  }

  void Close() {
    asio::post(io_context_, [this]() { socket_.close(); });
  }

 private:
  void DoConnect(const tcp::resolver::results_type& endpoints) {
    asio::async_connect(socket_, endpoints, [this](std::error_code ec, tcp::endpoint) {
      if (!ec) {
        DoReadHeader();
      }
    });
  }

  void DoReadHeader() {
    asio::async_read(socket_,
                     asio::buffer(read_msg_.data(), Message::kHeaderLen),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec && read_msg_.DecodeHeader()) {
                         DoReadBody();
                       } else {
                         socket_.close();
                       }
                     });
  }

  void DoReadBody() {
    asio::async_read(socket_,
                     asio::buffer(read_msg_.body(), read_msg_.body_length()),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec) {
                         //  std::cout.write(read_msg_.body(), read_msg_.body_length());
                         //  std::cout << "\n";
                         DoReadHeader();
                       } else {
                         socket_.close();
                       }
                     });
  }

  void DoWrite() {
    asio::async_write(socket_,
                      asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                      [this](std::error_code ec, std::size_t /*length*/) {
                        if (!ec) {
                          write_msgs_.pop_front();
                          if (!write_msgs_.empty()) {
                            DoWrite();
                          }
                        } else {
                          socket_.close();
                        }
                      });
  }

 private:
  asio::io_context& io_context_;
  tcp::socket socket_;
  Message read_msg_;
  ChatMessageQueue write_msgs_;
};

// Se
void TaskIoContextClient(void* pv_io_context) {
  asio::io_context& io_context = *static_cast<asio::io_context*>(pv_io_context);
  io_context.run();
}

void TaskClient(void* ignore) {
  try {
    asio::io_context io_context;

    tcp::resolver resolver(io_context);

    auto ip_read   = mmrr::uart::Read();
    auto endpoints = resolver.resolve(ip_read.c_str(), kPort);
    Client client(&io_context, endpoints);

    xTaskCreate(TaskIoContextClient,
                "TaskCliCtx",
                10 * configMINIMAL_STACK_SIZE,
                static_cast<void*>(&io_context),
                5,
                nullptr);

    while (true) {
      Message msg;
      auto read_msg = uart::Read();
      msg.body_length(read_msg.size());
      std::copy(read_msg.begin(), read_msg.end(), reinterpret_cast<char*>(msg.body()));
      client.Write(msg);
    }
  } catch (std::exception& e) {
    ESP_LOGE(kTag, "%s", fmt::format("Exception: {}", e.what()).c_str());
  }
}

}  // namespace

void Init() {
  xTaskCreatePinnedToCore(
      TaskClient, "TaskClient", configMINIMAL_STACK_SIZE * 10, nullptr, 5, nullptr, APP_CPU_NUM);
}

}  // namespace mmrr::client