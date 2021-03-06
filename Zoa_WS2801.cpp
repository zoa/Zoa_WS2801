#include "SPI.h"
#include "Zoa_WS2801.h"


//////////////////////////////////////////////////////////////
// Zoa_WS2801
//////////////////////////////////////////////////////////////

Zoa_WS2801::Zoa_WS2801(uint16_t n, uint8_t order) : Adafruit_WS2801(n,order)
{
  initialize();
}

//////////////////////////////////////////////////////////////

Zoa_WS2801::Zoa_WS2801(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order)
 : Adafruit_WS2801(n,dpin,cpin,order)
{
  initialize();
}

//////////////////////////////////////////////////////////////

void Zoa_WS2801::initialize()
{
  scaled = true;
  for ( uint16_t i = 0; i < 256; ++i )
  {
    scaledValues[i] = scaleValue(i);
  }
}

//////////////////////////////////////////////////////////////

void Zoa_WS2801::set_scaling( bool is_on )
{
  scaled = is_on;
}

//////////////////////////////////////////////////////////////

void Zoa_WS2801::show()
{
  // This mostly duplicates the code of the Adafruit show method, which
  // i don't like doing, but keeps us from having to modify the original library or waste
  // memory on having a separate scaled buffer
  uint16_t i, nl3 = numLEDs * 3; // 3 bytes per LED
  uint8_t  bit;

  // Write 24 bits per pixel:
  if(hardwareSPI) {
    for(i=0; i<nl3; i++) {
      byte val = scaled ? scaledValues[ pixels[i] ] : pixels[i];
      SPDR = val;
      while(!(SPSR & (1<<SPIF)));
    }
  } else {
    for(i=0; i<nl3; i++ ) {
      for(bit=0x80; bit; bit >>= 1) {
        if(pixels[i] & bit) *dataport |=  datapinmask;
        else                *dataport &= ~datapinmask;
        *clkport |=  clkpinmask;
        *clkport &= ~clkpinmask;
      }
    }
  }

  delay(1); // Data is latched by holding clock pin low for 1 millisecond
}

//////////////////////////////////////////////////////////////

rgbInfo_t Zoa_WS2801::getPixelRGBColor( uint16_t n )
{
  rgbInfo_t color;
  if (n < numLEDs) {
    uint16_t ofs = n*3;
    switch ( rgb_order )
    {
      case WS2801_BGR:
	color.r = pixels[ofs+2];
	color.g = pixels[ofs+1];
	color.b = pixels[ofs];
	break;
      case WS2801_GRB:
	color.r = pixels[ofs+1];
	color.g = pixels[ofs];
	color.b = pixels[ofs+2];
	break;
      case WS2801_RGB:
	color.r = pixels[ofs];
	color.g = pixels[ofs+1];
	color.b = pixels[ofs+2];
	break;
    }
  } else {
    color.r = 0;
    color.g = 0;
    color.b = 0;
  }
  return color;
}

//////////////////////////////////////////////////////////////

uint8_t Zoa_WS2801::scaleValue( uint8_t value ) {
 return value / ( pow(255,(1/3)) - pow(value,(1/3)) + 1 );
}

//////////////////////////////////////////////////////////////

void Zoa_WS2801::pushFront( rgbInfo_t color )
{
  byte* last = pixels + numLEDs*3 - 1;
  for ( byte* i = last - 3; i >= pixels; --i, --last )
  {
    *last = *i;
  }
  setPixelColor(0,color.r,color.g,color.b);
}

//////////////////////////////////////////////////////////////

void Zoa_WS2801::pushBack( rgbInfo_t color )
{
  byte* last = pixels + numLEDs*3;
  byte* prev = pixels;
  for ( byte* i = pixels+3; i < last; ++i, ++prev )
  {
    *prev = *i;
  }
  setPixelColor(numLEDs-1,color.r,color.g,color.b);
}


//////////////////////////////////////////////////////////////

void Zoa_WS2801::setAll( rgbInfo_t color )
{
  setPixelColor(0,color.r,color.g,color.b);
  byte* end = pixels + numLEDs*3;
  byte* prev = pixels;
  for ( byte* i = pixels + 3; i < end; ++i, ++prev )
  {
    *i = *prev;
  }
}

//////////////////////////////////////////////////////////////

void Zoa_WS2801::scalingTest() {
  for ( byte i = 0; i < 256; ++i ) {
    Serial.println( "Original value: " + String(i) + ", scaled value: " + String( scaleValue(i) ) );
  }
}



// Example to control WS2801-based RGB LED Modules in a strand or strip
// Written by Adafruit - MIT license
/*****************************************************************************/

// Constructor for use with hardware SPI (specific clock/data pins):
Adafruit_WS2801::Adafruit_WS2801(uint16_t n, uint8_t order) {
  rgb_order = order;
  alloc(n,pixels,numLEDs);
  updatePins();
}

// Constructor for use with arbitrary clock/data pins:
Adafruit_WS2801::Adafruit_WS2801(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order) {
  rgb_order = order;
  alloc(n,pixels,numLEDs);
  updatePins(dpin, cpin);
}

// Allocate 3 bytes per pixel, init to RGB 'off' state, count stored in cnt
void Adafruit_WS2801::alloc(uint16_t n, byte*& arr, uint16_t& cnt) {
  // The original Adafruit library just initialized the pixels array all the time here;
  // added an array argument so base classes could use it to initialize other arrays too
  begun   = false;
  cnt = ((arr = (uint8_t *)calloc(n, 3)) != NULL) ? n : 0;
}

