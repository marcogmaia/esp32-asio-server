#include "alarm/alarm.h"

#include <array>
#include <cassert>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board_configs.h"

namespace mmrr::alarm {

namespace {

constexpr auto* kTag = "Alarm";

template <typename GpioArray>
constexpr uint64_t ComputeBitMask(GpioArray gpio_array) {
  uint64_t bitmask = 0;
  for (auto& gpio : gpio_array) {
    if (gpio >= 0) {
      bitmask |= BIT(gpio);
    }
  }
  return bitmask;
}

void ConfigGpio(uint64_t gpio_bitmask, gpio_mode_t mode) {
  const gpio_config_t config{.pin_bit_mask = gpio_bitmask,
                             .mode         = mode,
                             .pull_up_en   = gpio_pullup_t::GPIO_PULLUP_DISABLE,
                             .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
                             .intr_type    = gpio_int_type_t::GPIO_INTR_DISABLE};
  gpio_config(&config);
}

void InitializeSegments() {
  constexpr std::array<gpio_num_t, 7> segment_pins{
      kPinSegA,
      kPinSegB,
      kPinSegC,
      kPinSegD,
      kPinSegE,
      kPinSegF,
      kPinSegG,
  };
  constexpr auto segs_bitmask = ComputeBitMask(segment_pins);
  ConfigGpio(segs_bitmask, gpio_mode_t::GPIO_MODE_OUTPUT);
}

template <typename T>
constexpr bool IsNumberInRange(T num, T low, T high) {
  return low <= num && num <= high;
}

void ShowDigit(int digit) {
  if (!IsNumberInRange(digit, 0, 9)) {
    ESP_LOGE(kTag, "Digit out of bounds.");
    return;
  }
  constexpr int num_array[10][7] = {{1, 1, 1, 1, 1, 1, 0},   // 0
                                    {0, 1, 1, 0, 0, 0, 0},   // 1
                                    {1, 1, 0, 1, 1, 0, 1},   // 2
                                    {1, 1, 1, 1, 0, 0, 1},   // 3
                                    {0, 1, 1, 0, 0, 1, 1},   // 4
                                    {1, 0, 1, 1, 0, 1, 1},   // 5
                                    {1, 0, 1, 1, 1, 1, 1},   // 6
                                    {1, 1, 1, 0, 0, 0, 0},   // 7
                                    {1, 1, 1, 1, 1, 1, 1},   // 8
                                    {1, 1, 1, 0, 0, 1, 1}};  // 9
  gpio_set_level(kPinSegA, num_array[digit][0]);
  gpio_set_level(kPinSegB, num_array[digit][1]);
  gpio_set_level(kPinSegC, num_array[digit][2]);
  gpio_set_level(kPinSegD, num_array[digit][3]);
  gpio_set_level(kPinSegE, num_array[digit][4]);
  gpio_set_level(kPinSegF, num_array[digit][5]);
  gpio_set_level(kPinSegG, num_array[digit][6]);
}

void TurnOffDigits() {
  gpio_set_level(kPinSegA, 0);
  gpio_set_level(kPinSegB, 0);
  gpio_set_level(kPinSegC, 0);
  gpio_set_level(kPinSegD, 0);
  gpio_set_level(kPinSegE, 0);
  gpio_set_level(kPinSegF, 0);
  gpio_set_level(kPinSegG, 0);
}

enum class State {
  Initial,
  ActivateAlarm,
  Detect,
  Password,
  End,
};

constexpr const char* GetStateString(State state) {
  // clang-format off
  switch (state) {
    case State::Initial:       return "Initial";
    case State::ActivateAlarm: return "ActivateAlarm";
    case State::Detect:        return "Detect";
    case State::Password:      return "Password";
    case State::End:           return "End";
    default: assert(false);
  }
  // clang-format on
}

void PrintState(State state) {
  ESP_LOGI(kTag, "Estado: %s", GetStateString(state));
}

void TwoBeeps(int delay_ms) {
  gpio_set_level(kPinBuzzer, 1);
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
  gpio_set_level(kPinBuzzer, 0);
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
  gpio_set_level(kPinBuzzer, 1);
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
  gpio_set_level(kPinBuzzer, 0);
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

void Fsm(void* ignore) {
  static State actual_state = State::Initial;
  PrintState(actual_state);
  vTaskDelay(pdMS_TO_TICKS(1000));

  switch (actual_state) {
    case State::Initial: {
      // Aqui a gente quer ler apenas se o botao foi pressionado.
      bool is_pin_pressed = gpio_get_level(kPinButton) == 1;
      ESP_LOGI(kTag, "Pressed: %d", is_pin_pressed);
      if (is_pin_pressed) {
        actual_state = State::ActivateAlarm;
      }
      break;
    }

    case State::ActivateAlarm: {
      // Pisca o led por 10 segundo.
      for (size_t i = 0; i < 20; ++i) {
        static bool led_state = true;
        gpio_set_level(kPinLed, static_cast<uint8_t>(led_state));
        led_state = !led_state;
        vTaskDelay(pdMS_TO_TICKS(200));  // espera 1s na realidade paralela
      }
      gpio_set_level(kPinLed, 0);  // Desliga.
      actual_state = State::Detect;
      break;
    };

    case State::Detect: {
      // bool is_movement_detected = digitalRead(pin_sensor_movement) == HIGH;
      bool is_movement_detected   = false;
      static bool is_light_detect = false;
      if (analogRead(kPinLdr) < 100) {
        is_light_detect = true;
      }
      bool buzzer_should_activate = is_movement_detected || is_light_detect;
      ESP_LOGI(kTag, "Movimento: %d, Luz: %d.", is_movement_detected, is_light_detect);
      if (buzzer_should_activate) {
        // Alguém entrou na casa, o buzzer vai explodir.
        TwoBeeps(1000);
        actual_state    = State::Password;
        is_light_detect = false;
      }
      break;
    }

    case State::Password: {
      static int last_time = millis();
      static int timer     = 0;
      auto actual_time     = millis();
      auto dt              = actual_time - last_time;
      timer += dt;
      last_time                  = actual_time;
      static bool is_last_chance = false;
      static int counter         = 10;

      // Simplesmente altera o 7 segmentos.
      if (timer > 1000) {
        --counter;
        timer   = 0;                          // reseta timer.
        counter = counter < 0 ? 0 : counter;  // impede do counter de ser negativo.
        ShowDigit(counter);
      }

      bool is_valid_password = false;

      // Lê o password
      if (Serial.available()) {
        Serial.println("Input");
        int read_password = Serial.read();
        is_valid_password = CheckPassword(read_password);
        TurnOffDigits();
        counter = 10;

        if (is_valid_password) {
          is_last_chance = false;
          actual_state   = State::ActivateAlarm;
          break;
        } else if (!is_last_chance) {
          is_last_chance = true;
          break;
        } else {
          actual_state = State::End;
          break;
        }
      }

      // Caso de falha
      if (counter == 0) {
        TurnOffDigits();
        counter = 10;
        if (is_last_chance) {
          actual_state = State::End;
          break;
        }
        is_last_chance = true;
        break;
      }

      break;
    };

    case State::End: {
      // Ativa o pega-ladrão
      gpio_set_level(pin_buzzer, HIGH);
      gpio_set_level(kPinLed, HIGH);
      break;
    };

    default:
      break;
  }
}

}  // namespace

void Init() {
  // Init guard -- init only once.
  bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  ESP_LOGI(kTag, "Initializing the alarm.");

  InitializeSegments();

  ESP_LOGI(kTag, "Alarm initialized.");
}

}  // namespace mmrr::alarm