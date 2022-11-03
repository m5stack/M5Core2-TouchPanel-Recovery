
#include <M5Unified.h>

#include "ft6336_fw_vid0x11_fid0x11_app.h"
#include "ft6336_fw_vid0x01_fid0x01_app.h"

extern void ft6336_fw_updater(const uint8_t* array, size_t len);

static constexpr const uint8_t ft_i2c_addr = 0x38;
static constexpr const uint32_t ft_i2c_freq = 400000;

static constexpr uint8_t FT_LIB_VER_H_REG = 0xA1;
static constexpr uint8_t FT_LIB_VER_L_REG = 0xA2;
static constexpr uint8_t FT_CIPHER_REG    = 0xA3;
static constexpr uint8_t FT_FIRMID_REG    = 0xA6;
static constexpr uint8_t FT_VENDID_REG    = 0xA8;

auto &display = M5.Display;

static void drawCursor(int x, int y)
{
  x -= 4;
  display.fillTriangle(x+12, y, x , y + 5, x, y - 5);
}

static void drawMenu(int idx, int mode)
{
  static constexpr const char* menutxt[] = { "write new model FW (0x01)", "write first model FW (0x11)" };
  switch (mode)
  {
  case 0:
    display.setTextColor(TFT_DARKGRAY);
    display.setColor(TFT_BLACK);
    break;

  case 1:
    display.setTextColor(TFT_WHITE);
    display.setColor(TFT_YELLOW);
    break;

  default:
    display.setTextColor(TFT_YELLOW);
    display.setColor(TFT_BLACK);
    break;
  }

  // display.fillCircle(10, 150 + idx * 20, 8);
  drawCursor(10, 150 + idx * 20);
  display.setCursor(30, 142 + idx * 20);
  display.print(menutxt[idx]);
}

static void drawYesNo(int idx, int mode)
{
  static constexpr const char* menutxt[] = { "Cancel", "Execute" };
  switch (mode)
  {
  case 0:
    display.setTextColor(TFT_DARKGRAY);
    display.setColor(TFT_BLACK);
    break;

  case 1:
    display.setTextColor(TFT_WHITE);
    display.setColor(TFT_YELLOW);
    break;

  default:
    display.setTextColor(TFT_YELLOW);
    display.setColor(TFT_GREEN);
    break;
  }

  // display.fillCircle(50 + idx * 120, 200, 8);
  drawCursor(50 + idx * 120, 200);
  display.setCursor(70 + idx * 120, 192);
  display.print(menutxt[idx]);
}

void setup(void)
{
  M5.begin();
  M5.Display.setFont(&fonts::DejaVu12);

  if (M5.Display.getBoard() != m5gfx::board_M5StackCore2)
  {
    M5.Display.print("\n\nThis program is only compatible with M5Stack Core2.");
    for (;;) { delay(100); }
  }

  uint8_t ft_lib_ver_h = M5.In_I2C.readRegister8(ft_i2c_addr, FT_LIB_VER_H_REG, ft_i2c_freq);
  uint8_t ft_lib_ver_l = M5.In_I2C.readRegister8(ft_i2c_addr, FT_LIB_VER_L_REG, ft_i2c_freq);
  uint8_t ft_cipher    = M5.In_I2C.readRegister8(ft_i2c_addr, FT_CIPHER_REG   , ft_i2c_freq);
  uint8_t ft_firmid    = M5.In_I2C.readRegister8(ft_i2c_addr, FT_FIRMID_REG   , ft_i2c_freq);
  uint8_t ft_vendid    = M5.In_I2C.readRegister8(ft_i2c_addr, FT_VENDID_REG   , ft_i2c_freq);

  display.setTextColor(TFT_LIGHTGRAY); 
  display.setCursor(0, 0);
  display.print("TouchScreen Information\n");
  display.printf("regA1 LIB Version: \n");
  display.printf("regA3 ChipVendorID:\n");
  display.printf("regA6 Firmware ID :\n");
  display.printf("regA8 CTPMVendorID:\n");

  display.setTextColor(TFT_YELLOW); 
  display.setClipRect(220, 0, 100, 240);
  display.setCursor(220, 0);
  display.printf("\n0x%04x\n", ft_lib_ver_h << 8 | ft_lib_ver_l);
  display.printf(" 0x%02x\n", ft_cipher);
  display.printf(" 0x%02x\n", ft_firmid);
  display.printf(" 0x%02x\n", ft_vendid);

  display.clearClipRect();
  display.setTextColor(TFT_WHITE); 
  display.setCursor(60, 88);
  display.print("please use power button\n");
  display.setCursor(58, display.getCursorY());
  display.print("single click : Move cursor\n");
  display.setCursor(60, display.getCursorY());
  display.print("press 1sec : Confirm\n");

  display.setTextColor(TFT_WHITE); 

  M5.Display.setFont(&fonts::DejaVu18);

  drawMenu(0, true);
  drawMenu(1, false);
  // drawYesNo(0, false);
  // drawYesNo(1, false);
  M5.Speaker.setVolume(24);
  M5.update();
}

enum menu_mode_t {
  mode_select_fw,
  mode_select_yesno,
};

menu_mode_t mode = mode_select_fw;
uint8_t cursor_index = 0;
uint8_t yesno_index = 0;

void loop(void)
{
  delay(14);
  M5.update();

  switch (mode) {
  case menu_mode_t::mode_select_fw:
    if (M5.BtnPWR.wasClicked()) {
      M5.Speaker.tone(880, 50);
      drawMenu(cursor_index, false);
      if (++cursor_index >= 2) {
        cursor_index = 0;
      }
      drawMenu(cursor_index, true);
    }
    if (M5.BtnPWR.wasHold()) {
      M5.Speaker.tone(1760, 100);
      drawMenu(cursor_index, 2);
      mode = menu_mode_t::mode_select_yesno;
      yesno_index = 0;
      drawYesNo(0, true);
      drawYesNo(1, false);
    }
    break;

  case menu_mode_t::mode_select_yesno:
    if (M5.BtnPWR.wasClicked()) {
      M5.Speaker.tone(880, 50);
      drawYesNo(yesno_index, false);
      yesno_index = !yesno_index;
      drawYesNo(yesno_index, true);
    }
    if (M5.BtnPWR.wasHold()) {
      M5.Speaker.tone(1760, 100);
      drawYesNo(yesno_index, 2);
      if (yesno_index) {
        display.clear();
        display.setCursor(0, 0);
        switch (cursor_index) {
        case 0:
          ft6336_fw_updater(ft6336_fw_vid0x01_fid0x01_app, sizeof(ft6336_fw_vid0x01_fid0x01_app));
          break;

        case 1:
          ft6336_fw_updater(ft6336_fw_vid0x11_fid0x11_app, sizeof(ft6336_fw_vid0x11_fid0x11_app));
          break;
        }
        M5.Power.deepSleep(500, false);
      }
      else{
        drawYesNo(0, false);
        drawYesNo(1, false);
        mode = menu_mode_t::mode_select_fw;
        drawMenu(cursor_index, true);
      }
    }
    break;
  }
  m5gfx::touch_point_t tp[2];

  int nums = M5.Display.getTouch(tp, 2);
  for (int i = 0; i < nums; ++i)
  {
    M5.Display.fillCircle(tp[i].x, tp[i].y, tp[i].size + 2, rand());
  }
}