// via Michael Vogt/neophob: empty constructor is used when strand length
// isn't known at compile-time; situations where program config might be
// read from internal flash memory or an SD card, or arrive via serial
// command.  If using this constructor, MUST follow up with updateLength()
// and updatePins() to establish the strand length and output pins!
// Also, updateOrder() to change RGB vs GRB order (RGB is default).
Adafruit_WS2801::Adafruit_WS2801(void) {
  begun     = false;
  numLEDs   = 0;
  pixels    = NULL;
  rgb_order = WS2801_RGB;
  updatePins(); // Must assume hardware SPI until pins are set
}

// Release memory (as needed):
Adafruit_WS2801::~Adafruit_WS2801(void) {
  if (pixels != NULL) {
    free(pixels);
  }
}

// Activate hard/soft SPI as appropriate:
void Adafruit_WS2801::begin(void) {
  if(hardwareSPI == true) {
    startSPI();
  } else {
    pinMode(datapin, OUTPUT);
    pinMode(clkpin , OUTPUT);
  }
  begun = true;
}

// Change pin assignments post-constructor, switching to hardware SPI:
void Adafruit_WS2801::updatePins(void) {
  hardwareSPI = true;
  datapin     = clkpin = 0;
  // If begin() was previously invoked, init the SPI hardware now:
  if(begun == true) startSPI();
  // Otherwise, SPI is NOT initted until begin() is explicitly called.

  // Note: any prior clock/data pin directions are left as-is and are
  // NOT restored as inputs!
}

// Change pin assignments post-constructor, using arbitrary pins:
void Adafruit_WS2801::updatePins(uint8_t dpin, uint8_t cpin) {

  if(begun == true) { // If begin() was previously invoked...
    // If previously using hardware SPI, turn that off:
    if(hardwareSPI == true) SPI.end();
    // Regardless, now enable output on 'soft' SPI pins:
    pinMode(dpin, OUTPUT);
    pinMode(cpin, OUTPUT);
  } // Otherwise, pins are not set to outputs until begin() is called.

  // Note: any prior clock/data pin directions are left as-is and are
  // NOT restored as inputs!

  hardwareSPI = false;
  datapin     = dpin;
  clkpin      = cpin;
  clkport     = portOutputRegister(digitalPinToPort(cpin));
  clkpinmask  = digitalPinToBitMask(cpin);
  dataport    = portOutputRegister(digitalPinToPort(dpin));
  datapinmask = digitalPinToBitMask(dpin);
}

// Enable SPI hardware and set up protocol details:
void Adafruit_WS2801::startSPI(void) {
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV16); // 1 MHz max, else flicker
}

uint16_t Adafruit_WS2801::numPixels(void) {
  return numLEDs;
}

// Change strand length (see notes with empty constructor, above):
void Adafruit_WS2801::updateLength(uint16_t n) {
  if(pixels != NULL) free(pixels); // Free existing data (if any)
  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numLEDs = ((pixels = (uint8_t *)calloc(n, 3)) != NULL) ? n : 0;
  // 'begun' state does not change -- pins retain prior modes
}

// Change RGB data order (see notes with empty constructor, above):
void Adafruit_WS2801::updateOrder(uint8_t order) {
  rgb_order = order;
  // Existing LED data, if any, is NOT reformatted to new data order.
  // Calling function should clear or fill pixel data anew.
}

void Adafruit_WS2801::show(void) {
  uint16_t i, nl3 = numLEDs * 3; // 3 bytes per LED
  uint8_t  bit;

  // Write 24 bits per pixel:
  if(hardwareSPI) {
    for(i=0; i<nl3; i++) {
      SPDR = pixels[i];
      while(!(SPSR & (1<<SPIF)));
    }
  } else {
    for(i=0; i<nl3; i++ ) {
      for(bit=0x80; bit; bit >>= 1) {
        if(pixels[i] & bit) *dataport |=  datapinmask;
        else                *dataport &= ~datapinmask;
        *clkport |=  clkpinmask;
        *clkport &= ~clkpinmask;
      }
    }
  }

  delay(1); // Data is latched by holding clock pin low for 1 millisecond
}

// Set pixel color from separate 8-bit R, G, B components:
void Adafruit_WS2801::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // See notes later regarding color order
    switch ( rgb_order )
    {
      case WS2801_BGR:
	*p++ = b;
	*p++ = g;
	*p++ = r;
	break;
      case WS2801_GRB:
	*p++ = g;
	*p++ = r;
	*p++ = b;
	break;
      case WS2801_RGB:
	*p++ = r;
	*p++ = g;
	*p++ = b;
	break;
    }
  }
}

// Set pixel color from 'packed' 32-bit RGB value:
void Adafruit_WS2801::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    
    switch ( rgb_order )
    {
      case WS2801_BGR:
	*p++ = c; // Red
	*p++ = c >>  8; // Green
	*p++ = c >> 16;       // Blue
	break;
      case WS2801_GRB:
	*p++ = c >>  8; // Green
	*p++ = c >> 16; // Red
	*p++ = c;       // Blue
	break;
      case WS2801_RGB:
	*p++ = c >> 16; // Red
	*p++ = c >>  8; // Green
	*p++ = c;       // Blue
	break;
    }
  }
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Adafruit_WS2801::getPixelColor(uint16_t n) {
  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    switch ( rgb_order )
    {
      case WS2801_BGR:
	return pixels[ofs + 2] | (pixels[ofs + 1] <<  8) | ((uint32_t)pixels[ofs] << 16);
      case WS2801_GRB:
	return (pixels[ofs] <<  8) | ((uint32_t)pixels[ofs + 1] << 16) | pixels[ofs + 2];
      case WS2801_RGB:
	return ((uint32_t)pixels[ofs] << 16) | (pixels[ofs + 1] <<  8) | pixels[ofs + 2];
    }
  }

  return 0; // Pixel # is out of bounds
}



