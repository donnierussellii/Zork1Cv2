// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"



//*****************************************************************************

// Video, audio and IR code derived from Zorkduino by Peter Barrett

/* Copyright (c) 2010-2014, Peter Barrett
 **
 ** Permission to use, copy, modify, and/or distribute this software for
 ** any purpose with or without fee is hereby granted, provided that the
 ** above copyright notice and this permission notice appear in all copies.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 ** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 ** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 ** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 ** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 ** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 ** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 ** SOFTWARE.
 */



#define CONSOLE_W  40
#define CONSOLE_H  24



// ~1016 cpu clocks per line at 16Mhz
#define USEC(_x)          (((_x)*F_CPU/1000000)-1)
#define TICKS_SCANLINE    USEC(63.555)
#define TICKS_HSYNC       USEC(4.7)
#define TICKS_LONG_HSYNC  USEC(63.555-4.7)
#define TICKS_HBLANK      USEC(10.30)

#define BLANK_LINES   40
#define ACTIVE_LINES  (CONSOLE_H*8)

#define STATE_VBLANK  0
#define STATE_PRE     1
#define STATE_ACTIVE  2
#define STATE_POST    3



const uint8_t VideoStateFrames[] PROGMEM =
{
  3,
  BLANK_LINES,
  ACTIVE_LINES,
  262-(3+BLANK_LINES+ACTIVE_LINES)
};

uint32_t VideoFrame = 0;
uint8_t VideoState = 0;
uint8_t VideoCount = 0;

uint8_t AudioCount = 0;
uint8_t AudioFreq = 0;
uint8_t AudioCycles = 0;

