#include "Audio.h"
#include "BTypes.h"

Audio audio;
TFloat audio_volume = .5; // half way

#ifdef __XTENSA__

#define SAMPLE_RATE (22050)
#define TIMER_LENGTH 50
#define AUDIO_BUFF_SIZE 12
esp_timer_create_args_t timer_args;
esp_timer_handle_t timer;

#else
#define SAMPLE_RATE (44100)
#define AUDIO_BUFF_SIZE 500

#endif


/*** ODROID GO START *******/
#ifdef __XTENSA__

// #include "odroid_settings.h"
#define I2S_NUM (I2S_NUM_0)
#define BUILTIN_DAC_ENABLED (1)

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include <string.h> // For memset

TFloat Audio::GetVolume(){
  return audio_volume;
}

void Audio::SetVolume(TFloat value) {
  TFloat newValue = audio_volume + value;

  if (newValue > .124f) {
    audio_volume = 0;
  }
  else {
    audio_volume = newValue;
  }

  printf("audio_volume = %f\n", audio_volume);
}


Audio::Audio() {
//  mMuted = EFalse;
}

Audio::~Audio() {
}


void Audio::Init(TAudioDriverCallback aDriverCallback) {
  printf("Audio::%s\n", __func__);fflush(stdout);

  // NOTE: buffer needs to be adjusted per AUDIO_SAMPLE_RATE
  i2s_config_t i2s_config = {
    .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,               //2-channels
    .communication_format = I2S_COMM_FORMAT_PCM,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                //Interrupt level 1
    .dma_buf_count = 48,
    .dma_buf_len = 8,
    .use_apll = 0, //1
    .fixed_mclk = 1
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

  i2s_set_pin(I2S_NUM, NULL);
  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);

  mAudioBuffer = (short *)heap_caps_malloc(sizeof(short) * AUDIO_BUFF_SIZE, MALLOC_CAP_8BIT); // SPI RAM
  memset(mAudioBuffer, 0, AUDIO_BUFF_SIZE);
//  audio.MuteMusic();

  //*** CREATE TIMER ***//


//  timer_args.callback = &Audio::i2sTimerCallback;
  timer_args.callback = *aDriverCallback;
  timer_args.name = "audioTimer";
//  timer_args.arg = (void *)mAudioBuffer;

  esp_timer_create(&timer_args, &timer);
  esp_timer_start_periodic(timer, TIMER_LENGTH);
}


void Audio::Terminate() {
  i2s_zero_dma_buffer(I2S_NUM);
  i2s_stop(I2S_NUM);

  i2s_start(I2S_NUM);


  esp_err_t err = rtc_gpio_init(GPIO_NUM_25);
  err = rtc_gpio_init(GPIO_NUM_26);

  if (err != ESP_OK){
    abort();
  }

  err = rtc_gpio_set_direction(GPIO_NUM_25, RTC_GPIO_MODE_OUTPUT_ONLY);
  err = rtc_gpio_set_direction(GPIO_NUM_26, RTC_GPIO_MODE_OUTPUT_ONLY);

  if (err != ESP_OK) {
    abort();
  }

  err = rtc_gpio_set_level(GPIO_NUM_25, 0);
  err = rtc_gpio_set_level(GPIO_NUM_26, 0);
  if (err != ESP_OK) {
    abort();
  }
}


void Audio::Submit(TInt16* stereomAudioBuffer, int frameCount) {
  TInt16 currentAudioSampleCount = frameCount * 2;

  // Convert for built in DAC
  for (TInt16 i = 0; i < currentAudioSampleCount; i += 2){
    int32_t dac0;
    int32_t dac1;

    if (audio_volume == 0.0f || mMuted) {
      // Disable amplifier
      dac0 = 0;
      dac1 = 0;
    }
    else {
      // Down mix stero to mono
      int32_t sample = stereomAudioBuffer[i];
      sample += stereomAudioBuffer[i + 1];
      sample >>= 1;

      // Normalize
      const TFloat sn = (TFloat)sample / 0x8000;

      // Scale
      const int magnitude = 127 + 127;
      const TFloat range = magnitude  * sn * audio_volume;

      // Convert to differential output
      if (range > 127) {
        dac1 = (range - 127);
        dac0 = 127;
      }
      else if (range < -127) {
        dac1  = (range + 127);
        dac0 = -127;
      }
      else{
        dac1 = 0;
        dac0 = range;
      }

      dac0 += 0x80;
      dac1 = 0x80 - dac1;

      dac0 <<= 8;
      dac1 <<= 8;
    }

    stereomAudioBuffer[i] = (int16_t)dac1;
    stereomAudioBuffer[i + 1] = (int16_t)dac0;
  }


  int len = currentAudioSampleCount * sizeof(int16_t);
  // this replaces i2s_write_bytes
  size_t count;
  i2s_write(I2S_NUM, (const char *)stereomAudioBuffer, len, &count, portMAX_DELAY);
//  int count = i2s_write_bytes(I2S_NUM, (const char *)stereomAudioBuffer, len, portMAX_DELAY);

  if (count != len)   {
    printf("i2s_write_bytes: count (%d) != len (%d)\n", count, len);
    abort();
  }
}


/**** END ODROID GO ***/
#else 
/*** START Mac/Linux ***/

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>


Audio::Audio() {
}

Audio::~Audio() {
  SDL_CloseAudio();
}



void Audio::Init(TAudioDriverCallback aDriverCallback) {
  SDL_AudioSpec audioSpec;
  printf("Initializing SDL2 Audio\n");
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "sdl: can't initialize: %s\n", SDL_GetError());
    return;
  }

  audioSpec.freq = SAMPLE_RATE;
  audioSpec.format = AUDIO_S16;
  audioSpec.channels = 2;
  // Lots of samples so we can get accurate row counts!
  audioSpec.samples = 512;
  audioSpec.size = 100;
  audioSpec.callback = aDriverCallback;

  if (SDL_OpenAudio(&audioSpec, nullptr) < 0) {
    fprintf(stderr, "%s\n", SDL_GetError());
  }

//  printf("AUDIO SPEC: samples: %i | size: %i\n", audioSpec.samples, audioSpec.size);

}

void Audio::SetVolume(TFloat value) {

}
TFloat Audio::GetVolume() {
  return 0.0;
}


/**** END Mac/Linux ****/
#endif





