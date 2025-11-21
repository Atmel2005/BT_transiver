#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"

BluetoothA2DPSink a2dp_sink;

// Пины I2S для MAX98357
// BCLK  -> GPIO 26
// LRCLK -> GPIO 25
// DIN   -> GPIO 22

void audio_callback(const uint8_t *data, uint32_t len) {
  // Пишем принятые PCM-данные напрямую в I2S
  size_t written = 0;
  i2s_write((i2s_port_t)0, data, len, &written, portMAX_DELAY);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // Конфигурация I2S для MAX98357: стерео, 16 бит, 44.1 кГц
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 128,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = 26,          // BCLK
      .ws_io_num = 25,           // LRCLK
      .data_out_num = 22,        // DIN к MAX98357
      .data_in_num = I2S_PIN_NO_CHANGE
  };

  // Инициализация I2S0
  esp_err_t err = i2s_driver_install((i2s_port_t)0, &i2s_config, 0, nullptr);
  if (err != ESP_OK) {
    Serial.printf("i2s_driver_install failed: %d\n", err);
  }
  err = i2s_set_pin((i2s_port_t)0, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("i2s_set_pin failed: %d\n", err);
  }

  // Используем только наш коллбэк, отключаем внутренний I2S-вывод библиотеки
  a2dp_sink.set_stream_reader(audio_callback, false);

  a2dp_sink.start("ESP32_BT_SPEAKER");
}

void loop() {
}