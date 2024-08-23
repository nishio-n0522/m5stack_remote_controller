#include <lvgl.h>
#include <M5Unified.h>
#include <IRremote.h>

/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES   320
#define TFT_VER_RES   240
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

// Set constant value
// remote controller for room lighting
const int address = 0x5284;
const int lightOnCommand = 0x6A;
const int lightOffCommand = 0x7C;
const int repeatNum = 2;

// GPIO
const int irEmitter = 26;

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
  uint32_t width = (area->x2 - area->x1 + 1);
  uint32_t height = (area->y2 - area->y1 + 1);
  lv_draw_sw_rgb565_swap(px_map, width*height);
  M5.Display.pushImageDMA<uint16_t>(area->x1, area->y1, width, height, (uint16_t *)px_map);
  lv_display_flush_ready(disp);
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

void setup()
{
    auto cfg = M5.config();

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.begin( 115200 );
    // Serial.begin(9600);
    IrSender.begin(irEmitter);
    Serial.println( LVGL_Arduino );

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick);

    lv_display_t * disp;
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, nullptr, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_obj_t *label = lv_label_create( lv_screen_active() );
    lv_label_set_text( label, "I'm lighting switch" );
    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

    Serial.println( "Setup done" );
}

void loop()
{

    M5.update();
    lv_timer_handler(); /* let the GUI do its work */
    delay(5); /* let this time pass */

    if (M5.BtnA.wasPressed()) {
      IrSender.sendNEC2(address, lightOnCommand, repeatNum);
    }

    if(M5.BtnB.wasPressed()) { 
      IrSender.sendNEC2(address, lightOffCommand, repeatNum);
    }

    if (M5.BtnC.wasPressed()) {
      M5.Power.powerOff();
    }
}