const uint8_t Font8x8[8*256] PROGMEM =
{
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,24,0,28,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,6,0,96,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,24,0,0,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,231,255,227,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,249,255,159,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,231,255,255,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,24,102,102,62,102,54,24,
  14,112,102,24,0,0,0,6,
  60,24,60,126,12,126,60,126,
  60,60,0,0,12,0,48,60,
  60,24,124,60,120,126,126,62,
  102,126,6,102,96,99,102,60,
  124,60,124,60,126,102,102,99,
  102,102,126,30,64,120,8,0,
  48,0,96,0,6,0,14,0,
  96,24,6,96,56,0,0,0,
  0,0,0,0,24,0,0,0,
  0,0,0,24,24,126,0,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,231,153,153,193,153,201,231,
  241,143,153,231,255,255,255,249,
  195,231,195,129,243,129,195,129,
  195,195,255,255,243,255,207,195,
  195,231,131,195,135,129,129,193,
  153,129,249,153,159,156,153,195,
  131,195,131,195,129,153,153,156,
  153,153,129,225,191,135,247,255,
  207,255,159,255,249,255,241,255,
  159,231,249,159,199,255,255,255,
  255,255,255,255,231,255,255,255,
  255,255,255,231,231,129,255,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,24,102,255,96,108,28,24,
  28,56,60,24,0,0,0,12,
  102,56,102,12,28,96,96,6,
  102,102,24,24,24,126,24,102,
  102,60,102,102,108,96,96,96,
  102,24,6,108,96,119,118,102,
  102,102,102,96,24,102,102,99,
  102,102,12,24,96,24,28,0,
  24,60,96,60,6,60,24,62,
  96,0,0,96,24,102,124,60,
  124,62,124,62,126,102,102,99,
  102,102,126,60,24,120,0,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,231,153,0,159,147,227,231,
  227,199,195,231,255,255,255,243,
  153,199,153,243,227,159,159,249,
  153,153,231,231,231,129,231,153,
  153,195,153,153,147,159,159,159,
  153,231,249,147,159,136,137,153,
  153,153,153,159,231,153,153,156,
  153,153,243,231,159,231,227,255,
  231,195,159,195,249,195,231,193,
  159,255,255,159,231,153,131,195,
  131,193,131,193,129,153,153,156,
  153,153,129,195,231,135,255,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,24,102,102,60,24,56,24,
  24,24,255,126,0,126,0,24,
  110,24,12,24,60,124,124,12,
  60,62,24,24,48,0,12,12,
  110,102,124,96,102,124,124,96,
  126,24,6,120,96,127,126,102,
  102,102,102,60,24,102,102,107,
  60,60,24,24,48,24,54,0,
  0,6,124,96,62,102,62,102,
  124,56,6,108,24,127,102,102,
  102,102,102,96,24,102,102,107,
  60,102,12,126,24,124,100,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,231,153,153,195,231,199,231,
  231,231,0,129,255,129,255,231,
  145,231,243,231,195,131,131,243,
  195,193,231,231,207,255,243,243,
  145,153,131,159,153,131,131,159,
  129,231,249,135,159,128,129,153,
  153,153,153,195,231,153,153,148,
  195,195,231,231,207,231,201,255,
  255,249,131,159,193,153,193,153,
  131,199,249,147,231,128,153,153,
  153,153,153,159,231,153,153,148,
  195,153,243,129,231,131,155,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,24,0,102,6,48,111,0,
  24,24,60,24,0,0,0,48,
  118,24,24,12,108,6,102,24,
  102,6,0,0,24,0,24,24,
  110,102,102,96,102,96,96,110,
  102,24,6,120,96,107,126,102,
  124,102,124,6,24,102,102,127,
  60,24,48,24,24,24,99,0,
  0,62,102,96,102,126,24,102,
  102,24,6,120,24,127,102,102,
  102,102,96,60,24,102,102,127,
  24,102,24,126,24,110,152,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,231,255,153,249,207,144,255,
  231,231,195,231,255,255,255,207,
  137,231,231,243,147,249,153,231,
  153,249,255,255,231,255,231,231,
  145,153,153,159,153,159,159,145,
  153,231,249,135,159,148,129,153,
  131,153,131,249,231,153,153,128,
  195,231,207,231,231,231,156,255,
  255,193,153,159,153,129,231,153,
  153,231,249,135,231,128,153,153,
  153,153,159,195,231,153,153,128,
  231,153,231,129,231,145,103,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,255,124,102,102,0,
  28,56,102,24,24,0,24,96,
  102,24,48,102,126,102,102,48,
  102,12,24,24,12,126,48,0,
  96,126,102,102,108,96,96,102,
  102,24,102,108,96,99,110,102,
  96,108,108,6,24,102,60,119,
  102,24,96,24,12,24,0,0,
  0,102,102,96,102,96,24,62,
  102,24,6,108,24,107,102,102,
  124,62,96,6,24,102,60,62,
  60,62,48,24,24,102,0,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,0,131,153,153,255,
  227,199,153,231,231,255,231,159,
  153,231,207,153,129,153,153,207,
  153,243,231,231,243,129,207,255,
  159,129,153,153,147,159,159,153,
  153,231,153,147,159,156,145,153,
  159,147,147,249,231,153,195,136,
  153,231,159,231,243,231,255,255,
  255,153,153,159,153,159,231,193,
  153,231,249,147,231,148,153,153,
  131,193,159,249,231,153,195,193,
  195,193,207,231,231,153,255,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,24,0,102,24,70,59,0,
  14,112,0,0,24,0,24,64,
  60,126,126,60,12,60,60,48,
  60,56,24,24,6,0,96,24,
  62,102,124,60,120,126,96,62,
  102,126,60,102,126,99,102,60,
  96,54,102,60,24,126,24,99,
  102,24,126,30,6,120,0,255,
  0,62,124,60,62,60,24,6,
  102,60,6,102,60,99,102,60,
  96,6,96,124,14,62,24,54,
  102,12,126,60,24,6,0,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,231,255,153,231,185,196,255,
  241,143,255,255,231,255,231,191,
  195,129,129,195,243,195,195,207,
  195,199,231,231,249,255,159,231,
  193,153,131,195,135,129,159,193,
  153,129,195,153,129,156,153,195,
  159,201,153,195,231,129,231,156,
  153,231,129,225,249,135,255,0,
  255,193,131,195,193,195,231,249,
  153,195,249,153,195,156,153,195,
  159,249,159,131,241,193,231,201,
  153,243,129,195,231,249,255,255,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,48,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,48,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,124,
  0,0,60,0,0,0,0,0,
  96,6,0,0,0,0,0,0,
  0,120,0,0,24,0,0,0,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,207,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,207,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,131,
  255,255,195,255,255,255,255,255,
  159,249,255,255,255,255,255,255,
  255,135,255,255,231,255,255,255
};



extern uint8_t ConsoleData[];



void IR_Interrupt(void);



static void inline TickAlign(uint8_t tick)
{
  __asm__ __volatile__
  (
      "sub  %[tick], %[tcnt1l]  \n"
"loop: subi %[tick], 3          \n"
      "brcc loop                \n"
      "cpi      %[tick],-3      \n"
      "breq     done            \n"
      "cpi      %[tick],-2      \n"
      "brne     .               \n"
"done:                          \n"
      :: [tick] "a" (tick),
      [tcnt1l] "a" (TCNT1L)
  );
}



