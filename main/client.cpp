
#include "client.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>

#include <esp_log.h>
#include <fmt/format.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <asio.hpp>

#include "board_configs.h"
#include "queue.h"
#include "uart.h"

namespace mmrr::client {

namespace {

using namespace mmrr::queue;

void SendToQueuePassword(bool is_password_correct) {
  xQueueOverwrite(queue_password, &is_password_correct);
}

class Message {
 public:
  static constexpr int kMaxBodyLen = 512;
  static constexpr int kHeaderLen  = 4;

  Message() = default;

  Message(std::string&& str) {
    char* data_begin = data_.data();
    if (str.size() > kMaxBodyLen) {
      std::copy_n(str.cbegin(), kMaxBodyLen, data_begin);
      size_ = kMaxBodyLen;
    } else {
      std::copy(str.cbegin(), str.cend(), data_begin);
      size_ = str.size();
    }
  }

  char* data() { return data_.data(); }
  const char* data() const { return data_.data(); }

  std::size_t size() const { return size_; }

  Message& operator=(std::string& str) {
    char* data_begin = data_.data();
    if (str.size() > kMaxBodyLen) {
      std::copy_n(str.cbegin(), kMaxBodyLen, data_begin);
      size_ = kMaxBodyLen;
    } else {
      std::copy(str.cbegin(), str.cend(), data_begin);
      size_ = str.size();
    }
    return *this;
  }

  Message& operator=(std::string&& str) { return operator=(str); }

 private:
  std::array<char, kMaxBodyLen> data_;
  std::size_t size_{0};
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
        ESP_LOGI(kTag, "Asio connected.");
        DoRead();
      }
    });
  }

  void DoRead() {
    asio::async_read(socket_,
                     asio::buffer(message_.data(), Message::kMaxBodyLen),
                     asio::transfer_at_least(4),
                     [this](std::error_code ec, std::size_t rx_len) {
                       if (!ec) {
                         Blink();
                         std::string_view message_received(message_.data(), rx_len);

                         bool is_password_correct = message_received[0] != 'E';  // "ERROR_PASSWORD"
                         SendToQueuePassword(is_password_correct);

                         ESP_LOGI(
                             kTag, "%s", fmt::format("Received: {}.", message_received).c_str());
                         DoRead();
                       } else {
                         socket_.close();
                       }
                     });
  }

  void DoWrite() {
    asio::async_write(socket_,
                      asio::buffer(write_msgs_.front().data(), write_msgs_.front().size()),
                      [this](std::error_code ec, std::size_t /*length*/) {
                        if (!ec) {
                          write_msgs_.pop_front();
                        } else {
                          socket_.close();
                        }
                      });
  }

 private:
  asio::io_context& io_context_;
  tcp::socket socket_;
  Message message_;
  ChatMessageQueue write_msgs_;
};

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

    // espera chegar a requisição de enviar o password.
    while (xSemaphoreTake(semaphore_password, portMAX_DELAY) == pdTRUE) {
      Message message{uart::Read()};  // Blocking read.
      client.Write(message);
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