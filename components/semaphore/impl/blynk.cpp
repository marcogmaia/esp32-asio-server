#include "mmrr/semaphore.h"

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPLjcp1K8-H"
#define BLYNK_DEVICE_NAME "semaforo"
#define BLYNK_AUTH_TOKEN "0AQ4beYVEUaOgY6NVKSxb1dylKrd8BoK"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <esp_log.h>

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiType.h>

namespace {

constexpr const char *kAuthToken = BLYNK_AUTH_TOKEN;
constexpr const char *kTag       = "Semaforo";
constexpr const char *kSsid      = "maia";
constexpr const char *kPass      = "marco3445";

mmrr::semaphore::SemaphoreTiming semaphore_timing{};

bool buzzer_state    = false;
int buzzer_frequency = 128;
int car_counter      = 0;

BlynkTimer timer;

void UpdateCarCounter() {
  Blynk.virtualWrite(V5, car_counter);
}

void TaskBlynk(void *ignore) {
  // Debug console
  Serial.begin(115200);
  Blynk.begin(kAuthToken, kSsid, kPass);
  ESP_LOGI(kTag, "Blynk started.");

  timer.setInterval(1000, UpdateCarCounter);

  while (true) {
    Blynk.run();
    timer.run();
    // timer.run();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

}  // namespace

// vermelho
BLYNK_WRITE(V0) {
  semaphore_timing.red = param.asInt();
}
// amarelo
BLYNK_WRITE(V1) {
  semaphore_timing.yellow = param.asInt();
}
// verde
BLYNK_WRITE(V2) {
  semaphore_timing.green = param.asInt();
}
// buzzer
BLYNK_WRITE(V3) {
  int value    = param.asInt();
  buzzer_state = static_cast<bool>(value);
  ESP_LOGI(kTag, "Buzzer state: %d", value);
}
// buzzer frequency
BLYNK_WRITE(V4) {
  buzzer_frequency = param.asInt();
  ESP_LOGI(kTag, "Buzzer frequency: %d", buzzer_frequency);
}
// buzzer frequency
// BLYNK_WRITE(V5) {
//   Blynk.virtualWrite(V5, )
//   // buzzer_frequency = param.asInt();
//   // ESP_LOGI(kTag, "Buzzer frequency: %d", buzzer_frequency);
// }

namespace mmrr::semaphore {

namespace detail {

void InitBlynk() {
  xTaskCreatePinnedToCore(
      TaskBlynk, "TaskBlynk", 20 * configMINIMAL_STACK_SIZE, nullptr, 5, nullptr, APP_CPU_NUM);
}

}  // namespace detail

SemaphoreTiming GetSemaphoreTiming() {
  return semaphore_timing;
}

bool IsBuzzerOn() {
  return buzzer_state;
}

int GetBuzzerFrequency() {
  return buzzer_frequency;
}

bool IsBuzzerStateChanged() {
  static bool last_state    = buzzer_state;
  static int last_frequency = buzzer_frequency;
  // Changed.
  if (last_state != buzzer_state) {
    last_state = buzzer_state;
    return true;
  }

  if (last_frequency != buzzer_frequency) {
    last_frequency = buzzer_frequency;
    return true;
  }

  return false;
}

int AddCarCounter() {
  return ++car_counter;
}

}  // namespace mmrr::semaphore