ISR(TIMER1_OVF_vect)
{
  OCR1A = VideoState ? TICKS_HSYNC : TICKS_LONG_HSYNC;
  
  if (VideoState == STATE_ACTIVE)
  {
    uint8_t a, i, c;

    //use USART1 to output to PD3

    UDR1 = 0; //USART Data Register. Contains the Receive and Transmit Registers and operates as FIFO. 
    UCSR1C = (1<<USBS1) | (3<<UCSZ10); //USART Control and Status Register C; Stop Bit Selection Bit (2 stop bits); Frame rate (8)
    TickAlign(TICKS_HBLANK);
    UCSR1B = 1<<TXEN1; //USART Control and Status Register B; Transmit Enable Bit

    uint8_t line = ACTIVE_LINES-VideoCount;
    uint8_t *src = ConsoleData + (line >> 3)*CONSOLE_W;
    uint16_t font = (uint16_t)Font8x8 + ((line&7) << 8);

    UCSR1C = (1<<UMSEL11) | (1<<UMSEL10); //USART Control and Status Register C; START SPI MODE
    
    i = 0;
    a = CONSOLE_W;
    while (a--)
    {
      c = src[i++]; if (!c) break;
      UDR1 = pgm_read_byte(font+c);
    }
    UDR1 = 0;

    UCSR1B = 0;
  }

  // Advance video state machine
  if (!--VideoCount)
  {
    VideoState = (VideoState+1) & 3;
    VideoCount = pgm_read_byte(VideoStateFrames + VideoState);

    if (VideoState == STATE_VBLANK) VideoFrame++;
  }



  IR_Interrupt();



  if (AudioCycles && !--AudioCount)
  {
    AudioCycles--;
    AudioCount = AudioFreq;
    PORTD ^= _BV(0);
  }
}



// also inits audio
void VideoInit(void)
{
  //USART Baud Rate Registers (high and low bits)
  UBRR1H = 0; // Fastest serial clock  
  UBRR1L = 0;

  DDRB |= _BV(5); //Data Direction for Port B     PB5     digital pin 11   output
  DDRD |= _BV(3); //Data Direction for Port D     PD3     digital pin 18   output
  DDRD |= _BV(0); //Data Direction for Port D     PD0     digital pin 21   output

  PORTD &= ~_BV(3); //Port D pins

  // Set OC1A on Compare Match, Fast PWM, No prescaling, Timer 1
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11); //Timer/Counter1 Control Register A
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); //Timer/Counter1 Control Register B
  ICR1 = TICKS_SCANLINE; //Input Capture Register of Timer/Counter1
  OCR1A = TICKS_HSYNC; //Output Capture Register of Timer/Counter1
  TIMSK1 = _BV(TOIE1); //Timer/Counter Interrupt Mask Register; Timer/Counter1 Overflow Interrupt Enable
  PORTD = 0;

  VideoFrame = 0;
  VideoState = 3;
  VideoCount = 1;

  sei();
}



void PlayTone(uint8_t freq, uint8_t cycles)
{
  AudioFreq = freq;
  AudioCount = freq;
  AudioCycles = cycles;
}



// WebTV UART-like keyboard protocol

// 3.25 bits: start (0)
// 10   bits: code for keyup, keydown, all keys released etc
// 8    bits: keycode
// 1    bits: parity

// use portb to monitor ir sensor
// on arduino mega PORTB can be accessed from PB0, which is digital pin 53



#define IR_PIN         0    //digital pin 53 (mega)
#define IR_BITS        12   //uart bits
#define IR_KEYDOWN     0x4A
#define IR_KEYUP       0x5E
#define IR_SHIFTLEFT   0x8C
#define IR_SHIFTRIGHT  0x4C



uint8_t IR_KeyDown = 0;
uint8_t IR_KeyUp = 0;



void IR_Init(void)
{
  DDRB &= ~_BV(IR_PIN); //set port B data direction register to read //pinMode(53, INPUT);
}



void ProcessIREvent(uint16_t e)
{
  uint8_t t = e & 0xff;
  uint8_t v = e >> 8;

  static uint8_t state = 0;
  static uint16_t code = 0;
  static uint8_t keydownlast = 0;

  if (state == 0)
  {
    //3.25 long 0, rising edge of start bit    
    if (t >= 36 && t <= 40 && v == 0)
      state = 1;
  }
  else if (state == 1)
  {
    //1.5ms ish
    state = (t >= 9 && t <= 13 && v == 1) ? 2 : 0;
  }
  else 
  {
    uint8_t bits = state-2;

    t += IR_BITS>>1;
    while (t > IR_BITS && bits < 16)
    {
      t -= IR_BITS;
      code = (code << 1) | v;
      bits++;
    }

    if (bits == 16)
    {
      uint8_t md = code >> 8;

      v = (t <= IR_BITS);
      code |= v; //low bit is parity

      if (md == IR_KEYDOWN)
      {
        if (keydownlast != (uint8_t)code)
          IR_KeyDown = keydownlast = code;
      }
      else if (md == IR_KEYUP)
      {
        IR_KeyUp = code;
        keydownlast = 0;
      }
      state = 0;
      return;
    }
    state = bits+2;
  }
}



