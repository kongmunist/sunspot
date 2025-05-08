/**********************************************************************
 *  XIAO-ESP32S3  ──>  SSDV-over-Serial demo
 *
 *  – Captures a QVGA JPEG frame
 *  – Encodes it into 256-byte SSDV packets with RS(255,223) FEC
 *  – Streams the packets out of the USB-CDC port at 921 600 bps
 *
 *  Packet format is byte-for-byte identical to the HAB SSDV spec,
 *  so any existing ground-station decoder will work unchanged.
 *
 *  Olivia Li – 2025-05-06
 *********************************************************************/
#include "esp_camera.h"
#include "ssdv.h"          // fsphil SSDV encoder (C)
#include "camera_pins.h"

// --------------------------- USER CONFIG ---------------------------
#define CAMERA_MODEL_XIAO_ESP32S3
static const char  CALLSIGN[7] = "KC1UQM";   // 6 chars + '\0'
static uint8_t     image_id    = 0;         // rolls over at 255
const uint32_t     DATA_BAUD   = 921600;    // USB-CDC can handle this
// -------------------------------------------------------------------

static uint8_t pkt[SSDV_PKT_SIZE];          // 256-byte packet buffer
static ssdv_t  ssdv;                        // encoder state

void setup() {
  // 1) High-speed serial for the packets
  Serial.begin(DATA_BAUD);
  Serial.setDebugOutput(false);

  // 2) Camera config (identical to your last working sketch)
  camera_config_t cfg = {
    .ledc_channel   = LEDC_CHANNEL_0,
    .ledc_timer     = LEDC_TIMER_0,
    .pin_d0         = Y2_GPIO_NUM,  .pin_d1 = Y3_GPIO_NUM,
    .pin_d2         = Y4_GPIO_NUM,  .pin_d3 = Y5_GPIO_NUM,
    .pin_d4         = Y6_GPIO_NUM,  .pin_d5 = Y7_GPIO_NUM,
    .pin_d6         = Y8_GPIO_NUM,  .pin_d7 = Y9_GPIO_NUM,
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_pclk       = PCLK_GPIO_NUM,
    .pin_vsync      = VSYNC_GPIO_NUM,
    .pin_href       = HREF_GPIO_NUM,
    .pin_sccb_sda   = SIOD_GPIO_NUM,
    .pin_sccb_scl   = SIOC_GPIO_NUM,
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,
    .xclk_freq_hz   = 20'000'000,
    .frame_size     = FRAMESIZE_QVGA,          // 320×240
    .pixel_format   = PIXFORMAT_JPEG,
    .fb_location    = CAMERA_FB_IN_PSRAM,
    .grab_mode      = CAMERA_GRAB_LATEST,
    .jpeg_quality   = 12, .fb_count = 2
  };
  if (!psramFound()) {                       // degrade gracefully
    cfg.frame_size = FRAMESIZE_SVGA;
    cfg.fb_location = CAMERA_FB_IN_DRAM;
    cfg.fb_count = 1;
  }
  if (esp_camera_init(&cfg) != ESP_OK) {
    Serial.println("Camera init failed");    // binary stream not started yet
    while (true) delay(1000);
  }
  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {             // aesthetic tweaks
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
}

void encode_and_send(camera_fb_t *fb) {
  /* 1.  Initialise encoder for this image */
  ssdv_enc_init(&ssdv, (uint8_t*)CALLSIGN, image_id, SSDV_TYPE_STANDARD);
  ssdv_enc_set_buffer(&ssdv, pkt);

  size_t fed = 0;
  const uint8_t *jpeg = fb->buf;
  size_t len  = fb->len;

  /* 2.  Produce packets until EOI */
  while (true) {
    int st = ssdv_enc_get_packet(&ssdv);
    if (st == SSDV_OK) {                     // a full 256-byte pkt ready
      Serial.write(pkt, SSDV_PKT_SIZE);
    }
    else if (st == SSDV_FEED_ME) {           // encoder wants more jpeg data
      size_t chunk = (len - fed) > 128 ? 128 : (len - fed);
      if (chunk == 0) break;                 // shouldn't happen
      ssdv_enc_feed(&ssdv, jpeg + fed, chunk);
      fed += chunk;
    }
    else if (st == SSDV_EOI) {               // done!
      break;
    }
    else {                                   // SSDV_ERROR
      Serial.printf("SSDV err %d\n", st);
      break;
    }
  }
  image_id++;                                // increment for next frame
}

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;

  encode_and_send(fb);                       // SSDV encoding + TX
  esp_camera_fb_return(fb);                  // free the framebuffer

  /* A QVGA frame ~15 kB → ~90 SSDV pkts (≈23 kB).  At 921 600 bps
     that’s ~200 ms of airtime; 1 s pause keeps  ~4 fps.  */
  delay(1000);
}
