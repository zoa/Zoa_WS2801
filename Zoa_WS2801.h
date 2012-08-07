#ifndef ADAFRUIT_WS2801_H
#define ADAFRUIT_WS2801_H



#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

// Not all LED pixels are RGB order; 36mm type expects GRB data.
// Optional flag to constructors indicates data order (default if
// unspecified is RGB).  As long as setPixelColor/getPixelColor are
// used, other code can always treat 'packed' colors as RGB; the
// library will handle any required translation internally.
#define WS2801_RGB 0
#define WS2801_GRB 1

class Adafruit_WS2801 {

 public:

  // Configurable pins:
  Adafruit_WS2801(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order=WS2801_RGB);
  // Use SPI hardware; specific pins only:
  Adafruit_WS2801(uint16_t n, uint8_t order=WS2801_RGB);
  // Empty constructor; init pins/strand length/data order later:
  Adafruit_WS2801();
  // Release memory (as needed):
  ~Adafruit_WS2801();

  void
    begin(void),
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    updatePins(uint8_t dpin, uint8_t cpin), // Change pins, configurable
    updatePins(void), // Change pins, hardware SPI
    updateLength(uint16_t n), // Change strand length
    updateOrder(uint8_t order); // Change data order
  uint16_t
    numPixels(void);
  uint32_t
    getPixelColor(uint16_t n);

 protected:

  uint16_t
    numLEDs;
  uint8_t
    *pixels,   // Holds color values for each LED (3 bytes each)
    rgb_order, // Color order; RGB vs GRB (or others, if needed in future)
    clkpin    , datapin,     // Clock & data pin numbers
    clkpinmask, datapinmask; // Clock & data PORT bitmasks
  volatile uint8_t
    *clkport  , *dataport;   // Clock & data PORT registers
  void
    alloc(uint16_t n, byte*& arr),
    startSPI(void);
  boolean
    hardwareSPI, // If 'true', using hardware SPI
    begun;       // If 'true', begin() method was previously invoked
};


///////////////////////////////////////////////////////////////////////////

struct RGB_color
{
  byte r,g,b;
  RGB_color( byte R, byte G, byte B) : r(R), g(G), b(B) {}
  void set( byte R, byte G, byte B ) { r=R; g=G; b=B; }
};


// Child class containing methods specific to Zoa project
class Zoa_WS2801 : public Adafruit_WS2801
{
public:
    // Configurable pins:
  Zoa_WS2801(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order=WS2801_RGB);
  // Use SPI hardware; specific pins only:
  Zoa_WS2801(uint16_t n, uint8_t order=WS2801_RGB);
  // Empty constructor; init pins/strand length/data order later:
  Zoa_WS2801();
  // Release memory (as needed):
  ~Zoa_WS2801();
  
  // Whether to scale the values before displaying. Defaults to true.
  void set_scaling( bool on );
  
  // This is exactly like the Adafruit show method, but scales the values
  // before displaying. (We don't want to scale them sooner because then we'd
  // have to unscale them in getPixelColor, and there isn't always a 1:1 mapping
  // between scaled and unscaled values.)
  void show();
  
  // Get the R/G/B components. Not tested yet. color argument must 
  // have size of 3.
  void getPixelRGBColor( uint16_t n, byte (&color)[3] );
  
  // Prepends the color to the pixels array and shifts all other values
  // to accommodate it.
  void pushFront( byte r, byte g, byte b );
  
  // Prints the scaled equivalents of values [0,255] to serial
  // (call Serial.begin before calling this)
  void scalingTest();
  
private:
  // Returns value from the scaledValues array at index value
  uint8_t scaleValue( uint8_t value );
  
  // Fills the scaled values array using a cube-root function (with some
  // coefficients added by trial and error to make it look more linear)
  void initialize();
  
  bool scaled;
  
  // Lookup array for the scaled values
  byte scaledValues[256]; 
  
  // Used by show() to hold the scaled values to be passed to the parent's
  // show method
  byte* scaledPixelBuffer;
};



#endif