// buffer implemented to save cycles in ISR

#define IR_BUFFER_SIZE  45

uint16_t IR_Buffer[IR_BUFFER_SIZE];
uint8_t IR_Count = 0;



void IR_Interrupt(void)
{
  uint8_t ir = PINB & 1;

  static uint8_t last = 0;
  static uint8_t count = 0;

  if (ir != last)
  {
    if (IR_Count < IR_BUFFER_SIZE)
      IR_Buffer[IR_Count++] = count | (last << 8);
    count = 0;
    last = ir;
  }
  if (count != 0xFF) count++;
}



const uint8_t IR_Keys[] PROGMEM =
{
  0x00,0x00,0x62,0x00,0x00,0x1D,0x00,0x00,0x00,0x1E,0x00,0x2F,0x00,0x1F,0x20,0x6E,
  0x60,0x00,0x35,0x00,0x00,0x00,0x00,0x3D,0x00,0x00,0x00,0x2D,0x00,0x00,0x00,0x36,
  0x00,0x00,0x76,0x2E,0x63,0x00,0x00,0x2C,0x78,0x00,0x00,0x00,0x7A,0x00,0x0A,0x6D,
  0x00,0x00,0x66,0x6C,0x64,0x00,0x00,0x6B,0x73,0x00,0x00,0x3B,0x61,0x00,0x5C,0x6A,
  0x00,0x00,0x74,0x00,0x00,0x00,0x00,0x5D,0x00,0x00,0x00,0x5B,0x09,0x00,0x08,0x79,
  0x00,0x00,0x34,0x39,0x33,0x00,0x00,0x38,0x32,0x00,0x00,0x30,0x31,0x00,0x00,0x37,
  0x00,0x00,0x67,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x27,0x00,0x00,0x00,0x68,
  0x00,0x00,0x72,0x6F,0x65,0x00,0x00,0x69,0x77,0x00,0x00,0x70,0x71,0x00,0x00,0x75,

  //shifted
  0x00,0x00,0x42,0x00,0x00,0x1D,0x00,0x00,0x00,0x1E,0x00,0x3F,0x00,0x1F,0x20,0x4E,
  0x7E,0x00,0x25,0x00,0x00,0x00,0x00,0x2B,0x00,0x00,0x00,0x5F,0x00,0x00,0x00,0x5E,
  0x00,0x00,0x56,0x3E,0x43,0x00,0x00,0x3C,0x58,0x00,0x00,0x00,0x5A,0x00,0x0A,0x4D,
  0x00,0x00,0x46,0x4C,0x44,0x00,0x00,0x4B,0x53,0x00,0x00,0x3A,0x41,0x00,0x7C,0x4A,
  0x00,0x00,0x54,0x00,0x00,0x00,0x00,0x7D,0x00,0x00,0x00,0x7B,0x09,0x00,0x08,0x59,
  0x00,0x00,0x24,0x28,0x23,0x00,0x00,0x2A,0x40,0x00,0x00,0x29,0x21,0x00,0x00,0x26,
  0x00,0x00,0x47,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x22,0x00,0x00,0x00,0x48,
  0x00,0x00,0x52,0x4F,0x45,0x00,0x00,0x49,0x57,0x00,0x00,0x50,0x51,0x00,0x00,0x55,
};



uint8_t IR_ReadKey(void)
{
  uint8_t k, c, v, i;

  static uint8_t mod = 0;


  // see loop() in arduino source
  if (serialEventRun) serialEventRun();


  if (IR_Count == 0) return 0;
  for (i=0; i<IR_Count; i++)
    ProcessIREvent(IR_Buffer[i]);
  IR_Count = 0;


  k = IR_KeyUp;
  if (k)
  {
    IR_KeyUp = 0;
    switch (k)
    {
      case IR_SHIFTLEFT:
      case IR_SHIFTRIGHT:
        mod &= ~0x80;
      break;
    }
  }
  
  k = IR_KeyDown;
  if (k)
  {
    IR_KeyDown = 0;
    switch (k)
    {
      case IR_SHIFTLEFT:
      case IR_SHIFTRIGHT:
        mod |= 0x80;
        return 0;
    }

    //check parity
    v = k;
    for (c=0; v; c++)
      v &= v-1;
    if (c & 1)
    {
      k = pgm_read_byte_near(IR_Keys + mod + (k >> 1));
      return k;
    }
  }

  return 0;
}

//*****************************************************************************



//############################################################################

void ResetConsole(void)
{
  void (*reset)(void) = 0;

  reset();
  // not reached
}



void setup(void)
{
  IR_Init();
  VideoInit();
  SetRandomSeed(0);
  GameLoop();
  ResetConsole();
  // not reached
}



void loop(void)
{
}

//############################################################################
