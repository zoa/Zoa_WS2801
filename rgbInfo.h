#ifndef RGBINFO_H
#define RGBINFO_H

typedef struct rgbInfo {
  rgbInfo() : r(0), g(0), b(0) {}
  rgbInfo(byte r_in, byte g_in, byte b_in) : r(r_in), g(g_in), b(b_in) {}
  byte r;
  byte g;
  byte b;
} rgbInfo_t;

#endif