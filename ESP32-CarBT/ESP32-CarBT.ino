#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

void setup() {
 
    i2s_pin_config_t pin_config = {
      .bck_io_num = 23,
      .ws_io_num = 26,
      .data_out_num = 25,
      .data_in_num = I2S_PIN_NO_CHANGE
    };
    a2dp_sink.set_pin_config(pin_config);
    static i2s_config_t i2s_config = {
      .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100, // updated automatically by A2DP
      .bits_per_sample = (i2s_bits_per_sample_t)32,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = true,
      .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
    };
    a2dp_sink.set_i2s_config(i2s_config);
    a2dp_sink.start("Honda");
}
 
void loop() // is quasi-interrupt routine
{

}
