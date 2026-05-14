#include "audio.h"
#include "config.h"
#include "state.h"
#include <driver/i2s.h>
#include <math.h>

static int32_t i2sBuf[I2S_BUFFER_LEN];

bool audioInit() {
  // ----- speaker (LEDC) -----
  ledcSetup(LEDC_CH_SPK, 1000, LEDC_RES_BITS);
  ledcAttachPin(PIN_SPK, LEDC_CH_SPK);
  ledcWrite(LEDC_CH_SPK, 0);

  // ----- I2S0 RX (INMP441) -----
  i2s_config_t cfg = {
    .mode             = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate      = SAMPLE_RATE_HZ,
    .bits_per_sample  = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format   = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count    = 4,
    .dma_buf_len      = I2S_BUFFER_LEN,
    .use_apll         = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk       = 0
  };
  i2s_pin_config_t pins = {
    .bck_io_num   = PIN_I2S_BCLK,
    .ws_io_num    = PIN_I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = PIN_I2S_DIN
  };
  if (i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL) != ESP_OK) return false;
  if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK)               return false;
  i2s_zero_dma_buffer(I2S_NUM_0);
  return true;
}

void audioSampleOnce() {
  size_t bytesRead = 0;
  if (i2s_read(I2S_NUM_0, (void*)i2sBuf, sizeof(i2sBuf), &bytesRead, pdMS_TO_TICKS(50)) != ESP_OK)
    return;
  size_t n = bytesRead / sizeof(int32_t);
  if (n == 0) return;

  // INMP441 is 24-bit left-justified in a 32-bit slot. Shift down.
  double sumSq = 0.0;
  for (size_t i = 0; i < n; i++) {
    int32_t v = i2sBuf[i] >> 8;          // to 24-bit signed
    sumSq += (double)v * (double)v;
  }
  double rms = sqrt(sumSq / n);
  // 24-bit full scale ~ 2^23
  double dbFs = 20.0 * log10(rms / 8388608.0 + 1e-12);
  // Empirical offset: INMP441 sensitivity ~-26 dBFS @ 94 dB SPL
  int16_t dbSpl = (int16_t)(dbFs + 120.0);   // rough calibration; tune in field
  dbSpl = constrain(dbSpl, 0, 140);
  stateUpdate([=](SensorState& s){ s.soundDb = dbSpl; });
}

void audioBeep(uint16_t hz, uint16_t ms) {
  ledcWriteTone(LEDC_CH_SPK, hz);
  ledcWrite(LEDC_CH_SPK, 64);   // ~25% duty
  vTaskDelay(pdMS_TO_TICKS(ms));
  ledcWriteTone(LEDC_CH_SPK, 0);
  ledcWrite(LEDC_CH_SPK, 0);
}

void audioStartupChime() {
  audioBeep(880,  80);
  audioBeep(1320, 80);
  audioBeep(1760, 120);
}
