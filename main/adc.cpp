
#include <cstring>

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "mmrr/configs.h"
#include "queue.h"

namespace mmrr::adc {

namespace {

constexpr int kDefaultVref = 1100;  // Use adc2_vref_to_gpio() to obtain a better estimate
constexpr int kNoOfSamples = 32;    // Multisampling

esp_adc_cal_characteristics_t adc_chars;

}  // namespace

constexpr const char* kTag = "Adc";

static void check_efuse(void) {
  // Check if TP is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
    printf("eFuse Two Point: Supported\n");
  } else {
    printf("eFuse Two Point: NOT supported\n");
  }
  // Check Vref is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
    printf("eFuse Vref: Supported\n");
  } else {
    printf("eFuse Vref: NOT supported\n");
  }
}

void TaskAdc(void* ignore) {
  while (true) {
    uint32_t adc_reading = 0;
    // Multisampling
    for (int i = 0; i < kNoOfSamples; i++) {
      if (kAdcUnit != ADC_UNIT_1) {
        ESP_LOGE(kTag, "Adc must be unit 1");
      }
      adc_reading += adc1_get_raw((adc1_channel_t)kAdcChannel);
    }

    adc_reading /= kNoOfSamples;
    // Convert adc_reading to voltage in mV
    // uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

    // Send the value read to the queue.
    xQueueOverwrite(queue::queue_adc, &adc_reading);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void PrintCharsValType(esp_adc_cal_value_t val_type) {
  switch (val_type) {
    case ESP_ADC_CAL_VAL_EFUSE_VREF: {
      printf("Characterized using eFuse Vref\n");
      break;
    }
    case ESP_ADC_CAL_VAL_EFUSE_TP: {
      printf("Characterized using Two Point Value\n");
      break;
    }
    case ESP_ADC_CAL_VAL_DEFAULT_VREF: {
      printf("Characterized using Default Vref\n");
      break;
    }
    case ESP_ADC_CAL_VAL_EFUSE_TP_FIT: {
      printf("Characterized using two points fit\n");
      break;
    }
    default: {
      ESP_LOGE(kTag, "Unknown characterization type.");
      break;
    }
  }
}

void Init() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  queue::Init();
  // xQueuePeek(mmrr::queue::queue_adc, &value_read, portMAX_DELAY);
  constexpr uint32_t kAdcInitialValue = 0;
  xQueueOverwrite(queue::queue_adc, &kAdcInitialValue);

  check_efuse();

  if (kAdcUnit == ADC_UNIT_1) {
    adc1_config_width(kAdcWidth);
    adc1_config_channel_atten(static_cast<adc1_channel_t>(kAdcChannel), kAdcAttenuation);
  } else {
    ESP_LOGE(kTag, "Adc must be unit 1");
  }

  // Characterize ADC
  std::memset(&adc_chars, 0, sizeof adc_chars);

  esp_adc_cal_value_t val_type =
      esp_adc_cal_characterize(kAdcUnit, kAdcAttenuation, kAdcWidth, kDefaultVref, &adc_chars);
  PrintCharsValType(val_type);

  xTaskCreatePinnedToCore(
      TaskAdc, "TaskAdc", configMINIMAL_STACK_SIZE * 2, nullptr, 5, nullptr, PRO_CPU_NUM);
}

uint32_t Read() {
  uint32_t value_read = 0;
  xQueuePeek(mmrr::queue::queue_adc, &value_read, portMAX_DELAY);
  return value_read;
}

extern "C" void AdcExampleMain(void) {
  Init();
}

}  // namespace mmrr::adc