#include <Arduino.h>
#include <FastLed.h>

namespace L {
typedef ::byte bit8x8[8]; 

const bit8x8 a={B00111100,
                 B01000010,
                 B01000010,
                 B01111110,
                 B01000010,
                 B01000010,
                 B01000010,
                 B01000010};
const bit8x8 b={B01111100,
                 B01000010,
                 B01000010,
                 B01111100,
                 B01000010,
                 B01000010,
                 B01000010,
                 B01111100};
const bit8x8 c={B00111110,B01000000,B01000000,B01000000,B01000000,B01000000,B01000000,B00111110};
const bit8x8 d={B01111100,B01000010,B01000010,B01000010,B01000010,B01000010,B01000010,B01111100};
const bit8x8 e={B01111110,B01000000,B01000000,B01111000,B01000000,B01000000,B01000000,B01111110};
const bit8x8 f={B01111110,B01000000,B01000000,B01111000,B01000000,B01000000,B01000000,B01000000};
const bit8x8 g={B00111100,B01000010,B01000010,B01000000,B01001110,B01000010,B01000010,B00111100};
const bit8x8 h={B01000010,B01000010,B01000010,B01111110,B01000010,B01000010,B01000010,B01000010};
const bit8x8 i={B01111100,B00010000,B00010000,B00010000,B00010000,B00010000,B00010000,B01111100};
const bit8x8 j={B00000010,B00000010,B00000010,B00000010,B00000010,B00000010,B01000010,B00111100};
const bit8x8 k={B01000100,B01001000,B01010000,B01100000,B01010000,B01001000,B01000100,B01000010};
const bit8x8 l={B01000000,B01000000,B01000000,B01000000,B01000000,B01000000,B01000000,B01111110};
const bit8x8 m={B01000010,B01100110,B01011010,B01000010,B01000010,B01000010,B01000010,B01000010};
const bit8x8 n={B01000010,B01100010,B01010010,B01001010,B01000110,B01000010,B01000010,B01000010};
const bit8x8 o={B00111100,B01000010,B01000010,B01000010,B01000010,B01000010,B01000010,B00111100};
const bit8x8 p={B00111100,B01000010,B01000010,B01111100,B01000000,B01000000,B01000000,B01000000};
const bit8x8 q={B00111100,B01000010,B01000010,B01000010,B01000010,B01001010,B01000100,B00111011};
const bit8x8 r={B00111100,B01000010,B01000010,B01111100,B01010000,B01001000,B01000100,B01000100};
const bit8x8 s={B00111100,B01000010,B01000000,B00111100,B00000010,B00000010,B01000010,B00111100};
const bit8x8 t={B01111100,B00010000,B00010000,B00010000,B00010000,B00010000,B00010000,B00010000};
const bit8x8 u={B01000010,B01000010,B01000010,B01000010,B01000010,B01000010,B01000010,B00111100};
const bit8x8 v={B01000010,B01000010,B01000010,B01000010,B01000010,B01000010,B00100100,B00011000};
const bit8x8 w={B01000010,B01000010,B01000010,B01000010,B01000010,B01011010,B01100110,B01000010};
const bit8x8 x={B01000010,B01000010,B00100100,B00011000,B00011000,B00100100,B01000010,B01000010};
const bit8x8 y={B01000100,B01000100,B00101000,B00010000,B00010000,B00010000,B00010000,B00010000};
const bit8x8 z={B01111110,B00000100,B00001000,B00010000,B00100000,B01000000,B01000000,B01111110};
const bit8x8 _error={B00000000,B00000000,B00001000,B00010000,B00000000,B00000000,B00000000,B00000000};

const bit8x8& get_letter(char letter) {
  switch(letter) {
    case('a'): return a;
    case('b'): return b;
    case('c'): return c;
    case('d'): return d;
    case('e'): return e;
    case('f'): return f;
    case('g'): return g;
    case('h'): return h;
    case('i'): return i;
    case('j'): return j;
    case('k'): return k;
    case('l'): return l;
    case('m'): return m;
    case('n'): return n;
    case('o'): return o;
    case('p'): return p;
    case('q'): return q;
    case('r'): return r;
    case('s'): return s;
    case('t'): return t;
    case('u'): return u;
    case('v'): return v;
    case('x'): return x;
    case('w'): return w;
    case('y'): return y;
    case('z'): return z;
  }
  return _error;
}

}

inline CHSV background() {
  constexpr float BACKGROUND_SPEED = 1.05;
  static float hue = 0;
  hue = hue > 255 ? 0 : hue + BACKGROUND_SPEED;
  // Serial << hue << " - " << static_cast<uint8_t>(hue) << endl;
  return CHSV(static_cast<uint8_t>(hue), 255, 50);
}

void paint_letter(const L::bit8x8 &letter, CRGB& set(int, int)) {
  auto foreground = CRGB::Green;
  for (int j=0; j< 8; j++)
    for (int i=0; i< 8; i++) {
      if(letter[j] & ( 1 << (7-i)))
        set(i, j) = foreground;
      else
        set(i, j) = (background() + CHSV((i+j)*10, 255, 50));
    }
}
