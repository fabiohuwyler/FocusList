#ifndef THEME_H
#define THEME_H

#include <pebble.h>

#ifdef PBL_COLOR
  #define COLOR_BG GColorFromHEX(0xF5EBCB)
  #define COLOR_TEXT GColorFromHEX(0x222222)
  #define COLOR_MUTED GColorFromHEX(0x77705F)
  #define COLOR_ACCENT GColorFromHEX(0x4A6B43)
  #define COLOR_OVERDUE GColorFromHEX(0xA63B32)
  #define COLOR_COMPLETE GColorFromHEX(0x5B8C4A)
  #define COLOR_LINE GColorFromHEX(0xB8AF96)
#else
  #define COLOR_BG GColorWhite
  #define COLOR_TEXT GColorBlack
  #define COLOR_MUTED GColorBlack
  #define COLOR_ACCENT GColorBlack
  #define COLOR_OVERDUE GColorBlack
  #define COLOR_COMPLETE GColorBlack
  #define COLOR_LINE GColorBlack
#endif

#endif
