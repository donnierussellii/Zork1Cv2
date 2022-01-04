// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#ifdef COMPILE_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <conio.h> // getch
  int ConsoleW, ConsoleH, CursorColumn, CursorRow;
  HANDLE hStdOutput;
#elif COMPILE_DOS
  #include <conio.h> // getch
  #include <dos.h> // REGS int386 (open watcom)
  int ConsoleW, ConsoleH, CursorColumn, CursorRow;
#elif __AVR_ATmega2560__
  #include <Arduino.h>
  #include <EEPROM.h>
  int ConsoleW = 40, ConsoleH = 24, CursorColumn = 0, CursorRow = 0;
  uint8_t ConsoleData[40*24];
#else
  #define NO_CONIO
  int ConsoleW = 80, ConsoleH = 25, CursorColumn = 0, CursorRow = 0;
#endif  



#include "def.h"
#include "_enum.h"



int TextColor       = 7 | (1<<4); // gray on blue
int StatusLineColor = 1 | (7<<4); // blue on gray

int NumPrintedLines = 0;

uint8_t AutoPlayMode = 0;
const char *AutoPlayP;



extern struct GAMEFLAGS GameFlags;
extern struct GAMEVARS GameVars;
extern struct ROOM_STRUCT Room[NUM_ROOMS];
extern struct OBJ_STRUCT Obj[NUM_OBJECTS];



void DeactivateAutoPlay(void);



//#############################################################################

#ifdef COMPILE_DOS

int SetupDOSConsoleRoutine(int c)
{
  int i, row = 0, prev_row, col = 0, prev_col;

  {
    union REGS regs;
    regs.h.ah = 0x02; // set cursor position
    regs.h.bh = 0; // page number
    regs.h.dh = row; // row
    regs.h.dl = col; // column
    int386(0x10, &regs, &regs);
  }

  for (i=0; i<1024; i++)
  {
    prev_row = row;
    prev_col = col;

    fputc(c, stdout);
    fflush(stdout);

    {
      union REGS regs;
      regs.h.ah = 0x03; // get cursor position and shape
      regs.h.bh = 0; // page number
      int386(0x10, &regs, &regs);
      // ignore ax,ch,cl
      row = regs.h.dh; // row
      col = regs.h.dl; // column
    }

    if (c == ' '  && prev_col >  col) return prev_col+1;
    if (c == '\n' && prev_row == row) return prev_row+1;
  }

  return 0; // something's weird
}

#endif



void SetupConsole(void)
{
#ifdef COMPILE_WINDOWS

  hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hStdOutput == INVALID_HANDLE_VALUE) exit(1);

  {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdOutput, &csbi);
    ConsoleW = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
    ConsoleH = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
    CursorColumn = csbi.dwCursorPosition.X;
    CursorRow    = csbi.dwCursorPosition.Y;
  }

  {
    WORD atr = TextColor;
    SetConsoleTextAttribute(hStdOutput, atr);
  }

#ifdef USE_CHAR_CURSOR
  {
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 25;
    cci.bVisible = 0;
    SetConsoleCursorInfo(hStdOutput, &cci);
  }
#endif

#elif COMPILE_DOS

  ConsoleW = SetupDOSConsoleRoutine( ' '); if (ConsoleW == 0) ConsoleW = 80;
  ConsoleH = SetupDOSConsoleRoutine('\n'); if (ConsoleH == 0) ConsoleH = 25;

  {
    union REGS regs;
    regs.h.ah = 0x03; // get cursor position and shape
    regs.h.bh = 0; // page number
    int386(0x10, &regs, &regs);
    // ignore ax,ch,cl
    CursorRow = regs.h.dh; // row
    CursorColumn = regs.h.dl; // column
  }

#ifdef USE_CHAR_CURSOR
  {
    union REGS regs;
    regs.h.ah = 0x01; // set text-mode cursor shape
    regs.h.ch = 0x07; // scan row start (> end hides cursor)
    regs.h.cl = 0x00; // scan row end
    int386(0x10, &regs, &regs);
  }
#endif

#endif  
}



#ifndef __AVR_ATmega2560__

int main(void)
{
  SetupConsole();
  SetRandomSeed(0);
  GameLoop();
  return 0;
}

#endif

//#############################################################################



//*****************************************************************************

// low level output functions



#ifdef __AVR_ATmega2560__



void ShiftScreenUp(void)
{
#ifdef NO_STATUS_LINE
  uint16_t i = 0;
#else
  uint16_t i = ConsoleW; // skip top line of console
#endif

  for ( ; i<ConsoleW*(ConsoleH-1); i++)
    ConsoleData[i] = ConsoleData[i+ConsoleW];
  for (; i<ConsoleW*ConsoleH; i++)
    ConsoleData[i] = 0;
}



void PutNewLine(void)
{
  CursorColumn = 0;
  if (CursorRow == ConsoleH-1)
    ShiftScreenUp();
  else
    CursorRow++;
}



void PutChar(uint8_t c)
{
  switch (c)
  {
    case '\n':
      PutNewLine();
    break;

    case '\b':
      if (CursorColumn > 0)
        CursorColumn--;
    break;

    default:
      ConsoleData[ConsoleW*CursorRow+CursorColumn] = c;
      if (CursorColumn == ConsoleW-1)
        PutNewLine();
      else
        CursorColumn++;
    break;
  }
}



#elif COMPILE_DOS



void ShiftScreenUp(void)
{
  union REGS regs;
  regs.h.ah = 0x06; // scroll up window
  regs.h.al = 1; // lines to scroll
  regs.h.bh = TextColor; // background and foreground color
  regs.h.cl = 0; // left column
  regs.h.dl = ConsoleW-1; // right column
#ifdef NO_STATUS_LINE
  regs.h.ch = 0; // top row
#else
  regs.h.ch = 1; // top row; skip top line of console
#endif
  regs.h.dh = ConsoleH-1; // bottom row
  int386(0x10, &regs, &regs);
}



void PutNewLine(void)
{
  CursorColumn = 0;
  if (CursorRow == ConsoleH-1)
    ShiftScreenUp();
  else
    CursorRow++;
}



void PutChar(uint8_t c)
{
  switch (c)
  {
    case '\n':
      PutNewLine();
    break;

    case '\b':
      if (CursorColumn > 0)
        CursorColumn--;
    break;

    default:
      {
        union REGS regs;
        regs.h.ah = 0x02; // set cursor position
        regs.h.bh = 0; // page number
        regs.h.dh = CursorRow; // row
        regs.h.dl = CursorColumn; // column
        int386(0x10, &regs, &regs);
      }

      {
        union REGS regs;
        regs.h.ah = 0x09; // write character and attribute at cursor position
        regs.h.al = c; // character
        regs.h.bh = 0; // page number
        regs.h.bl = TextColor; // background and foreground color
        regs.w.cx = 1; // number of times to print character
        int386(0x10, &regs, &regs);
      }

      if (CursorColumn == ConsoleW-1)
        PutNewLine();
      else
        CursorColumn++;
    break;
  }

  {
    union REGS regs;
    regs.h.ah = 0x02; // set cursor position
    regs.h.bh = 0; // page number
    regs.h.dh = CursorRow; // row
    regs.h.dl = CursorColumn; // column
    int386(0x10, &regs, &regs);
  }
}



#elif COMPILE_WINDOWS



void ShiftScreenUp(void)
{
  SMALL_RECT sr;
  COORD d;
  CHAR_INFO ci;

  sr.Left = 0;
  sr.Right = ConsoleW-1;
#ifdef NO_STATUS_LINE
  sr.Top = 1;
#else
  sr.Top = 2;
#endif
  sr.Bottom = ConsoleH-1;

  d.X = 0,
  d.Y = sr.Top-1;

  ci.Char.AsciiChar = ' ';
  ci.Attributes = TextColor;

  ScrollConsoleScreenBuffer(hStdOutput, &sr, 0, d, &ci);
}



void PutNewLine(void)
{
  CursorColumn = 0;
  if (CursorRow == ConsoleH-1)
    ShiftScreenUp();
  else
    CursorRow++;
}



void PutChar(uint8_t c)
{
  switch (c)
  {
    case '\n':
      PutNewLine();
    break;

    case '\b':
      if (CursorColumn > 0)
        CursorColumn--;
    break;

    default:
      {
        char chr[2] = {(char)c, 0};
        WORD atr[2] = {(WORD)TextColor, 0};
        COORD cp;
        DWORD num;
        cp.X = CursorColumn;
        cp.Y = CursorRow;
        WriteConsoleOutputCharacter(hStdOutput, chr, 1, cp, &num);
        WriteConsoleOutputAttribute(hStdOutput, atr, 1, cp, &num);
      }

      if (CursorColumn == ConsoleW-1)
        PutNewLine();
      else
        CursorColumn++;
    break;
  }

  {
    COORD cp;
    cp.X = CursorColumn;
    cp.Y = CursorRow;
    SetConsoleCursorPosition(hStdOutput, cp);
  }
}



#else



void PutChar(uint8_t c)
{
  fputc(c, stdout);
  fflush(stdout);

  switch (c)
  {
    case '\n':
      CursorColumn = 0;
    break;

    case '\b':
      if (CursorColumn > 0)
        CursorColumn--;
    break;

    default:
      if (CursorColumn == ConsoleW-1)
        CursorColumn = 0;
      else
        CursorColumn++;
    break;
  }
}



#endif

//*****************************************************************************



//*****************************************************************************

// low level input functions



void MyDelay(int ms)
{
  int key_pressed = 0;

#if defined(COMPILE_DOS) || defined(COMPILE_WINDOWS)
  clock_t start;

  static int factor = 9;

  ms = ms * (1+factor) / 10;
  start = clock();
  while (clock() - start < ms)
  {
    if (kbhit())
    {
      int c = getch();

      if (c == 0) getch();
      else if (c >= '0' && c <= '9') factor = c - '0';
      else key_pressed = 1;

      break;
    }
#ifdef COMPILE_DOS
    delay(1);
  }
#elif COMPILE_WINDOWS
    Sleep(1);
  }
#endif
#endif

  if (key_pressed && AutoPlayMode)
  {
    // fast forward to end of autoplay string
    for (;;)
    {
      if (pgm_read_byte(AutoPlayP) == 0) break;
      else AutoPlayP++;
    }
  }
}



#ifdef NO_CONIO
 


char *GetString(void)
{
  static char buffer[80];

  if (AutoPlayMode)
    MyDelay(1000*NumPrintedLines);
  if (AutoPlayMode && pgm_read_byte(AutoPlayP) == 0)
    DeactivateAutoPlay();

  NumPrintedLines = 0;

  memset(buffer, 0, 80);

  if (AutoPlayMode)
  {
    char *p = buffer;
    int c;

    for (;;)
    {
      MyDelay(200);
      c = pgm_read_byte(AutoPlayP++);

      if (p < buffer+80-1)
      {
        *p++ = c;
        if (c) PutChar(c);
      }

      if (c == 0)
      {
        DeactivateAutoPlay();
        break;
      }
      else if (c == '\n')
        break;
    }
  }
  else
    fgets(buffer, 80, stdin);

  return buffer;
}



// actually wait for enter
void WaitForKey(void)
{
  if (AutoPlayMode == 0)
    while (fgetc(stdin) != '\n') {}
}



#else



#ifdef __AVR_ATmega2560__

#define SAVER_FRAMES   (5*60*30)  // 5 minutes  ~30 frames per second (NTSC)
#define INVERT_FRAMES  (1*60*30)  // 1 minute

extern uint32_t VideoFrame;

uint8_t IR_ReadKey(void);
void PlayTone(uint8_t freq, uint8_t cycles);

void InvertScreen(void)
{
  uint16_t i;

  for (i=0; i<ConsoleW*ConsoleH; i++)
    ConsoleData[i] ^= 128;
}

#else

void PlayTone(uint8_t /* freq */ , uint8_t /* cycles */ )
{
}

#endif



int GetKey(uint8_t more)
{
  int c;
#ifdef __AVR_ATmega2560__
  uint8_t inverted = 0, saver = 0;
  uint32_t start, elapsed;
#endif


  if (AutoPlayMode)
  {
    if (more) return '\n';

    MyDelay(200);
    c = pgm_read_byte(AutoPlayP++);

    if (c == 0)
      DeactivateAutoPlay();

    return c;
  }


#ifdef __AVR_ATmega2560__
  start = VideoFrame;
  for (;;)
  {
    c = IR_ReadKey();
    if (c) break;
    elapsed = VideoFrame;
    if (elapsed-start >= (saver ? INVERT_FRAMES : SAVER_FRAMES))
    {
      InvertScreen();
      saver = 1;
      inverted ^= 1;
      start = VideoFrame;
    }
  }
  if (inverted)
    InvertScreen();
  // translate arrow key codes to conio
       if (c == 28) c = -72;
  else if (c == 29) c = -80;
  else if (c == 30) c = -75;
  else if (c == 31) c = -77;
#else
  for (;;)
  {
    // non-standard c
    c = getch();
    if (c == 0)
      c = -getch();
    if (c) break;
  }
#endif

  return c;
}



#define NUM_STRING_BUFFERS  6

char StringBuffer[80*NUM_STRING_BUFFERS];



void CopyStringBuffer(int8_t dir, uint8_t *curbuf, uint8_t *pos)
{
  if (*curbuf+dir > 0 && *curbuf+dir < NUM_STRING_BUFFERS && StringBuffer[80*(*curbuf+dir)])
  {
    uint8_t i;

    *curbuf += dir;

#ifdef USE_CHAR_CURSOR
    PutChar(' ');
    PutChar('\b');
#endif

    for (i=0; i<*pos; i++)
    {
      PutChar(' ');
      PutChar('\b');
      PutChar('\b');
    }
  
    *pos = 0;
    for (i=0; i<80; i++)
    {
      uint8_t c = StringBuffer[80*(*curbuf) + i];

      if (CursorColumn >= ConsoleW-2)
        c = 0;

      StringBuffer[i] = c;
      if (c)
      {
        PutChar(c);
        (*pos)++;
      }
    }

#ifdef USE_CHAR_CURSOR
    PutChar('_');
    PutChar('\b');
#endif

  }
}



char *GetString(void)
{
  uint8_t curbuf = 0, pos = 0, i, j;

  if (AutoPlayMode)
    MyDelay(1000*NumPrintedLines);

  NumPrintedLines = 0;

  for (j=0; j<80; j++)
    StringBuffer[j] = 0;

#ifdef USE_CHAR_CURSOR
  PutChar('_');
  PutChar('\b');
#endif

  for (;;)
  {
    int c = GetKey(0);

         if (c == -72) CopyStringBuffer( 1, &curbuf, &pos); // up
    else if (c == -80) CopyStringBuffer(-1, &curbuf, &pos); // down
    else if (c == -75) {} // left
    else if (c == -77) {} // right
    else if (c == '\r' || c == '\n')
    {
#ifdef USE_CHAR_CURSOR
      PutChar(' ');
#endif
      PutChar('\n');
      PlayTone(15, 8);
      break;
    }
    else if (c == '\b')
    {
      if (pos > 0)
      {
#ifdef USE_CHAR_CURSOR
        PutChar(' ');
        PutChar('\b');
        PutChar('\b');
        PutChar('_');
        PutChar('\b');
#else
        PutChar('\b');
        PutChar(' ');
        PutChar('\b');
#endif
        StringBuffer[--pos] = 0;
        PlayTone(15, 8);
      }
      else
        PlayTone(12, 8);
    }    
    else if (c >= ' ' && c <= '~')
    {
      if (pos < 80-1 && CursorColumn < ConsoleW-2)
      {
        PutChar(c);
#ifdef USE_CHAR_CURSOR
        PutChar('_');
        PutChar('\b');
#endif
        StringBuffer[pos++] = c;
        StringBuffer[pos] = 0;
        PlayTone(15, 8);
      }
      else
        PlayTone(12, 8);
    }
  }

  if (*StringBuffer == 0)
    return StringBuffer;

  for (i=1; i<NUM_STRING_BUFFERS; i++)
  {
    for (j=0; j<80; j++)
      if (StringBuffer[80*i+j] != StringBuffer[j]) break;
    if (j == 80) return StringBuffer;
  }

  for (i=NUM_STRING_BUFFERS-1; i>0; i--)
    for (j=0; j<80; j++)
      StringBuffer[80*i+j] = StringBuffer[80*(i-1)+j];

  return StringBuffer;
}



void WaitForKey(void)
{
  GetKey(1);
  PlayTone(15, 8);
}



#endif

//*****************************************************************************



//*****************************************************************************

// compressed text streaming functions



const uint8_t *StreamP;
uint8_t StreamB, StreamC1, StreamC2, StreamC3;

const uint8_t *SavedStreamP;
uint8_t SavedStreamB, SavedStreamC1, SavedStreamC2, SavedStreamC3;



extern const uint8_t HuffmanTree[];
extern const uint8_t TreeStartIndex;



void OpenStream(const uint8_t *p)
{
  StreamP  = p;
  StreamB  = 0;
  StreamC1 = 0;
  StreamC2 = 0;
  StreamC3 = 0;
}



void SaveStream(void)
{
  SavedStreamP  = StreamP;
  SavedStreamB  = StreamB;
  SavedStreamC1 = StreamC1;
  SavedStreamC2 = StreamC2;
  SavedStreamC3 = StreamC3;
}



void RestoreStream(void)
{
  StreamP  = SavedStreamP;
  StreamB  = SavedStreamB;
  StreamC1 = SavedStreamC1;
  StreamC2 = SavedStreamC2;
  StreamC3 = SavedStreamC3;
}



uint8_t GetStream(void)
{
  uint8_t node, byte, c;

  if (StreamC1) {c = StreamC1; StreamC1 = 0; return c;}
  if (StreamC2) {c = StreamC2; StreamC2 = 0; return c;}
  if (StreamC3) {c = StreamC3; StreamC3 = 0; return c;}

  node = TreeStartIndex;
  byte = pgm_read_byte(StreamP);
  for (;;)
  {
    if (byte & (1<<StreamB)) node = pgm_read_byte(HuffmanTree + 3*node+2);
    else                     node = pgm_read_byte(HuffmanTree + 3*node+1);

    if (StreamB == 7) {byte = pgm_read_byte(++StreamP); StreamB = 0;} else StreamB++;

    c = pgm_read_byte(HuffmanTree + 3*node);
    switch (c)
    {
      case 0: break;
      case 1: return 0;

      case 2: c = 't'; StreamC1 = 'h'; StreamC2 = 'e'; StreamC3 = ' '; return c;
      case 3: c = 'T'; StreamC1 = 'h'; StreamC2 = 'e'; StreamC3 = ' '; return c;
      case 4: c = 'i'; StreamC1 = 'n'; StreamC2 = 'g'; StreamC3 = ' '; return c;
      case 5: c = 'y'; StreamC1 = 'o'; StreamC2 = 'u'; StreamC3 = ' '; return c;
      case 6: c = 'Y'; StreamC1 = 'o'; StreamC2 = 'u'; StreamC3 = ' '; return c;
      case 7: c = 'a'; StreamC1 = 'n'; StreamC2 = 'd'; StreamC3 = ' '; return c;
      case 8: c = ' '; StreamC1 = 'i'; StreamC2 = 's'; StreamC3 = ' '; return c;
      case 9: c = ' '; StreamC1 = 't'; StreamC2 = 'o'; StreamC3 = ' '; return c;

      case 11: c = ' '; StreamC1 = 'o'; StreamC2 = 'f'; StreamC3 = ' '; return c;
      case 12: c = 'h'; StreamC1 = 'e'; StreamC2 = 'r'; StreamC3 = 'e'; return c;
      case 13: c = 'y'; StreamC1 = 'o'; StreamC2 = 'u'; StreamC3 = 'r'; return c;
      case 14: c = 'w'; StreamC1 = 'i'; StreamC2 = 't'; StreamC3 = 'h'; return c;

      default: return c;
    }
  }
}



void AdvanceStream(int16_t i)
{
  uint8_t node = TreeStartIndex, byte = pgm_read_byte(StreamP), c;

  while (i)
  {
    if (byte & (1<<StreamB)) node = pgm_read_byte(HuffmanTree + 3*node+2);
    else                     node = pgm_read_byte(HuffmanTree + 3*node+1);

    if (StreamB == 7) {byte = pgm_read_byte(++StreamP); StreamB = 0;} else StreamB++;

    c = pgm_read_byte(HuffmanTree + 3*node);
    if (c)
    {
      if (c == 1) i--;
      node = TreeStartIndex;
    }
  }

  StreamC1 = 0;
  StreamC2 = 0;
  StreamC3 = 0;
}

//*****************************************************************************



//*****************************************************************************

#ifdef NO_STATUS_LINE
  #define MAX_PRINTED_LINES  (ConsoleH-1)
#else
  #define MAX_PRINTED_LINES  (ConsoleH-2)
#endif



void HandleMore(void)
{
  uint8_t i;

  if (NumPrintedLines < MAX_PRINTED_LINES) return;

  i = ConsoleW-1 - 4; while (i--) PutChar(' ');
  PutChar('M'); PutChar('O'); PutChar('R'); PutChar('E');

  if (AutoPlayMode)
    MyDelay(1000*NumPrintedLines);

  NumPrintedLines = 0;

  WaitForKey();

  PutChar('\b'); PutChar('\b'); PutChar('\b'); PutChar('\b');
  PutChar(' '); PutChar(' '); PutChar(' '); PutChar(' ');
  i = ConsoleW-1; while (i--) PutChar('\b');
}



void PrintNewLine(void)
{
  PutChar('\n');
  NumPrintedLines++;
  HandleMore();
}



void PrintChar(uint8_t c)
{
  PutChar(c);
  if (CursorColumn == 0)
  {
    NumPrintedLines++;
    HandleMore();
  }
}



// does word wrapping; recognizes newline char
// print terminated by '^' or nullchar
void PrintText(const char *p)
{
  int flag = 0;

  for (;;)
  {
    const char *q = p;

    while (*q != ' ' && *q != '\n' && *q != 0 && *q != '^')
      q++;
    if (q - p)
      flag = 1;
    if (q - p > ConsoleW-CursorColumn)
      PrintNewLine();
    while (p < q)
      PrintChar(*p++);

    if (*p == ' ')
    {
      p++;
      if (flag == 0 || CursorColumn)
        PrintChar(' ');
    }
    else if (*p == '\n')
    {
      p++;
      if (flag == 0 || CursorColumn)
        PrintNewLine();
      flag = 0;
    }
    else
      break;
  }
}



// prints a newline automatically after text
// note: if text ends in a newline, automatic newline will not be printed
void PrintLine(const char *p)
{
  PrintText(p);
  if (CursorColumn) PrintNewLine();
}



// does word wrapping; recognizes newline char
// print terminated by '^' or nullchar
void PrintStreamRoutine(void)
{
  int n, c, flag = 0;

  for (;;)
  {
    SaveStream();
    n = 0;
    for (;;)
    {
      c = GetStream();
      if (c == ' ' || c == '\n' || c == 0 || c == '^')
        break;
      n++;
    }
    RestoreStream();
    if (n)
      flag = 1;
    if (n > ConsoleW-CursorColumn)
      PrintNewLine();
    while (n--)
      PrintChar(GetStream());

    if (c == ' ')
    {
      GetStream();
      if (flag == 0 || CursorColumn)
        PrintChar(' ');
    }
    else if (c == '\n')
    {
      GetStream();
      if (flag == 0 || CursorColumn)
        PrintNewLine();
      flag = 0;
    }
    else
      break;
  }
}



// returns 0 if nothing printed
uint8_t PrintTextSegmentPGM(int16_t segment)
{
  uint8_t c;

  // skip ^ separators
  while (segment)
  {
    c = GetStream();
    if (c == 0) return 0;
    if (c == '^') segment--;
  }

  SaveStream();
  c = GetStream();
  if (c == 0 || c == '^') return 0;
  RestoreStream();

  PrintStreamRoutine();
  return 1;
}



// returns number of characters "printed"
int PrintIntToBuffer(int num, char *buf)
{
  int neg, len, i, c;

  if (num == 0)
  {
    buf[0] = '0';
    buf[1] = 0;
    return 1;
  }

  if (num < 0)
    {num = -num; neg = 1;}
  else
    neg = 0;

  len = 0;
  while (num)
  {
    buf[len++] = '0' + (num % 10);
    num /= 10;
  }

  if (neg)
    buf[len++] = '-';

  buf[len] = 0;

  // reverse string
  for (i=0; i<len/2; i++)
  {
    c = buf[i];
    buf[i] = buf[len-1-i];
    buf[len-1-i] = c;
  }

  return len;
}



void PrintInteger(int num)
{
  char buf[80];

  PrintIntToBuffer(num, buf);
  PrintText(buf);
}

//*****************************************************************************



//*****************************************************************************

extern const uint8_t CompString[];
extern const uint16_t CompStringStartP[];
extern const uint8_t CompStringStartB[];

extern const uint8_t RoomDesc[];
extern const uint16_t RoomDescStartP[];
extern const uint8_t RoomDescStartB[];

extern const uint8_t BlockMsg[];
extern const uint16_t BlockMsgStartP[];
extern const uint8_t BlockMsgStartB[];

extern const uint8_t ObjectDesc[];
extern const uint16_t ObjectDescStartP[];
extern const uint8_t ObjectDescStartB[];



void PrintTextIndex(int16_t i)
{
  OpenStream(CompString);
  if (i/10 > 0)
  {
    StreamP += pgm_read_word(CompStringStartP + i/10 - 1);
    StreamB  = pgm_read_byte(CompStringStartB + i/10 - 1);
    i -= (i/10)*10;
  }
  AdvanceStream(i);

  PrintStreamRoutine();
}



// prints a newline automatically after text
// note: if text ends in a newline, automatic newline will not be printed
void PrintLineIndex(int16_t i)
{
  PrintTextIndex(i);
  if (CursorColumn) PrintNewLine();
}



void PrintRoomDescIndex(int16_t i, int16_t segment)
{
  OpenStream(RoomDesc);
  if (i/5 > 0)
  {
    StreamP += pgm_read_word(RoomDescStartP + i/5 - 1);
    StreamB  = pgm_read_byte(RoomDescStartB + i/5 - 1);
    i -= (i/5)*5;
  }
  AdvanceStream(i);

  if (PrintTextSegmentPGM(segment))
    if (CursorColumn) PrintNewLine();
}



#ifndef NO_STATUS_LINE



char StatusLine[40];



void PrintRoomDescIndexToStatusLine(int16_t i)
{
  int j, c = ' ';

  OpenStream(RoomDesc);
  if (i/5 > 0)
  {
    StreamP += pgm_read_word(RoomDescStartP + i/5 - 1);
    StreamB  = pgm_read_byte(RoomDescStartB + i/5 - 1);
    i -= (i/5)*5;
  }
  AdvanceStream(i);

  for (j=0; j<20; j++)
  {
    if (c)
    {
      c = GetStream();
      if (c == '^') c = 0;
    }
    StatusLine[j] = c;
  }
}



void PrintScoreAndMovesToStatusLine(void)
{
  int len, i;
  char buf[128];

  len = 6; memcpy(buf, "Score:", len);
  len += PrintIntToBuffer(Score, buf+len);
  buf[len++] = '/';
  len += PrintIntToBuffer(NumMoves, buf+len);

  for (i=0; i<20; i++)
    StatusLine[20+i] = (len-20+i < 0) ? 0 : buf[len-20+i];
}



void PrintStatusLine(void)
{
  int i, c;

  for (i=0; i<ConsoleW; i++)
  {
    if (i-1 >= 0 && i-1 < 20)
      c = StatusLine[i-1];
    else if (i-ConsoleW+40+1 >= 20 && i-ConsoleW+40+1 < 40)
      c = StatusLine[i-ConsoleW+40+1];
    else c = 0;

#ifdef __AVR_ATmega2560__
    ConsoleData[i] = 128+c;
#elif COMPILE_WINDOWS
    {
      char chr[2] = {(char)(c ? c : ' '), 0};
      WORD atr[2] = {(WORD)StatusLineColor, 0};
      COORD cp;
      DWORD num;
      cp.X = i;
      cp.Y = 0;
      WriteConsoleOutputCharacter(hStdOutput, chr, 1, cp, &num);
      WriteConsoleOutputAttribute(hStdOutput, atr, 1, cp, &num);
    }
#elif COMPILE_DOS
    {
      union REGS regs;
      regs.h.ah = 0x02; // set cursor position
      regs.h.bh = 0; // page number
      regs.h.dh = 0; // row
      regs.h.dl = i; // column
      int386(0x10, &regs, &regs);
      regs.h.ah = 0x09; // write character and attribute at cursor position
      regs.h.al = (c ? c : ' '); // character
      regs.h.bh = 0; // page number
      regs.h.bl = StatusLineColor; // background and foreground color
      regs.w.cx = 1; // number of times to print character
      int386(0x10, &regs, &regs);
    }
#endif
  }

#ifdef COMPILE_WINDOWS
  {
    COORD cp;
    cp.X = CursorColumn;
    cp.Y = CursorRow;
    SetConsoleCursorPosition(hStdOutput, cp);
  }
#elif COMPILE_DOS
  {
    union REGS regs;
    regs.h.ah = 0x02; // set cursor position
    regs.h.bh = 0; // page number
    regs.h.dh = CursorRow; // row
    regs.h.dl = CursorColumn; // column
    int386(0x10, &regs, &regs);
  }
#endif
}



#endif



void PrintBlockMsgIndex(int16_t i, int16_t segment)
{
  OpenStream(BlockMsg);
  if (i/5 > 0)
  {
    StreamP += pgm_read_word(BlockMsgStartP + i/5 - 1);
    StreamB  = pgm_read_byte(BlockMsgStartB + i/5 - 1);
    i -= (i/5)*5;
  }
  AdvanceStream(i);

  if (PrintTextSegmentPGM(segment))
    if (CursorColumn) PrintNewLine();
}



void PrintObjectDescIndex(int16_t i, int16_t segment)
{
  OpenStream(ObjectDesc);
  if (i/5 > 0)
  {
    StreamP += pgm_read_word(ObjectDescStartP + i/5 - 1);
    StreamB  = pgm_read_byte(ObjectDescStartB + i/5 - 1);
    i -= (i/5)*5;
  }
  AdvanceStream(i);

  if (PrintTextSegmentPGM(segment))
    if (segment && CursorColumn) PrintNewLine();
}

//*****************************************************************************



//*****************************************************************************

struct {const short action; const short obj;} const DoMiscWithTo[] PROGMEM =
{
  { A_TIE        , OBJ_ROPE            },
  { A_TIE        , FOBJ_RAILING        },
  { A_UNTIE      , OBJ_ROPE            },
  { A_TURN       , FOBJ_BOLT           },
  { A_FIX        , FOBJ_LEAK           },
  { A_INFLATE    , OBJ_INFLATABLE_BOAT },
  { A_INFLATE    , OBJ_INFLATED_BOAT   },
  { A_INFLATE    , OBJ_PUNCTURED_BOAT  },
  { A_FILL       , OBJ_INFLATABLE_BOAT },
  { A_FILL       , OBJ_INFLATED_BOAT   },
  { A_FILL       , OBJ_PUNCTURED_BOAT  },
  { A_DEFLATE    , OBJ_INFLATED_BOAT   },
  { A_DEFLATE    , OBJ_INFLATABLE_BOAT },
  { A_DEFLATE    , OBJ_PUNCTURED_BOAT  },
  { A_FIX        , OBJ_PUNCTURED_BOAT  },
  { A_LOCK       , FOBJ_GRATE          },
  { A_UNLOCK     , FOBJ_GRATE          },
  { A_ACTIVATE   , OBJ_LAMP            },
  { A_DEACTIVATE , OBJ_LAMP            },
  { A_ACTIVATE   , OBJ_MATCH           },
  { A_DEACTIVATE , OBJ_MATCH           },
  { A_ACTIVATE   , OBJ_CANDLES         },
  { A_DEACTIVATE , OBJ_CANDLES         },
  { A_ACTIVATE   , OBJ_MACHINE         },
  { A_ACTIVATE   , FOBJ_MACHINE_SWITCH },
  { A_TURN       , FOBJ_MACHINE_SWITCH },
  { A_DIG        , FOBJ_SAND           },
  { A_FILL       , OBJ_BOTTLE          },
  { A_ATTACK     , OBJ_BAT             },
  { A_ATTACK     , OBJ_GHOSTS          },
  { A_ATTACK     , OBJ_CYCLOPS         },
  { A_ATTACK     , OBJ_THIEF           },
  { A_ATTACK     , OBJ_TROLL           },
  { A_ATTACK     , OBJ_YOU             },
  { A_ACTIVATE   , OBJ_LEAVES          },
  { A_ACTIVATE   , OBJ_BOOK            },
  { A_ACTIVATE   , OBJ_SANDWICH_BAG    },
  { A_ACTIVATE   , OBJ_ADVERTISEMENT   },
  { A_ACTIVATE   , OBJ_INFLATED_BOAT   },
  { A_ACTIVATE   , OBJ_PAINTING        },
  { A_ACTIVATE   , OBJ_PUNCTURED_BOAT  },
  { A_ACTIVATE   , OBJ_INFLATABLE_BOAT },
  { A_ACTIVATE   , OBJ_COAL            },
  { A_ACTIVATE   , OBJ_BOAT_LABEL      },
  { A_ACTIVATE   , OBJ_GUIDE           },
  { A_ACTIVATE   , OBJ_NEST            },
  { A_ACTIVATE   , FOBJ_WHITE_HOUSE    },
  { A_ACTIVATE   , FOBJ_FRONT_DOOR     },
  { A_ACTIVATE   , OBJ_TORCH           },
  { A_DEACTIVATE , OBJ_TORCH           },
  { A_TURN       , OBJ_BOOK            },
  { A_POUR       , OBJ_WATER           },
  { A_POUR       , OBJ_PUTTY           },
  { A_OIL        , FOBJ_BOLT           },
  { A_BRUSH      , OBJ_YOU             },
  { A_TIE        , OBJ_CYCLOPS         },
  { A_TIE        , OBJ_THIEF           },
  { A_TIE        , OBJ_TROLL           },

  { 0, 0 }
};



int GetDoMiscWithTo_Action(int i)
{
  return pgm_read_word(&DoMiscWithTo[i].action);
}

int GetDoMiscWithTo_Obj(int i)
{
  return pgm_read_word(&DoMiscWithTo[i].obj);
}

//*****************************************************************************



//*****************************************************************************

struct {const short action; const short obj;} const DoMisc[] PROGMEM =
{
  { A_OPEN         , FOBJ_KITCHEN_WINDOW  },
  { A_CLOSE        , FOBJ_KITCHEN_WINDOW  },
  { A_MOVE         , FOBJ_RUG             },
  { A_PUSH         , FOBJ_RUG             },
  { A_OPEN         , FOBJ_TRAP_DOOR       },
  { A_CLOSE        , FOBJ_TRAP_DOOR       },
  { A_RAISE        , OBJ_RAISED_BASKET    },
  { A_RAISE        , OBJ_LOWERED_BASKET   },
  { A_LOWER        , OBJ_RAISED_BASKET    },
  { A_LOWER        , OBJ_LOWERED_BASKET   },
  { A_PUSH         , FOBJ_BLUE_BUTTON     },
  { A_PUSH         , FOBJ_RED_BUTTON      },
  { A_PUSH         , FOBJ_BROWN_BUTTON    },
  { A_PUSH         , FOBJ_YELLOW_BUTTON   },
  { A_ENTER        , OBJ_INFLATED_BOAT    },
  { A_EXIT         , OBJ_INFLATED_BOAT    },
  { A_MOVE         , OBJ_LEAVES           },
  { A_OPEN         , FOBJ_GRATE           },
  { A_CLOSE        , FOBJ_GRATE           },
  { A_RING         , OBJ_BELL             },
  { A_WIND         , OBJ_CANARY           },
  { A_WIND         , OBJ_BROKEN_CANARY    },
  { A_WAVE         , OBJ_SCEPTRE          },
  { A_RAISE        , OBJ_SCEPTRE          },
  { A_TOUCH        , FOBJ_MIRROR1         },
  { A_TOUCH        , FOBJ_MIRROR2         },
  { A_READ         , OBJ_BOOK             },
  { A_READ         , OBJ_ADVERTISEMENT    },
  { A_READ         , OBJ_MATCH            },
  { A_READ         , OBJ_MAP              },
  { A_READ         , OBJ_BOAT_LABEL       },
  { A_READ         , OBJ_GUIDE            },
  { A_READ         , OBJ_TUBE             },
  { A_READ         , OBJ_OWNERS_MANUAL    },
  { A_READ         , FOBJ_PRAYER          },
  { A_READ         , FOBJ_WOODEN_DOOR     },
  { A_READ         , OBJ_ENGRAVINGS       },
  { A_OPEN         , OBJ_EGG              },
  { A_BREAK        , OBJ_EGG              },
  { A_PRY          , OBJ_EGG              },
  { A_CLIMBTHROUGH , FOBJ_KITCHEN_WINDOW  },
  { A_ENTER        , FOBJ_KITCHEN_WINDOW  },
  { A_EXIT         , FOBJ_KITCHEN_WINDOW  },
  { A_CLIMBTHROUGH , FOBJ_TRAP_DOOR       },
  { A_ENTER        , FOBJ_TRAP_DOOR       },
  { A_CLIMBTHROUGH , FOBJ_GRATE           },
  { A_ENTER        , FOBJ_GRATE           },
  { A_CLIMBTHROUGH , FOBJ_SLIDE           },
  { A_ENTER        , FOBJ_SLIDE           },
  { A_CLIMBTHROUGH , FOBJ_CHIMNEY         },
  { A_ENTER        , FOBJ_CHIMNEY         },
  { A_CLIMBTHROUGH , FOBJ_BARROW_DOOR     },
  { A_ENTER        , FOBJ_BARROW_DOOR     },
  { A_ENTER        , FOBJ_BARROW          },
  { A_CLIMBTHROUGH , FOBJ_GATE            },
  { A_ENTER        , FOBJ_GATE            },
  { A_CLIMBTHROUGH , FOBJ_CRACK           },
  { A_ENTER        , FOBJ_CRACK           },
  { A_ENTER        , FOBJ_WHITE_HOUSE     },
  { A_SLIDEDOWN    , FOBJ_SLIDE           },
  { A_CLIMBUP      , FOBJ_MOUNTAIN_RANGE  },
  { A_CLIMB        , FOBJ_MOUNTAIN_RANGE  },
  { A_CLIMBUP      , FOBJ_WHITE_CLIFF     },
  { A_CLIMB        , FOBJ_WHITE_CLIFF     },
  { A_CLIMBUP      , FOBJ_TREE            },
  { A_CLIMB        , FOBJ_TREE            },
  { A_CLIMBDOWN    , FOBJ_TREE            },
  { A_CLIMBUP      , FOBJ_CHIMNEY         },
  { A_CLIMB        , FOBJ_CHIMNEY         },
  { A_CLIMBDOWN    , FOBJ_CHIMNEY         },
  { A_CLIMBUP      , FOBJ_LADDER          },
  { A_CLIMB        , FOBJ_LADDER          },
  { A_CLIMBDOWN    , FOBJ_LADDER          },
  { A_CLIMBUP      , FOBJ_SLIDE           },
  { A_CLIMB        , FOBJ_SLIDE           },
  { A_CLIMBDOWN    , FOBJ_SLIDE           },
  { A_CLIMBUP      , FOBJ_CLIMBABLE_CLIFF },
  { A_CLIMB        , FOBJ_CLIMBABLE_CLIFF },
  { A_CLIMBDOWN    , FOBJ_CLIMBABLE_CLIFF },
  { A_CLIMBUP      , FOBJ_STAIRS          },
  { A_CLIMB        , FOBJ_STAIRS          },
  { A_CLIMBDOWN    , FOBJ_STAIRS          },
  { A_EXAMINE      , OBJ_SWORD            },
  { A_EXAMINE      , OBJ_MATCH            },
  { A_EXAMINE      , OBJ_CANDLES          },
  { A_EXAMINE      , OBJ_TORCH            },
  { A_EXAMINE      , OBJ_THIEF            },
  { A_EXAMINE      , OBJ_TOOL_CHEST       },
  { A_EXAMINE      , FOBJ_BOARD           },
  { A_EXAMINE      , FOBJ_CHAIN           },
  { A_OPEN         , OBJ_TOOL_CHEST       },
  { A_OPEN         , OBJ_BOOK             },
  { A_CLOSE        , OBJ_BOOK             },
  { A_OPEN         , FOBJ_BOARDED_WINDOW  },
  { A_BREAK        , FOBJ_BOARDED_WINDOW  },
  { A_OPEN         , FOBJ_DAM             },
  { A_CLOSE        , FOBJ_DAM             },
  { A_RING         , OBJ_HOT_BELL         },
  { A_READ         , FOBJ_YELLOW_BUTTON   },
  { A_READ         , FOBJ_BROWN_BUTTON    },
  { A_READ         , FOBJ_RED_BUTTON      },
  { A_READ         , FOBJ_BLUE_BUTTON     },
  { A_RAISE        , FOBJ_GRANITE_WALL    },
  { A_LOWER        , FOBJ_GRANITE_WALL    },
  { A_RAISE        , FOBJ_CHAIN           },
  { A_LOWER        , FOBJ_CHAIN           },
  { A_MOVE         , FOBJ_CHAIN           },
  { A_COUNT        , OBJ_CANDLES          },
  { A_COUNT        , OBJ_LEAVES           },
  { A_EXAMINE      , OBJ_LAMP             },
  { A_EXAMINE      , OBJ_TROLL            },
  { A_EXAMINE      , OBJ_CYCLOPS          },
  { A_EXAMINE      , FOBJ_WHITE_HOUSE     },
  { A_OPEN         , FOBJ_BARROW_DOOR     },
  { A_CLOSE        , FOBJ_BARROW_DOOR     },
  { A_OPEN         , FOBJ_STUDIO_DOOR     },
  { A_CLOSE        , FOBJ_STUDIO_DOOR     },
  { A_OPEN         , OBJ_BAG_OF_COINS     },
  { A_CLOSE        , OBJ_BAG_OF_COINS     },
  { A_OPEN         , OBJ_TRUNK            },
  { A_CLOSE        , OBJ_TRUNK            },
  { A_OPEN         , OBJ_LARGE_BAG        },
  { A_CLOSE        , OBJ_LARGE_BAG        },
  { A_OPEN         , FOBJ_FRONT_DOOR      },
  { A_COUNT        , OBJ_MATCH            },
  { A_OPEN         , OBJ_MATCH            },
  { A_EAT          , OBJ_LUNCH            },
  { A_EAT          , OBJ_GARLIC           },
  { A_DRINK        , OBJ_WATER            },
  { A_CLIMBDOWN    , OBJ_ROPE             },
  { A_BREAK        , FOBJ_MIRROR1         },
  { A_BREAK        , FOBJ_MIRROR2         },
  { A_LOOKIN       , FOBJ_MIRROR1         },
  { A_LOOKIN       , FOBJ_MIRROR2         },
  { A_EXAMINE      , FOBJ_MIRROR1         },
  { A_EXAMINE      , FOBJ_MIRROR2         },
  { A_LOOKTHROUGH  , FOBJ_KITCHEN_WINDOW  },
  { A_LOOKIN       , FOBJ_KITCHEN_WINDOW  },
  { A_LOOKUNDER    , FOBJ_RUG             },
  { A_LOOKUNDER    , OBJ_LEAVES           },
  { A_LOOKUNDER    , FOBJ_RAINBOW         },
  { A_LOOKIN       , FOBJ_CHIMNEY         },
  { A_EXAMINE      , FOBJ_CHIMNEY         },
  { A_EXAMINE      , FOBJ_KITCHEN_WINDOW  },
  { A_LOOKIN       , OBJ_BAG_OF_COINS     },
  { A_EXAMINE      , OBJ_BAG_OF_COINS     },
  { A_LOOKIN       , OBJ_TRUNK            },
  { A_EXAMINE      , OBJ_TRUNK            },
  { A_SQUEEZE      , OBJ_TUBE             },
  { A_EXAMINE      , OBJ_RAISED_BASKET    },
  { A_EXAMINE      , OBJ_LOWERED_BASKET   },
  { A_LOOKIN       , OBJ_LARGE_BAG        },
  { A_EXAMINE      , OBJ_LARGE_BAG        },
  { A_LOOKTHROUGH  , FOBJ_GRATE           },
  { A_LOOKIN       , FOBJ_GRATE           },
  { A_EXAMINE      , OBJ_WATER            },
  { A_LOOKIN       , OBJ_WATER            },
  { A_WHEREIS      , FOBJ_GRANITE_WALL    },
  { A_WHEREIS      , FOBJ_SONGBIRD        },
  { A_WHEREIS      , FOBJ_WHITE_HOUSE     },
  { A_WHEREIS      , FOBJ_FOREST          },
  { A_READ         , FOBJ_GRANITE_WALL    },
  { A_EXAMINE      , OBJ_ZORKMID          },
  { A_EXAMINE      , OBJ_GRUE             },
  { A_WHEREIS      , OBJ_ZORKMID          },
  { A_WHEREIS      , OBJ_GRUE             },
  { A_LISTENTO     , OBJ_TROLL            },
  { A_LISTENTO     , OBJ_THIEF            },
  { A_LISTENTO     , OBJ_CYCLOPS          },
  { A_LISTENTO     , FOBJ_FOREST          },
  { A_LISTENTO     , FOBJ_SONGBIRD        },
  { A_CROSS        , FOBJ_RAINBOW         },
  { A_CROSS        , FOBJ_LAKE            },
  { A_CROSS        , FOBJ_STREAM          },
  { A_CROSS        , FOBJ_CHASM           },
  { A_EXORCISE     , OBJ_GHOSTS           },
  { A_RAISE        , FOBJ_RUG             },
  { A_RAISE        , FOBJ_TRAP_DOOR       },
  { A_SMELL        , FOBJ_GAS             },
  { A_SMELL        , OBJ_SANDWICH_BAG     },

  { 0, 0 }
};



int GetDoMisc_Action(int i)
{
  return pgm_read_word(&DoMisc[i].action);
}

int GetDoMisc_Obj(int i)
{
  return pgm_read_word(&DoMisc[i].obj);
}

//*****************************************************************************



//*****************************************************************************

//A_IN and A_OUT can also be handled here
struct {const int room; const int action;} const GoFrom[] PROGMEM =
{
  { ROOM_STONE_BARROW         , A_WEST      },
  { ROOM_STONE_BARROW         , A_IN        },
  { ROOM_WEST_OF_HOUSE        , A_SOUTHWEST },
  { ROOM_WEST_OF_HOUSE        , A_IN        },
  { ROOM_EAST_OF_HOUSE        , A_WEST      },
  { ROOM_EAST_OF_HOUSE        , A_IN        },
  { ROOM_KITCHEN              , A_EAST      },
  { ROOM_KITCHEN              , A_OUT       },
  { ROOM_LIVING_ROOM          , A_WEST      },
  { ROOM_CELLAR               , A_UP        },
  { ROOM_TROLL_ROOM           , A_EAST      },
  { ROOM_TROLL_ROOM           , A_WEST      },
  { ROOM_GRATING_ROOM         , A_UP        },
  { ROOM_CYCLOPS_ROOM         , A_EAST      },
  { ROOM_CYCLOPS_ROOM         , A_UP        },
  { ROOM_RESERVOIR_SOUTH      , A_NORTH     },
  { ROOM_RESERVOIR_NORTH      , A_SOUTH     },
  { ROOM_ENTRANCE_TO_HADES    , A_SOUTH     },
  { ROOM_ENTRANCE_TO_HADES    , A_IN        },
  { ROOM_DOME_ROOM            , A_DOWN      },
  { ROOM_ARAGAIN_FALLS        , A_WEST      },
  { ROOM_ARAGAIN_FALLS        , A_UP        },
  { ROOM_END_OF_RAINBOW       , A_UP        },
  { ROOM_END_OF_RAINBOW       , A_NORTHEAST },
  { ROOM_END_OF_RAINBOW       , A_EAST      },
  { ROOM_MAZE_2               , A_DOWN      },
  { ROOM_MAZE_7               , A_DOWN      },
  { ROOM_MAZE_9               , A_DOWN      },
  { ROOM_MAZE_12              , A_DOWN      },
  { ROOM_GRATING_CLEARING     , A_DOWN      },
  { ROOM_LIVING_ROOM          , A_DOWN      },
  { ROOM_SOUTH_TEMPLE         , A_DOWN      },
  { ROOM_WHITE_CLIFFS_NORTH   , A_SOUTH     },
  { ROOM_WHITE_CLIFFS_NORTH   , A_WEST      },
  { ROOM_WHITE_CLIFFS_SOUTH   , A_NORTH     },
  { ROOM_TIMBER_ROOM          , A_WEST      },
  { ROOM_LOWER_SHAFT          , A_EAST      },
  { ROOM_LOWER_SHAFT          , A_OUT       },
  { ROOM_KITCHEN              , A_DOWN      },
  { ROOM_STUDIO               , A_UP        },
  { ROOM_LAND_OF_LIVING_DEAD  , A_OUT       },
  { ROOM_STRANGE_PASSAGE      , A_IN        },
  { ROOM_NORTH_TEMPLE         , A_OUT       },
  { ROOM_MINE_ENTRANCE        , A_IN        },
  { ROOM_DAM_LOBBY            , A_NORTH     },
  { ROOM_DAM_LOBBY            , A_EAST      },

  { 0, 0 }
};



int GetGoFrom_Room(int i)
{
  return pgm_read_word(&GoFrom[i].room);
}

int GetGoFrom_Action(int i)
{
  return pgm_read_word(&GoFrom[i].action);
}

//*****************************************************************************



//*****************************************************************************

const unsigned short ObjStartLoc[NUM_OBJECTS] PROGMEM =
{
  /* OBJ_NOTHING            */  0                            ,
  /* OBJ_YOU                */  ROOM_WEST_OF_HOUSE           ,
  /* OBJ_CYCLOPS            */  ROOM_CYCLOPS_ROOM            ,
  /* OBJ_GHOSTS             */  ROOM_ENTRANCE_TO_HADES       ,
  /* OBJ_BAT                */  ROOM_BAT_ROOM                ,
  /* OBJ_THIEF              */  ROOM_ROUND_ROOM              ,
  /* OBJ_TROLL              */  ROOM_TROLL_ROOM              ,
  /* OBJ_LOWERED_BASKET     */  ROOM_LOWER_SHAFT             ,
  /* OBJ_RAISED_BASKET      */  ROOM_SHAFT_ROOM              ,
  /* OBJ_TROPHY_CASE        */  ROOM_LIVING_ROOM             ,
  /* OBJ_MACHINE            */  ROOM_MACHINE_ROOM            ,
  /* OBJ_MAILBOX            */  ROOM_WEST_OF_HOUSE           ,
  /* OBJ_KITCHEN_TABLE      */  ROOM_KITCHEN                 ,
  /* OBJ_ATTIC_TABLE        */  ROOM_ATTIC                   ,
  /* OBJ_WATER              */    INSIDE + OBJ_BOTTLE        ,
  /* OBJ_SKULL              */  ROOM_LAND_OF_LIVING_DEAD     ,
  /* OBJ_TIMBERS            */  ROOM_TIMBER_ROOM             ,
  /* OBJ_LUNCH              */    INSIDE + OBJ_SANDWICH_BAG  ,
  /* OBJ_BELL               */  ROOM_NORTH_TEMPLE            ,
  /* OBJ_HOT_BELL           */  0                            ,
  /* OBJ_BOOK               */  ROOM_SOUTH_TEMPLE            ,
  /* OBJ_AXE                */    INSIDE + OBJ_TROLL         ,
  /* OBJ_BROKEN_LAMP        */  0                            ,
  /* OBJ_SCEPTRE            */    INSIDE + OBJ_COFFIN        ,
  /* OBJ_SANDWICH_BAG       */    INSIDE + OBJ_KITCHEN_TABLE ,
  /* OBJ_CHALICE            */  ROOM_TREASURE_ROOM           ,
  /* OBJ_GARLIC             */    INSIDE + OBJ_SANDWICH_BAG  ,
  /* OBJ_TRIDENT            */  ROOM_ATLANTIS_ROOM           ,
  /* OBJ_BOTTLE             */    INSIDE + OBJ_KITCHEN_TABLE ,
  /* OBJ_COFFIN             */  ROOM_EGYPT_ROOM              ,
  /* OBJ_PUMP               */  ROOM_RESERVOIR_NORTH         ,
  /* OBJ_DIAMOND            */  0                            ,
  /* OBJ_JADE               */  ROOM_BAT_ROOM                ,
  /* OBJ_KNIFE              */    INSIDE + OBJ_ATTIC_TABLE   ,
  /* OBJ_BURNED_OUT_LANTERN */  ROOM_MAZE_5                  ,
  /* OBJ_BAG_OF_COINS       */  ROOM_MAZE_5                  ,
  /* OBJ_LAMP               */  ROOM_LIVING_ROOM             ,
  /* OBJ_EMERALD            */    INSIDE + OBJ_BUOY          ,
  /* OBJ_ADVERTISEMENT      */    INSIDE + OBJ_MAILBOX       ,
  /* OBJ_INFLATED_BOAT      */  0                            ,
  /* OBJ_MATCH              */  ROOM_DAM_LOBBY               ,
  /* OBJ_PAINTING           */  ROOM_GALLERY                 ,
  /* OBJ_CANDLES            */  ROOM_SOUTH_TEMPLE            ,
  /* OBJ_GUNK               */  0                            ,
  /* OBJ_LEAVES             */  ROOM_GRATING_CLEARING        ,
  /* OBJ_PUNCTURED_BOAT     */  0                            ,
  /* OBJ_INFLATABLE_BOAT    */  ROOM_DAM_BASE                ,
  /* OBJ_BAR                */  ROOM_LOUD_ROOM               ,
  /* OBJ_POT_OF_GOLD        */  ROOM_END_OF_RAINBOW          ,
  /* OBJ_BUOY               */  ROOM_RIVER_4                 ,
  /* OBJ_ROPE               */  ROOM_ATTIC                   ,
  /* OBJ_RUSTY_KNIFE        */  ROOM_MAZE_5                  ,
  /* OBJ_BRACELET           */  ROOM_GAS_ROOM                ,
  /* OBJ_TOOL_CHEST         */  ROOM_MAINTENANCE_ROOM        ,
  /* OBJ_SCREWDRIVER        */  ROOM_MAINTENANCE_ROOM        ,
  /* OBJ_KEYS               */  ROOM_MAZE_5                  ,
  /* OBJ_SHOVEL             */  ROOM_SANDY_BEACH             ,
  /* OBJ_COAL               */  ROOM_DEAD_END_5              ,
  /* OBJ_SCARAB             */  ROOM_SANDY_CAVE              ,
  /* OBJ_LARGE_BAG          */    INSIDE + OBJ_THIEF         ,
  /* OBJ_STILETTO           */    INSIDE + OBJ_THIEF         ,
  /* OBJ_SWORD              */  ROOM_LIVING_ROOM             ,
  /* OBJ_MAP                */    INSIDE + OBJ_TROPHY_CASE   ,
  /* OBJ_BOAT_LABEL         */    INSIDE + OBJ_INFLATED_BOAT ,
  /* OBJ_TORCH              */  ROOM_TORCH_ROOM              ,
  /* OBJ_GUIDE              */  ROOM_DAM_LOBBY               ,
  /* OBJ_TRUNK              */  ROOM_RESERVOIR               ,
  /* OBJ_TUBE               */  ROOM_MAINTENANCE_ROOM        ,
  /* OBJ_PUTTY              */    INSIDE + OBJ_TUBE          ,
  /* OBJ_OWNERS_MANUAL      */  ROOM_STUDIO                  ,
  /* OBJ_WRENCH             */  ROOM_MAINTENANCE_ROOM        ,
  /* OBJ_NEST               */  ROOM_UP_A_TREE               ,
  /* OBJ_EGG                */    INSIDE + OBJ_NEST          ,
  /* OBJ_BROKEN_EGG         */  0                            ,
  /* OBJ_BAUBLE             */  0                            ,
  /* OBJ_CANARY             */    INSIDE + OBJ_EGG           ,
  /* OBJ_BROKEN_CANARY      */    INSIDE + OBJ_BROKEN_EGG    ,
  /* OBJ_ENGRAVINGS         */  ROOM_ENGRAVINGS_CAVE         ,
  /* OBJ_ZORKMID            */  0                            ,
  /* OBJ_GRUE               */  0
};



void InitObjectLocations(void)
{
  int i;

  for (i=0; i<NUM_OBJECTS; i++)
    Obj[i].loc = pgm_read_word(ObjStartLoc + i);
}



const unsigned short ObjectStartProp[NUM_OBJECTS] PROGMEM =
{
  /* OBJ_NOTHING            */  0                                                          ,
  /* OBJ_YOU                */  0                                                          ,
  /* OBJ_CYCLOPS            */  PROP_NOTTAKEABLE | PROP_ACTOR                              ,
  /* OBJ_GHOSTS             */  PROP_NOTTAKEABLE | PROP_ACTOR                              ,
  /* OBJ_BAT                */  PROP_NOTTAKEABLE | PROP_ACTOR                              ,
  /* OBJ_THIEF              */  PROP_NOTTAKEABLE | PROP_INVISIBLE | PROP_OPEN | PROP_ACTOR ,
  /* OBJ_TROLL              */  PROP_NOTTAKEABLE | PROP_OPEN | PROP_ACTOR                  ,
  /* OBJ_LOWERED_BASKET     */  PROP_NOTTAKEABLE | PROP_OPEN                               ,
  /* OBJ_RAISED_BASKET      */  PROP_NOTTAKEABLE | PROP_OPEN                               ,
  /* OBJ_TROPHY_CASE        */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_OPENABLE             ,
  /* OBJ_MACHINE            */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_OPENABLE             ,
  /* OBJ_MAILBOX            */  PROP_NOTTAKEABLE | PROP_OPENABLE                           ,
  /* OBJ_KITCHEN_TABLE      */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_OPEN | PROP_SURFACE  ,
  /* OBJ_ATTIC_TABLE        */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_OPEN | PROP_SURFACE  ,
  /* OBJ_WATER              */  PROP_NOTTAKEABLE | PROP_OPEN | PROP_EVERYWHERE             ,
  /* OBJ_SKULL              */  0                                                          ,
  /* OBJ_TIMBERS            */  0                                                          ,
  /* OBJ_LUNCH              */  0                                                          ,
  /* OBJ_BELL               */  0                                                          ,
  /* OBJ_HOT_BELL           */  PROP_NOTTAKEABLE                                           ,
  /* OBJ_BOOK               */  PROP_INFLAMMABLE                                           ,
  /* OBJ_AXE                */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_WEAPON               ,
  /* OBJ_BROKEN_LAMP        */  0                                                          ,
  /* OBJ_SCEPTRE            */  PROP_INSIDEDESC | PROP_WEAPON                              ,
  /* OBJ_SANDWICH_BAG       */  PROP_OPENABLE | PROP_INSIDEDESC | PROP_INFLAMMABLE         ,
  /* OBJ_CHALICE            */  PROP_OPEN                                                  ,
  /* OBJ_GARLIC             */  0                                                          ,
  /* OBJ_TRIDENT            */  0                                                          ,
  /* OBJ_BOTTLE             */  PROP_OPENABLE | PROP_INSIDEDESC                            ,
  /* OBJ_COFFIN             */  PROP_OPENABLE | PROP_SACRED                                ,
  /* OBJ_PUMP               */  0                                                          ,
  /* OBJ_DIAMOND            */  0                                                          ,
  /* OBJ_JADE               */  0                                                          ,
  /* OBJ_KNIFE              */  PROP_INSIDEDESC | PROP_WEAPON                              ,
  /* OBJ_BURNED_OUT_LANTERN */  0                                                          ,
  /* OBJ_BAG_OF_COINS       */  0                                                          ,
  /* OBJ_LAMP               */  0                                                          ,
  /* OBJ_EMERALD            */  0                                                          ,
  /* OBJ_ADVERTISEMENT      */  PROP_INFLAMMABLE                                           ,
  /* OBJ_INFLATED_BOAT      */  PROP_OPEN | PROP_INFLAMMABLE                               ,
  /* OBJ_MATCH              */  0                                                          ,
  /* OBJ_PAINTING           */  PROP_INFLAMMABLE                                           ,
  /* OBJ_CANDLES            */  PROP_LIT                                                   ,
  /* OBJ_GUNK               */  0                                                          ,
  /* OBJ_LEAVES             */  PROP_INFLAMMABLE                                           ,
  /* OBJ_PUNCTURED_BOAT     */  PROP_INFLAMMABLE                                           ,
  /* OBJ_INFLATABLE_BOAT    */  PROP_INFLAMMABLE                                           ,
  /* OBJ_BAR                */  PROP_SACRED                                                ,
  /* OBJ_POT_OF_GOLD        */  PROP_INVISIBLE                                             ,
  /* OBJ_BUOY               */  PROP_OPENABLE                                              ,
  /* OBJ_ROPE               */  PROP_SACRED                                                ,
  /* OBJ_RUSTY_KNIFE        */  PROP_WEAPON                                                ,
  /* OBJ_BRACELET           */  0                                                          ,
  /* OBJ_TOOL_CHEST         */  PROP_NOTTAKEABLE                                           ,
  /* OBJ_SCREWDRIVER        */  0                                                          ,
  /* OBJ_KEYS               */  0                                                          ,
  /* OBJ_SHOVEL             */  0                                                          ,
  /* OBJ_COAL               */  PROP_INFLAMMABLE                                           ,
  /* OBJ_SCARAB             */  PROP_INVISIBLE                                             ,
  /* OBJ_LARGE_BAG          */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_OPENABLE | PROP_OPEN ,
  /* OBJ_STILETTO           */  PROP_NOTTAKEABLE | PROP_NODESC | PROP_WEAPON               ,
  /* OBJ_SWORD              */  PROP_WEAPON                                                ,
  /* OBJ_MAP                */  PROP_INVISIBLE | PROP_INSIDEDESC                           ,
  /* OBJ_BOAT_LABEL         */  PROP_INFLAMMABLE                                           ,
  /* OBJ_TORCH              */  PROP_LIT                                                   ,
  /* OBJ_GUIDE              */  PROP_INFLAMMABLE                                           ,
  /* OBJ_TRUNK              */  PROP_INVISIBLE                                             ,
  /* OBJ_TUBE               */  PROP_OPENABLE                                              ,
  /* OBJ_PUTTY              */  0                                                          ,
  /* OBJ_OWNERS_MANUAL      */  0                                                          ,
  /* OBJ_WRENCH             */  0                                                          ,
  /* OBJ_NEST               */  PROP_OPEN | PROP_INFLAMMABLE                               ,
  /* OBJ_EGG                */  PROP_INSIDEDESC                                            ,
  /* OBJ_BROKEN_EGG         */  0                                                          ,
  /* OBJ_BAUBLE             */  0                                                          ,
  /* OBJ_CANARY             */  PROP_INSIDEDESC                                            ,
  /* OBJ_BROKEN_CANARY      */  PROP_INSIDEDESC                                            ,
  /* OBJ_ENGRAVINGS         */  PROP_NOTTAKEABLE                                           ,
  /* OBJ_ZORKMID            */  PROP_NOTTAKEABLE | PROP_INVISIBLE | PROP_EVERYWHERE        ,
  /* OBJ_GRUE               */  PROP_NOTTAKEABLE | PROP_INVISIBLE | PROP_EVERYWHERE
};



void InitObjectProperties(void)
{
  int i;

  for (i=0; i<NUM_OBJECTS; i++)
    Obj[i].prop = pgm_read_word(ObjectStartProp + i);
}



const unsigned short ObjectSizeCapacity[2*NUM_OBJECTS] PROGMEM =
{
  //                            size  capacity

  /* OBJ_NOTHING            */   0,      0,
  /* OBJ_YOU                */   0,      0,
  /* OBJ_CYCLOPS            */   0,      0,
  /* OBJ_GHOSTS             */   0,      0,
  /* OBJ_BAT                */   0,      0,
  /* OBJ_THIEF              */   0,      0,
  /* OBJ_TROLL              */   0,      0,
  /* OBJ_LOWERED_BASKET     */   0,      0,
  /* OBJ_RAISED_BASKET      */   0,     50,
  /* OBJ_TROPHY_CASE        */   0,  10000,
  /* OBJ_MACHINE            */   0,     50,
  /* OBJ_MAILBOX            */   0,     10,
  /* OBJ_KITCHEN_TABLE      */   0,     50,
  /* OBJ_ATTIC_TABLE        */   0,     40,
  /* OBJ_WATER              */   4,      0,
  /* OBJ_SKULL              */   0,      0,
  /* OBJ_TIMBERS            */  50,      0,
  /* OBJ_LUNCH              */   0,      0,
  /* OBJ_BELL               */   0,      0,
  /* OBJ_HOT_BELL           */   0,      0,
  /* OBJ_BOOK               */  10,      0,
  /* OBJ_AXE                */  25,      0,
  /* OBJ_BROKEN_LAMP        */   0,      0,
  /* OBJ_SCEPTRE            */   3,      0,
  /* OBJ_SANDWICH_BAG       */   9,      9,
  /* OBJ_CHALICE            */  10,      5,
  /* OBJ_GARLIC             */   4,      0,
  /* OBJ_TRIDENT            */  20,      0,
  /* OBJ_BOTTLE             */   0,      4,
  /* OBJ_COFFIN             */  55,     35,
  /* OBJ_PUMP               */   0,      0,
  /* OBJ_DIAMOND            */   0,      0,
  /* OBJ_JADE               */  10,      0,
  /* OBJ_KNIFE              */   0,      0,
  /* OBJ_BURNED_OUT_LANTERN */  20,      0,
  /* OBJ_BAG_OF_COINS       */  15,      0,
  /* OBJ_LAMP               */  15,      0,
  /* OBJ_EMERALD            */   0,      0,
  /* OBJ_ADVERTISEMENT      */   2,      0,
  /* OBJ_INFLATED_BOAT      */  20,    100,
  /* OBJ_MATCH              */   2,      0,
  /* OBJ_PAINTING           */  15,      0,
  /* OBJ_CANDLES            */  10,      0,
  /* OBJ_GUNK               */  10,      0,
  /* OBJ_LEAVES             */  25,      0,
  /* OBJ_PUNCTURED_BOAT     */  20,      0,
  /* OBJ_INFLATABLE_BOAT    */  20,      0,
  /* OBJ_BAR                */  20,      0,
  /* OBJ_POT_OF_GOLD        */  15,      0,
  /* OBJ_BUOY               */  10,     20,
  /* OBJ_ROPE               */  10,      0,
  /* OBJ_RUSTY_KNIFE        */  20,      0,
  /* OBJ_BRACELET           */  10,      0,
  /* OBJ_TOOL_CHEST         */   0,      0,
  /* OBJ_SCREWDRIVER        */   0,      0,
  /* OBJ_KEYS               */  10,      0,
  /* OBJ_SHOVEL             */  15,      0,
  /* OBJ_COAL               */  20,      0,
  /* OBJ_SCARAB             */   8,      0,
  /* OBJ_LARGE_BAG          */   0,      0,
  /* OBJ_STILETTO           */  10,      0,
  /* OBJ_SWORD              */  30,      0,
  /* OBJ_MAP                */   2,      0,
  /* OBJ_BOAT_LABEL         */   2,      0,
  /* OBJ_TORCH              */  20,      0,
  /* OBJ_GUIDE              */   0,      0,
  /* OBJ_TRUNK              */  35,      0,
  /* OBJ_TUBE               */   5,      7,
  /* OBJ_PUTTY              */   6,      0,
  /* OBJ_OWNERS_MANUAL      */   0,      0,
  /* OBJ_WRENCH             */  10,      0,
  /* OBJ_NEST               */   0,     20,
  /* OBJ_EGG                */   0,      6,
  /* OBJ_BROKEN_EGG         */   0,      6,
  /* OBJ_BAUBLE             */   0,      0,
  /* OBJ_CANARY             */   0,      0,
  /* OBJ_BROKEN_CANARY      */   0,      0,
  /* OBJ_ENGRAVINGS         */   0,      0,
  /* OBJ_ZORKMID            */   0,      0,
  /* OBJ_GRUE               */   0,      0
};



int GetObjectSize(int obj)
{
  return pgm_read_word(ObjectSizeCapacity + 2*obj+0);
}

int GetObjectCapacity(int obj)
{
  return pgm_read_word(ObjectSizeCapacity + 2*obj+1);
}

//*****************************************************************************



//*****************************************************************************

const unsigned char RoomStartProp[NUM_ROOMS] PROGMEM =
{
  /* null room                */  0                                              ,
  /* ROOM_WEST_OF_HOUSE       */  R_SACRED | R_LIT                               ,
  /* ROOM_STONE_BARROW        */  R_SACRED | R_LIT                               ,
  /* ROOM_NORTH_OF_HOUSE      */  R_SACRED | R_LIT                               ,
  /* ROOM_SOUTH_OF_HOUSE      */  R_SACRED | R_LIT                               ,
  /* ROOM_EAST_OF_HOUSE       */  R_SACRED | R_LIT                               ,
  /* ROOM_FOREST_1            */  R_SACRED | R_LIT                               ,
  /* ROOM_FOREST_2            */  R_SACRED | R_LIT                               ,
  /* ROOM_MOUNTAINS           */  R_SACRED | R_LIT                               ,
  /* ROOM_FOREST_3            */  R_SACRED | R_LIT                               ,
  /* ROOM_PATH                */  R_SACRED | R_LIT                               ,
  /* ROOM_UP_A_TREE           */  R_SACRED | R_LIT                               ,
  /* ROOM_GRATING_CLEARING    */  R_SACRED | R_LIT                               ,
  /* ROOM_CLEARING            */  R_SACRED | R_LIT                               ,
  /* ROOM_KITCHEN             */  R_SACRED | R_LIT                               ,
  /* ROOM_ATTIC               */  R_SACRED                                       ,
  /* ROOM_LIVING_ROOM         */  R_SACRED | R_LIT                               ,
  /* ROOM_CELLAR              */  0                                              ,
  /* ROOM_TROLL_ROOM          */  0                                              ,
  /* ROOM_EAST_OF_CHASM       */  0                                              ,
  /* ROOM_GALLERY             */  R_LIT                                          ,
  /* ROOM_STUDIO              */  0                                              ,
  /* ROOM_MAZE_1              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_2              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_3              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_4              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_DEAD_END_1          */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_5              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_DEAD_END_2          */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_6              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_7              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_8              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_DEAD_END_3          */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_9              */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_10             */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_11             */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_GRATING_ROOM        */  0                                              ,
  /* ROOM_MAZE_12             */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_DEAD_END_4          */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_13             */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_14             */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_MAZE_15             */  R_ALWAYSDESC | R_MAZE                          ,
  /* ROOM_CYCLOPS_ROOM        */  0                                              ,
  /* ROOM_STRANGE_PASSAGE     */  0                                              ,
  /* ROOM_TREASURE_ROOM       */  0                                              ,
  /* ROOM_RESERVOIR_SOUTH     */  R_WATERHERE                                    ,
  /* ROOM_RESERVOIR           */  R_WATERHERE | R_BODYOFWATER                    ,
  /* ROOM_RESERVOIR_NORTH     */  R_WATERHERE                                    ,
  /* ROOM_STREAM_VIEW         */  R_WATERHERE                                    ,
  /* ROOM_IN_STREAM           */  R_WATERHERE | R_BODYOFWATER                    ,
  /* ROOM_MIRROR_ROOM_1       */  0                                              ,
  /* ROOM_MIRROR_ROOM_2       */  R_LIT                                          ,
  /* ROOM_SMALL_CAVE          */  0                                              ,
  /* ROOM_TINY_CAVE           */  0                                              ,
  /* ROOM_COLD_PASSAGE        */  0                                              ,
  /* ROOM_NARROW_PASSAGE      */  0                                              ,
  /* ROOM_WINDING_PASSAGE     */  0                                              ,
  /* ROOM_TWISTING_PASSAGE    */  0                                              ,
  /* ROOM_ATLANTIS_ROOM       */  0                                              ,
  /* ROOM_EW_PASSAGE          */  0                                              ,
  /* ROOM_ROUND_ROOM          */  0                                              ,
  /* ROOM_DEEP_CANYON         */  0                                              ,
  /* ROOM_DAMP_CAVE           */  0                                              ,
  /* ROOM_LOUD_ROOM           */  0                                              ,
  /* ROOM_NS_PASSAGE          */  0                                              ,
  /* ROOM_CHASM_ROOM          */  0                                              ,
  /* ROOM_ENTRANCE_TO_HADES   */  R_LIT                                          ,
  /* ROOM_LAND_OF_LIVING_DEAD */  R_LIT                                          ,
  /* ROOM_ENGRAVINGS_CAVE     */  0                                              ,
  /* ROOM_EGYPT_ROOM          */  0                                              ,
  /* ROOM_DOME_ROOM           */  0                                              ,
  /* ROOM_TORCH_ROOM          */  0                                              ,
  /* ROOM_NORTH_TEMPLE        */  R_SACRED | R_LIT                               ,
  /* ROOM_SOUTH_TEMPLE        */  R_SACRED | R_LIT                               ,
  /* ROOM_DAM_ROOM            */  R_WATERHERE | R_LIT                            ,
  /* ROOM_DAM_LOBBY           */  R_LIT                                          ,
  /* ROOM_MAINTENANCE_ROOM    */  0                                              ,
  /* ROOM_DAM_BASE            */  R_SACRED | R_WATERHERE | R_LIT                 ,
  /* ROOM_RIVER_1             */  R_SACRED | R_WATERHERE | R_LIT | R_BODYOFWATER ,
  /* ROOM_RIVER_2             */  R_SACRED | R_WATERHERE | R_BODYOFWATER         ,
  /* ROOM_RIVER_3             */  R_SACRED | R_WATERHERE | R_BODYOFWATER         ,
  /* ROOM_WHITE_CLIFFS_NORTH  */  R_SACRED | R_WATERHERE                         ,
  /* ROOM_WHITE_CLIFFS_SOUTH  */  R_SACRED | R_WATERHERE                         ,
  /* ROOM_RIVER_4             */  R_SACRED | R_WATERHERE | R_BODYOFWATER         ,
  /* ROOM_RIVER_5             */  R_SACRED | R_WATERHERE | R_LIT | R_BODYOFWATER ,
  /* ROOM_SHORE               */  R_SACRED | R_WATERHERE | R_LIT                 ,
  /* ROOM_SANDY_BEACH         */  R_SACRED | R_WATERHERE                         ,
  /* ROOM_SANDY_CAVE          */  0                                              ,
  /* ROOM_ARAGAIN_FALLS       */  R_SACRED | R_WATERHERE | R_LIT                 ,
  /* ROOM_ON_RAINBOW          */  R_SACRED | R_LIT                               ,
  /* ROOM_END_OF_RAINBOW      */  R_WATERHERE | R_LIT                            ,
  /* ROOM_CANYON_BOTTOM       */  R_SACRED | R_WATERHERE | R_LIT                 ,
  /* ROOM_CLIFF_MIDDLE        */  R_SACRED | R_LIT                               ,
  /* ROOM_CANYON_VIEW         */  R_SACRED | R_LIT                               ,
  /* ROOM_MINE_ENTRANCE       */  0                                              ,
  /* ROOM_SQUEEKY_ROOM        */  0                                              ,
  /* ROOM_BAT_ROOM            */  R_SACRED                                       ,
  /* ROOM_SHAFT_ROOM          */  0                                              ,
  /* ROOM_SMELLY_ROOM         */  0                                              ,
  /* ROOM_GAS_ROOM            */  R_SACRED                                       ,
  /* ROOM_LADDER_TOP          */  0                                              ,
  /* ROOM_LADDER_BOTTOM       */  0                                              ,
  /* ROOM_DEAD_END_5          */  0                                              ,
  /* ROOM_TIMBER_ROOM         */  R_SACRED                                       ,
  /* ROOM_LOWER_SHAFT         */  R_SACRED                                       ,
  /* ROOM_MACHINE_ROOM        */  0                                              ,
  /* ROOM_MINE_1              */  0                                              ,
  /* ROOM_MINE_2              */  0                                              ,
  /* ROOM_MINE_3              */  0                                              ,
  /* ROOM_MINE_4              */  0                                              ,
  /* ROOM_SLIDE_ROOM          */  0
};



void InitRoomProperties(void)
{
  int i;

  for (i=0; i<NUM_ROOMS; i++)
    Room[i].prop = pgm_read_byte(RoomStartProp + i);
}

//*****************************************************************************



//*****************************************************************************

const uint8_t RoomPassages[10*NUM_ROOMS] PROGMEM =
{
  /* null room                */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

                                  // N  S  E  W  NE  NW  SE  SW  U  D

  /* ROOM_WEST_OF_HOUSE       */  ROOM_NORTH_OF_HOUSE, ROOM_SOUTH_OF_HOUSE, BL1, ROOM_FOREST_1, ROOM_NORTH_OF_HOUSE, BL0, ROOM_SOUTH_OF_HOUSE, BL0, BL0, BL0,
  /* ROOM_STONE_BARROW        */  BL0, BL0, BL0, BL0, ROOM_WEST_OF_HOUSE, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_NORTH_OF_HOUSE      */  ROOM_PATH, BL2, ROOM_EAST_OF_HOUSE, ROOM_WEST_OF_HOUSE, BL0, BL0, ROOM_EAST_OF_HOUSE, ROOM_WEST_OF_HOUSE, BL0, BL0,
  /* ROOM_SOUTH_OF_HOUSE      */  BL2, ROOM_FOREST_3, ROOM_EAST_OF_HOUSE, ROOM_WEST_OF_HOUSE, ROOM_EAST_OF_HOUSE, ROOM_WEST_OF_HOUSE, BL0, BL0, BL0, BL0,
  /* ROOM_EAST_OF_HOUSE       */  ROOM_NORTH_OF_HOUSE, ROOM_SOUTH_OF_HOUSE, ROOM_CLEARING, BL0, BL0, ROOM_NORTH_OF_HOUSE, BL0, ROOM_SOUTH_OF_HOUSE, BL0, BL0,
  /* ROOM_FOREST_1            */  ROOM_GRATING_CLEARING, ROOM_FOREST_3, ROOM_PATH, BL3, BL0, BL0, BL0, BL0, BL4, BL0,
  /* ROOM_FOREST_2            */  BL5, ROOM_CLEARING, ROOM_MOUNTAINS, ROOM_PATH, BL0, BL0, BL0, BL0, BL4, BL0,
  /* ROOM_MOUNTAINS           */  ROOM_FOREST_2, ROOM_FOREST_2, BL6, ROOM_FOREST_2, BL0, BL0, BL0, BL0, BL6, BL0,
  /* ROOM_FOREST_3            */  ROOM_CLEARING, BL7, BL8, ROOM_FOREST_1, BL0, ROOM_SOUTH_OF_HOUSE, BL0, BL0, BL4, BL0,
  /* ROOM_PATH                */  ROOM_GRATING_CLEARING, ROOM_NORTH_OF_HOUSE, ROOM_FOREST_2, ROOM_FOREST_1, BL0, BL0, BL0, BL0, ROOM_UP_A_TREE, BL0,
  /* ROOM_UP_A_TREE           */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL9, ROOM_PATH,
  /* ROOM_GRATING_CLEARING    */  BL5, ROOM_PATH, ROOM_FOREST_2, ROOM_FOREST_1, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_CLEARING            */  ROOM_FOREST_2, ROOM_FOREST_3, ROOM_CANYON_VIEW, ROOM_EAST_OF_HOUSE, BL0, BL0, BL0, BL0, BL4, BL0,
  /* ROOM_KITCHEN             */  BL0, BL0, BL0, ROOM_LIVING_ROOM, BL0, BL0, BL0, BL0, ROOM_ATTIC, BL0,
  /* ROOM_ATTIC               */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_KITCHEN,
  /* ROOM_LIVING_ROOM         */  BL0, BL0, ROOM_KITCHEN, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_CELLAR              */  ROOM_TROLL_ROOM, ROOM_EAST_OF_CHASM, BL0, BLA, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_TROLL_ROOM          */  BL0, ROOM_CELLAR, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_EAST_OF_CHASM       */  ROOM_CELLAR, BL0, ROOM_GALLERY, BL0, BL0, BL0, BL0, BL0, BL0, BLB,
  /* ROOM_GALLERY             */  ROOM_STUDIO, BL0, BL0, ROOM_EAST_OF_CHASM, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_STUDIO              */  BL0, ROOM_GALLERY, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_1              */  ROOM_MAZE_1, ROOM_MAZE_2, ROOM_TROLL_ROOM, ROOM_MAZE_4, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_2              */  BL0, ROOM_MAZE_1, ROOM_MAZE_3, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_3              */  ROOM_MAZE_4, BL0, BL0, ROOM_MAZE_2, BL0, BL0, BL0, BL0, ROOM_MAZE_5, BL0,
  /* ROOM_MAZE_4              */  ROOM_MAZE_1, BL0, ROOM_DEAD_END_1, ROOM_MAZE_3, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_DEAD_END_1          */  BL0, ROOM_MAZE_4, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_5              */  ROOM_MAZE_3, BL0, ROOM_DEAD_END_2, BL0, BL0, BL0, BL0, ROOM_MAZE_6, BL0, BL0,
  /* ROOM_DEAD_END_2          */  BL0, BL0, BL0, ROOM_MAZE_5, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_6              */  BL0, BL0, ROOM_MAZE_7, ROOM_MAZE_6, BL0, BL0, BL0, BL0, ROOM_MAZE_9, ROOM_MAZE_5,
  /* ROOM_MAZE_7              */  BL0, ROOM_MAZE_15, ROOM_MAZE_8, ROOM_MAZE_6, BL0, BL0, BL0, BL0, ROOM_MAZE_14, BL0,
  /* ROOM_MAZE_8              */  BL0, BL0, BL0, ROOM_MAZE_8, ROOM_MAZE_7, BL0, ROOM_DEAD_END_3, BL0, BL0, BL0,
  /* ROOM_DEAD_END_3          */  ROOM_MAZE_8, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_9              */  ROOM_MAZE_6, ROOM_MAZE_13, ROOM_MAZE_10, ROOM_MAZE_12, BL0, ROOM_MAZE_9, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_10             */  BL0, BL0, ROOM_MAZE_9, ROOM_MAZE_13, BL0, BL0, BL0, BL0, ROOM_MAZE_11, BL0,
  /* ROOM_MAZE_11             */  BL0, BL0, BL0, BL0, ROOM_GRATING_ROOM, ROOM_MAZE_13, BL0, ROOM_MAZE_12, BL0, ROOM_MAZE_10,
  /* ROOM_GRATING_ROOM        */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_MAZE_11, BL0, BL0,
  /* ROOM_MAZE_12             */  ROOM_DEAD_END_4, BL0, ROOM_MAZE_13, BL0, BL0, BL0, BL0, ROOM_MAZE_11, ROOM_MAZE_9, BL0,
  /* ROOM_DEAD_END_4          */  BL0, ROOM_MAZE_12, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_13             */  BL0, ROOM_MAZE_10, ROOM_MAZE_9, ROOM_MAZE_11, BL0, BL0, BL0, BL0, BL0, ROOM_MAZE_12,
  /* ROOM_MAZE_14             */  BL0, ROOM_MAZE_7, BL0, ROOM_MAZE_15, ROOM_MAZE_7, ROOM_MAZE_14, BL0, BL0, BL0, BL0,
  /* ROOM_MAZE_15             */  BL0, ROOM_MAZE_7, BL0, ROOM_MAZE_14, BL0, BL0, ROOM_CYCLOPS_ROOM, BL0, BL0, BL0,
  /* ROOM_CYCLOPS_ROOM        */  BL0, BL0, BL0, BL0, BL0, ROOM_MAZE_15, BL0, BL0, BL0, BL0,
  /* ROOM_STRANGE_PASSAGE     */  BL0, BL0, ROOM_LIVING_ROOM, ROOM_CYCLOPS_ROOM, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_TREASURE_ROOM       */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_CYCLOPS_ROOM,
  /* ROOM_RESERVOIR_SOUTH     */  BL0, BL0, ROOM_DAM_ROOM, ROOM_STREAM_VIEW, BL0, BL0, ROOM_DEEP_CANYON, ROOM_CHASM_ROOM, BL0, BL0,
  /* ROOM_RESERVOIR           */  ROOM_RESERVOIR_NORTH, ROOM_RESERVOIR_SOUTH, BL0, ROOM_IN_STREAM, BL0, BL0, BL0, BL0, ROOM_IN_STREAM, BLC,
  /* ROOM_RESERVOIR_NORTH     */  ROOM_ATLANTIS_ROOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_STREAM_VIEW         */  BL0, BL0, ROOM_RESERVOIR_SOUTH, BLD, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_IN_STREAM           */  BL0, ROOM_STREAM_VIEW, ROOM_RESERVOIR, BLE, BL0, BL0, BL0, BL0, BLE, ROOM_RESERVOIR,
  /* ROOM_MIRROR_ROOM_1       */  ROOM_COLD_PASSAGE, BL0, ROOM_SMALL_CAVE, ROOM_TWISTING_PASSAGE, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MIRROR_ROOM_2       */  ROOM_NARROW_PASSAGE, BL0, ROOM_TINY_CAVE, ROOM_WINDING_PASSAGE, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_SMALL_CAVE          */  ROOM_MIRROR_ROOM_1, ROOM_ATLANTIS_ROOM, BL0, ROOM_TWISTING_PASSAGE, BL0, BL0, BL0, BL0, BL0, ROOM_ATLANTIS_ROOM,
  /* ROOM_TINY_CAVE           */  ROOM_MIRROR_ROOM_2, BL0, BL0, ROOM_WINDING_PASSAGE, BL0, BL0, BL0, BL0, BL0, ROOM_ENTRANCE_TO_HADES,
  /* ROOM_COLD_PASSAGE        */  BL0, ROOM_MIRROR_ROOM_1, BL0, ROOM_SLIDE_ROOM, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_NARROW_PASSAGE      */  ROOM_ROUND_ROOM, ROOM_MIRROR_ROOM_2, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_WINDING_PASSAGE     */  ROOM_MIRROR_ROOM_2, BL0, ROOM_TINY_CAVE, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_TWISTING_PASSAGE    */  ROOM_MIRROR_ROOM_1, BL0, ROOM_SMALL_CAVE, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_ATLANTIS_ROOM       */  BL0, ROOM_RESERVOIR_NORTH, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_SMALL_CAVE, BL0,
  /* ROOM_EW_PASSAGE          */  ROOM_CHASM_ROOM, BL0, ROOM_ROUND_ROOM, ROOM_TROLL_ROOM, BL0, BL0, BL0, BL0, BL0, ROOM_CHASM_ROOM,
  /* ROOM_ROUND_ROOM          */  ROOM_NS_PASSAGE, ROOM_NARROW_PASSAGE, ROOM_LOUD_ROOM, ROOM_EW_PASSAGE, BL0, BL0, ROOM_ENGRAVINGS_CAVE, BL0, BL0, BL0,
  /* ROOM_DEEP_CANYON         */  BL0, BL0, ROOM_DAM_ROOM, BL0, BL0, ROOM_RESERVOIR_SOUTH, BL0, ROOM_NS_PASSAGE, BL0, ROOM_LOUD_ROOM,
  /* ROOM_DAMP_CAVE           */  BL0, BLF, ROOM_WHITE_CLIFFS_NORTH, ROOM_LOUD_ROOM, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_LOUD_ROOM           */  BL0, BL0, ROOM_DAMP_CAVE, ROOM_ROUND_ROOM, BL0, BL0, BL0, BL0, ROOM_DEEP_CANYON, BL0,
  /* ROOM_NS_PASSAGE          */  ROOM_CHASM_ROOM, ROOM_ROUND_ROOM, BL0, BL0, ROOM_DEEP_CANYON, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_CHASM_ROOM          */  BL0, ROOM_NS_PASSAGE, BL0, BL0, ROOM_RESERVOIR_SOUTH, BL0, BL0, ROOM_EW_PASSAGE, ROOM_EW_PASSAGE, BLG,
  /* ROOM_ENTRANCE_TO_HADES   */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_TINY_CAVE, BL0,
  /* ROOM_LAND_OF_LIVING_DEAD */  ROOM_ENTRANCE_TO_HADES, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_ENGRAVINGS_CAVE     */  BL0, BL0, ROOM_DOME_ROOM, BL0, BL0, ROOM_ROUND_ROOM, BL0, BL0, BL0, BL0,
  /* ROOM_EGYPT_ROOM          */  BL0, BL0, BL0, ROOM_NORTH_TEMPLE, BL0, BL0, BL0, BL0, ROOM_NORTH_TEMPLE, BL0,
  /* ROOM_DOME_ROOM           */  BL0, BL0, BL0, ROOM_ENGRAVINGS_CAVE, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_TORCH_ROOM          */  BL0, ROOM_NORTH_TEMPLE, BL0, BL0, BL0, BL0, BL0, BL0, BLH, ROOM_NORTH_TEMPLE,
  /* ROOM_NORTH_TEMPLE        */  ROOM_TORCH_ROOM, ROOM_SOUTH_TEMPLE, ROOM_EGYPT_ROOM, BL0, BL0, BL0, BL0, BL0, ROOM_TORCH_ROOM, ROOM_EGYPT_ROOM,
  /* ROOM_SOUTH_TEMPLE        */  ROOM_NORTH_TEMPLE, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_DAM_ROOM            */  ROOM_DAM_LOBBY, ROOM_DEEP_CANYON, ROOM_DAM_BASE, ROOM_RESERVOIR_SOUTH, BL0, BL0, BL0, BL0, BL0, ROOM_DAM_BASE,
  /* ROOM_DAM_LOBBY           */  BL0, ROOM_DAM_ROOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MAINTENANCE_ROOM    */  BL0, ROOM_DAM_LOBBY, BL0, ROOM_DAM_LOBBY, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_DAM_BASE            */  ROOM_DAM_ROOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_DAM_ROOM, BL0,
  /* ROOM_RIVER_1             */  BL0, BL0, BLI, ROOM_DAM_BASE, BL0, BL0, BL0, BL0, BLJ, ROOM_RIVER_2,
  /* ROOM_RIVER_2             */  BL0, BL0, BLI, BLK, BL0, BL0, BL0, BL0, BLJ, ROOM_RIVER_3,
  /* ROOM_RIVER_3             */  BL0, BL0, BL0, ROOM_WHITE_CLIFFS_NORTH, BL0, BL0, BL0, BL0, BLJ, ROOM_RIVER_4,
  /* ROOM_WHITE_CLIFFS_NORTH  */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_WHITE_CLIFFS_SOUTH  */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_RIVER_4             */  BL0, BL0, ROOM_SANDY_BEACH, ROOM_WHITE_CLIFFS_SOUTH, BL0, BL0, BL0, BL0, BLJ, ROOM_RIVER_5,
  /* ROOM_RIVER_5             */  BL0, BL0, ROOM_SHORE, BL0, BL0, BL0, BL0, BL0, BLJ, BL0,
  /* ROOM_SHORE               */  ROOM_SANDY_BEACH, ROOM_ARAGAIN_FALLS, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_SANDY_BEACH         */  BL0, ROOM_SHORE, BL0, BL0, ROOM_SANDY_CAVE, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_SANDY_CAVE          */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_SANDY_BEACH, BL0, BL0,
  /* ROOM_ARAGAIN_FALLS       */  ROOM_SHORE, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BLL,
  /* ROOM_ON_RAINBOW          */  BL0, BL0, ROOM_ARAGAIN_FALLS, ROOM_END_OF_RAINBOW, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_END_OF_RAINBOW      */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_CANYON_BOTTOM, BL0, BL0,
  /* ROOM_CANYON_BOTTOM       */  ROOM_END_OF_RAINBOW, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_CLIFF_MIDDLE, BL0,
  /* ROOM_CLIFF_MIDDLE        */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_CANYON_VIEW, ROOM_CANYON_BOTTOM,
  /* ROOM_CANYON_VIEW         */  BL0, BL7, ROOM_CLIFF_MIDDLE, ROOM_FOREST_3, BL0, ROOM_CLEARING, BL0, BL0, BL0, ROOM_CLIFF_MIDDLE,
  /* ROOM_MINE_ENTRANCE       */  BL0, ROOM_SLIDE_ROOM, BL0, ROOM_SQUEEKY_ROOM, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_SQUEEKY_ROOM        */  ROOM_BAT_ROOM, BL0, ROOM_MINE_ENTRANCE, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_BAT_ROOM            */  BL0, ROOM_SQUEEKY_ROOM, ROOM_SHAFT_ROOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_SHAFT_ROOM          */  ROOM_SMELLY_ROOM, BL0, BL0, ROOM_BAT_ROOM, BL0, BL0, BL0, BL0, BL0, BLM,
  /* ROOM_SMELLY_ROOM         */  BL0, ROOM_SHAFT_ROOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_GAS_ROOM,
  /* ROOM_GAS_ROOM            */  BL0, BL0, ROOM_MINE_1, BL0, BL0, BL0, BL0, BL0, ROOM_SMELLY_ROOM, BL0,
  /* ROOM_LADDER_TOP          */  BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_MINE_4, ROOM_LADDER_BOTTOM,
  /* ROOM_LADDER_BOTTOM       */  BL0, ROOM_DEAD_END_5, BL0, ROOM_TIMBER_ROOM, BL0, BL0, BL0, BL0, ROOM_LADDER_TOP, BL0,
  /* ROOM_DEAD_END_5          */  ROOM_LADDER_BOTTOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_TIMBER_ROOM         */  BL0, BL0, ROOM_LADDER_BOTTOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_LOWER_SHAFT         */  BL0, ROOM_MACHINE_ROOM, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MACHINE_ROOM        */  ROOM_LOWER_SHAFT, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MINE_1              */  ROOM_GAS_ROOM, BL0, ROOM_MINE_1, BL0, ROOM_MINE_2, BL0, BL0, BL0, BL0, BL0,
  /* ROOM_MINE_2              */  ROOM_MINE_2, ROOM_MINE_1, BL0, BL0, BL0, BL0, ROOM_MINE_3, BL0, BL0, BL0,
  /* ROOM_MINE_3              */  BL0, ROOM_MINE_3, ROOM_MINE_2, BL0, BL0, BL0, BL0, ROOM_MINE_4, BL0, BL0,
  /* ROOM_MINE_4              */  ROOM_MINE_3, BL0, BL0, ROOM_MINE_4, BL0, BL0, BL0, BL0, BL0, ROOM_LADDER_TOP,
  /* ROOM_SLIDE_ROOM          */  ROOM_MINE_ENTRANCE, BL0, ROOM_COLD_PASSAGE, BL0, BL0, BL0, BL0, BL0, BL0, ROOM_CELLAR
};



uint8_t GetRoomPassage(int16_t room, int16_t dir)
{
  return pgm_read_byte(RoomPassages + 10*room + dir);
}

//*****************************************************************************



//****************************************************************************

extern const uint16_t NounPhraseToFixedObj[];
extern const uint16_t NounPhraseToObj[];
extern const uint16_t VerbToAction[];



void GetNounPhraseToFixedObj_Words(int i, int *w1, int *w2)
{
  *w1 = pgm_read_word(NounPhraseToFixedObj + 4*i + 0);
  *w2 = pgm_read_word(NounPhraseToFixedObj + 4*i + 1);
}

int GetNounPhraseToFixedObj_Room(int i)
{
  return pgm_read_word(NounPhraseToFixedObj + 4*i + 2);
}

int GetNounPhraseToFixedObj_FObj(int i)
{
  return pgm_read_word(NounPhraseToFixedObj + 4*i + 3);
}



void GetNounPhraseToObj_Words(int i, int *w1, int *w2, int *w3)
{
  *w1 = pgm_read_word(NounPhraseToObj + 4*i + 0);
  *w2 = pgm_read_word(NounPhraseToObj + 4*i + 1);
  *w3 = pgm_read_word(NounPhraseToObj + 4*i + 2);
}

int GetNounPhraseToObj_Obj(int i)
{
  return pgm_read_word(NounPhraseToObj + 4*i + 3);
}



void GetVerbToAction_Words(int i, int *w1, int *w2)
{
  *w1 = pgm_read_word(VerbToAction + 3*i + 0);
  *w2 = pgm_read_word(VerbToAction + 3*i + 1);
}

int GetVerbToAction_Action(int i)
{
  return pgm_read_word(VerbToAction + 3*i + 2);
}

//****************************************************************************



//****************************************************************************

extern const unsigned char WordList[];



uint8_t ReadWordList(uint8_t mode)
{
  static const unsigned char *p = WordList;

  if (mode == 0) {p = WordList; return 0;}
  else if (mode == 1) return pgm_read_byte(p);
  else {p++; return 0;}
}

//****************************************************************************



//*****************************************************************************

// warning: if ConsoleW is 40, for example, autoplay lines must not be longer
// than 40-2 characters (rest will be cut off); see GetString



const char AutoPlayTextA[] PROGMEM =


#define AUTOPLAY_SEED  347460  // seed must sync with autoplay text
  "GO NORTH\n"
  "GO NORTH\n"
  "CLIMB TREE\n"
  "TAKE EGG\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "OPEN WINDOW\n"
  "ENTER HOUSE\n"
  "GO WEST\n"
  "TAKE LAMP AND SWORD\n"
  "MOVE RUG\n"
  "OPEN CASE\n"
  "GO EAST\n"
  "GO UP\n"
  "TURN ON LAMP\n"
  "GRAB KNIFE\n"
  "GO DOWN\n"
  "GO WEST\n"
  "OPEN TRAPDOOR\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "TAKE PAINTING\n"
  "GO WEST\n"
  "GO NORTH\n"
  "GO NORTH\n"
  "KILL TROLL WITH SWORD\n"
  "AGAIN\n"
  "GO WEST\n"
  "GO WEST\n"
  "GO WEST\n"
  "GO UP\n"
  "TAKE COINS\n"
  "GO SOUTHWEST\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO SOUTHEAST\n"
  "ULYSSES\n"
  "CLIMB STAIRS\n"
  "GIVE EGG TO THIEF\n"
  "KILL THIEF WITH NASTY KNIFE\n"
  "AGAIN\n"
  "AGAIN\n"
  "AGAIN\n"
  "TAKE CANARY, CHALICE AND EGG\n"
  "GO DOWN\n"
  "GO EAST\n"
  "GO EAST\n"
  "PUT ALL BUT LAMP AND CANARY INTO CASE\n"
  "OPEN TRAPDOOR\n"
  "GO WEST\n"
  "GO WEST\n"
  "GO UP\n"
  "TEMPLE\n"
  "GO DOWN\n"
  "TAKE COFFIN\n"
  "GO UP\n"
  "GO SOUTH\n"
  "PRAY\n"
  "GO EAST\n"
  "WIND UP CANARY\n"
  "TAKE BAUBLE\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "GO IN\n"
  "GO WEST\n"
  "PUT CANARY AND BAUBLE IN CASE\n"
  "GO DOWN\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO DOWN\n"
  "GO NORTHEAST\n"
  "GO EAST\n"
  "GO NORTH\n"
  "GO NORTH\n"
  "PUSH YELLOW BUTTON\n"
  "TAKE WRENCH AND SCREWDRIVER\n"
  "GO SOUTH\n"
  "GO SOUTH\n"
  "TURN BOLT WITH WRENCH\n"
  "GO DOWN\n"
  "TAKE BOAT\n"
  "GO UP\n"
  "GO WEST\n"
  "DROP BOAT, COFFIN AND WRENCH\n"
  "WAIT\n"
  "WAIT\n"
  "GO NORTH\n"
  "GO NORTH\n"
  "GO NORTH\n"
  "TAKE TRIDENT\n"
  "GO SOUTH\n"
  "TAKE PUMP\n"
  "GO SOUTH\n"
  "TAKE TRUNK\n"
  "GO SOUTH\n"
  "PUMP UP BOAT\n"
  "PUT SCREWDRIVER AND TRUNK IN BOAT\n"
  "TAKE COFFIN\n"
  "OPEN COFFIN\n"
  "PUT TRIDENT IN COFFIN\n"
  "PUT COFFIN IN BOAT\n"
  "DEFLATE BOAT\n"
  "TAKE BOAT\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO DOWN\n"
  "ECHO\n"
  "TAKE BAR\n"
  "GO EAST\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "DROP BOAT\n"
  "INFLATE BOAT\n"
  "GET IN BOAT\n"
  "LAUNCH\n"
  "TAKE BUOY\n"
  "GO EAST\n"
  "GET OUT OF BOAT\n"
  "TAKE SHOVEL\n"
  "GO NORTHEAST\n"
  "DIG SAND WITH SHOVEL\n"
  "AGAIN\n"
  "AGAIN\n"
  "AGAIN\n"
  "DROP BUOY AND SHOVEL\n"
  "OPEN BUOY\n"
  "TAKE SCARAB AND EMERALD\n"
  "GO SOUTHWEST\n"
  "TAKE SCEPTRE AND BOAT\n"
  "GO SOUTH\n"
  "GO SOUTH\n"
  "WAVE SCEPTRE\n"
  "GO WEST\n"
  "GO WEST\n"
  "TAKE POT OF GOLD\n"
  "GO SOUTHWEST\n"
  "GO UP\n"
  "GO UP\n"
  "GO NORTHWEST\n"
  "GO WEST\n"
  "GO IN\n"
  "OPEN SACK\n"
  "TAKE CLOVE OUT OF SACK\n"
  "GO WEST\n"
  "DROP BOAT\n"
  "PUT ALL BUT LAMP AND CLOVE IN CASE\n"
  "TAKE TRIDENT, COFFIN AND SCREWDRIVER\n"
  "PUT TRIDENT AND COFFIN IN CASE\n"
  "GO WEST\n"
  "GO WEST\n"
  "GO UP\n"
  "TEMPLE\n"
  "TURN OFF LAMP\n"
  "GO NORTH\n"
  "TAKE TORCH\n"
  "GO SOUTH\n"
  "TAKE BELL\n"
  "GO SOUTH\n"
  "GET BOOK AND CANDLES\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "DROP CANDLES\n"
  "RING BELL\n"
  "TAKE CANDLES\n"
  "READ BOOK\n"
  "GO SOUTH\n"
  "TAKE SKULL\n"
  "GO NORTH\n"
  "GO UP\n"
  "GO NORTH\n"
  "TOUCH THE MIRROR\n"
  "GO NORTH\n"
  "GO WEST\n"
  "GO NORTH\n"
  "GO WEST\n"
  "GO NORTH\n"
  "GO EAST\n"
  "PUT TORCH,CLOVE,SCREWDRIVER IN BASKET\n"
  "TURN ON LAMP\n"
  "GO WEST\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "TAKE COAL\n"
  "GO NORTH\n"
  "GO UP\n"
  "GO UP\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO NORTH\n"
  "GO UP\n"
  "GO SOUTH\n"
  "PUT COAL IN BASKET\n"
  "LOWER BASKET\n"
  "GO WEST\n"
  "GO NORTHEAST\n"
  "GO SOUTHEAST\n"
  "GO SOUTHWEST\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "GO WEST\n"
  "DROP ALL\n"
  "GO WEST\n"
  "TAKE SCREWDRIVER, COAL AND TORCH\n"
  "GO SOUTH\n"
  "OPEN LID\n"
  "PUT COAL IN MACHINE\n"
  "CLOSE LID\n"
  "FLIP SWITCH WITH SCREWDRIVER\n"
  "OPEN LID\n"
  "TAKE DIAMOND\n"
  "GO NORTH\n"
  "PUT ALL IN BASKET\n"
  "GO EAST\n"
  "TAKE SKULL AND LAMP\n"
  "GO EAST\n"
  "GO UP\n"
  "GO UP\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO NORTH\n"
  "TAKE BRACELET\n"
  "GO UP\n"
  "GO SOUTH\n"
  "RAISE BASKET\n"
  "TAKE DIAMOND, TORCH AND CLOVE\n"
  "GO WEST\n"
  "TAKE JADE\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO DOWN\n"
  "GO UP\n"
  "TAKE TRUNK\n"
  "PUT ALL IN CASE\n"
  "READ MAP\n"
  "GO EAST\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO WEST\n"
  "GO SOUTHWEST\n"
  "ENTER BARROW\n"


/*
#define AUTOPLAY_SEED  347460  // seed must sync with autoplay text
  "N.N.U\n"
  "TAKE EGG\n"
  "D.S.E\n"
  "OPEN WINDOW\n"
  "IN.W\n"
  "TAKE LAMP AND SWORD\n"
  "MOVE RUG\n"
  "OPEN CASE\n"
  "E.U\n"
  "TURN ON LAMP\n"
  "TAKE KNIFE\n"
  "D.W\n"
  "OPEN TRAPDOOR\n"
  "D.S.E\n"
  "TAKE PAINTING\n"
  "W.N.N\n"
  "KILL TROLL WITH SWORD.G\n"
  "W.W.W.U\n"
  "TAKE COINS\n"
  "SW.E.S.SE\n"
  "ULYSSES\n"
  "U\n"
  "GIVE EGG TO THIEF\n"
  "KILL THIEF WITH NASTY KNIFE.G.G.G\n"
  "TAKE CANARY, CHALICE AND EGG\n"
  "D.E.E\n"
  "PUT ALL BUT LAMP AND CANARY INTO CASE\n"
  "OPEN TRAPDOOR\n"
  "W.W.U\n"
  "TEMPLE\n"
  "D\n"
  "TAKE COFFIN\n"
  "U.S\n"
  "PRAY\n"
  "E\n"
  "WIND UP CANARY\n"
  "TAKE BAUBLE\n"
  "S.E.IN.W\n"
  "PUT CANARY AND BAUBLE IN CASE\n"
  "D.N.E.D.NE.E.N.N\n"
  "PUSH YELLOW BUTTON\n"
  "TAKE WRENCH AND SCREWDRIVER\n"
  "S.S\n"
  "TURN BOLT WITH WRENCH\n"
  "D\n"
  "TAKE BOAT\n"
  "U.W\n"
  "DROP BOAT, COFFIN AND WRENCH\n"
  "Z.Z\n"
  "N.N.N\n"
  "TAKE TRIDENT\n"
  "S\n"
  "TAKE PUMP\n"
  "S\n"
  "TAKE TRUNK\n"
  "S\n"
  "PUMP BOAT\n"
  "PUT SCREWDRIVER AND TRUNK IN BOAT\n"
  "TAKE COFFIN\n"
  "OPEN COFFIN\n"
  "PUT TRIDENT IN COFFIN\n"
  "PUT COFFIN IN BOAT\n"
  "DEFLATE BOAT\n"
  "TAKE BOAT\n"
  "E.S.D\n"
  "ECHO\n"
  "TAKE BAR\n"
  "E.E.S\n"
  "DROP BOAT\n"
  "PUMP UP BOAT\n"
  "GET IN BOAT\n"
  "LAUNCH\n"
  "TAKE BUOY\n"
  "E\n"
  "GET OUT OF BOAT\n"
  "TAKE SHOVEL\n"
  "NE\n"
  "DIG SAND WITH SHOVEL.G.G.G\n"
  "DROP BUOY AND SHOVEL\n"
  "OPEN BUOY\n"
  "TAKE SCARAB AND EMERALD\n"
  "SW\n"
  "TAKE SCEPTRE AND BOAT\n"
  "S.S\n"
  "WAVE SCEPTRE\n"
  "W.W\n"
  "TAKE POT\n"
  "SW.U.U.NW.W.IN\n"
  "OPEN SACK\n"
  "TAKE CLOVE\n"
  "W\n"
  "DROP BOAT\n"
  "PUT ALL BUT LAMP AND CLOVE IN CASE\n"
  "TAKE TRIDENT, COFFIN AND SCREWDRIVER\n"
  "PUT TRIDENT AND COFFIN IN CASE\n"
  "W.W.U\n"
  "TEMPLE\n"
  "TURN OFF LAMP\n"
  "N\n"
  "TAKE TORCH\n"
  "S\n"
  "TAKE BELL\n"
  "S\n"
  "TAKE BOOK AND CANDLES\n"
  "D.D\n"
  "DROP CANDLES\n"
  "RING BELL\n"
  "TAKE CANDLES\n"
  "READ BOOK\n"
  "S\n"
  "TAKE SKULL\n"
  "N.U.N\n"
  "TOUCH MIRROR\n"
  "N.W.N.W.N.E\n"
  "PUT TORCH,CLOVE,SCREWDRIVER IN BASKET\n"
  "TURN ON LAMP\n"
  "W.D.D.S\n"
  "TAKE COAL\n"
  "N.U.U.N.E.S.N.U.S\n"
  "PUT COAL IN BASKET\n"
  "LOWER BASKET\n"
  "W.NE.SE.SW.D.D.W\n"
  "DROP ALL\n"
  "W\n"
  "TAKE SCREWDRIVER, COAL AND TORCH\n"
  "S\n"
  "OPEN LID\n"
  "PUT COAL IN MACHINE\n"
  "CLOSE LID\n"
  "FLIP SWITCH WITH SCREWDRIVER\n"
  "OPEN LID\n"
  "TAKE DIAMOND\n"
  "N\n"
  "PUT ALL IN BASKET\n"
  "E\n"
  "TAKE SKULL AND LAMP\n"
  "E.U.U.N.E.S.N\n"
  "TAKE BRACELET\n"
  "U.S\n"
  "RAISE BASKET\n"
  "TAKE DIAMOND, TORCH AND CLOVE\n"
  "W\n"
  "TAKE JADE\n"
  "S.E.S.D.U\n"
  "TAKE TRUNK\n"
  "PUT ALL IN CASE\n"
  "READ MAP\n"
  "E.E.S.W.SW.IN\n"
*/


/*
#define AUTOPLAY_SEED  58744  // seed must sync with autoplay text
  "S.E\n"
  "OPEN WINDOW\n"
  "W.W\n"
  "GET LAMP\n"
  "MOVE RUG\n"
  "OPEN TRAP\n"
  "TURN ON LAMP\n"
  "D.S.E\n"
  "GET PAINTING\n"
  "N.U.U\n"
  "GET KNIFE,ROPE\n"
  "D.W\n"
  "OPEN CASE\n"
  "PUT PAINTING,KNIFE IN CASE\n"
  "GET SWORD\n"
  "OPEN TRAP\n"
  "D.N\n"
  "KILL TROLL\n"
  "DROP SWORD\n"
  "E.E.SE.E\n"
  "TIE ROPE TO RAIL\n"
  "D\n"
  "TURN OFF LAMP\n"
  "GET TORCH\n"
  "S.E\n"
  "OPEN COFFIN\n"
  "GET SCEPTRE\n"
  "PUT LAMP IN COFFIN\n"
  "GET COFFIN\n"
  "W.S\n"
  "PRAY\n"
  "S.N.E.D.D.N\n"
  "WAVE SCEPTRE\n"
  "GET POT\n"
  "SW.U.U.NW.W.IN\n"
  "OPEN BAG\n"
  "GET GARLIC\n"
  "W\n"
  "PUT POT,SCEPTRE IN CASE\n"
  "GET LAMP\n"
  "PUT COFFIN IN CASE\n"
  "OPEN TRAP\n"
  "D.N.E.N.NE.E.N\n"
  "GET MATCHES\n"
  "N\n"
  "GET WRENCH,SCREWDRIVER\n"
  "PUSH YELLOW BUTTON\n"
  "S.S\n"
  "TURN BOLT\n"
  "DROP WRENCH\n"
  "S\n"
  "Z.Z.Z.Z\n"
  "D\n"
  "ECHO\n"
  "GET BAR\n"
  "W.SE.E.D.S\n"
  "GET BELL\n"
  "S\n"
  "GET CANDLES,BOOK\n"
  "GET BOOK\n"
  "D.D\n"
  "RING BELL\n"
  "GET CANDLES\n"
  "LIGHT MATCH\n"
  "LIGHT CANDLES\n"
  "READ BOOK\n"
  "DROP BOOK,CANDLES\n"
  "S\n"
  "GET SKULL\n"
  "N.U.N\n"
  "TOUCH MIRROR\n"
  "N.W.N.W.N.E\n"
  "PUT TORCH,SCREWDRIVER IN BASKET\n"
  "TURN ON LAMP\n"
  "N.D.E.NE.SE.SW.D.D.S\n"
  "GET COAL\n"
  "N.U.U.N.E.S.N.U.S\n"
  "PUT COAL IN BASKET\n"
  "LOWER BASKET\n"
  "N.D.E.NE.SE.SW.D.D.W\n"
  "DROP ALL\n"
  "W\n"
  "GET COAL,SCREWDRIVER,TORCH\n"
  "S\n"
  "OPEN LID\n"
  "PUT COAL IN MACHINE\n"
  "CLOSE LID\n"
  "TURN SWITCH WITH SCREWDRIVER\n"
  "DROP SCREWDRIVER\n"
  "OPEN LID\n"
  "GET DIAMOND\n"
  "N\n"
  "PUT TORCH,DIAMOND IN BASKET\n"
  "E\n"
  "GET SKULL,BAR,LAMP,GARLIC\n"
  "E.U.U.N.E.S.N\n"
  "GET BRACELET\n"
  "U.S\n"
  "RAISE BASKET\n"
  "TURN OFF LAMP\n"
  "GET DIAMOND,TORCH\n"
  "W\n"
  "GET FIGURINE\n"
  "S.E.S.D.U\n"
  "PUT ALL BUT TORCH IN CASE\n"
  "D.N.E.N.NE.N\n"
  "GET TRUNK\n"
  "N\n"
  "GET PUMP\n"
  "N\n"
  "GET TRIDENT\n"
  "S.S.S.E.E\n"
  "INFLATE PLASTIC\n"
  "DROP PUMP\n"
  "ENTER BOAT\n"
  "LAUNCH\n"
  "Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z\n"
  "OPEN BUOY\n"
  "GET EMERALD\n"
  "E\n"
  "LEAVE BOAT\n"
  "N\n"
  "GET SHOVEL\n"
  "NE\n"
  "DIG SAND\n"
  "G\n"
  "G\n"
  "G\n"
  "DROP SHOVEL\n"
  "GET SCARAB\n"
  "SW.S.S.W.W.SW.U.U.NW.W.W.W\n"
  "PUT ALL BUT TORCH IN CASE\n"
  "GET KNIFE\n"
  "E.E.N.N.U\n"
  "GET EGG\n"
  "D.S.E.W.W.D.N.W.S.E.U\n"
  "GET COINS,KEY\n"
  "SW.E.S.SE\n"
  "ODYSSEUS\n"
  "U\n"
  "GIVE EGG TO THIEF\n"
  "KILL THIEF\n"
  "TAKE KNIFE\n"
  "KILL THIEF\n"
  "DROP KNIFE\n"
  "GET ALL BUT STILETTO,KNIFE\n"
  "GET CANARY\n"
  "D.E.E\n"
  "PUT EGG,COINS,CHALICE IN CASE\n"
  "E.E.N.N\n"
  "WIND CANARY\n"
  "GET BAUBLE\n"
  "S.SE.W.W\n"
  "PUT BAUBLE,CANARY,TORCH IN CASE\n"
  "READ MAP\n"
  "E.E.NW.SW.SW\n"
  "ENTER BARROW\n"
*/


/*
#define AUTOPLAY_SEED  58744  // seed must sync with autoplay text
  "GO SOUTH\n"
  "GO EAST\n"
  "OPEN WINDOW\n"
  "CLIMB THROUGH WINDOW\n"
  "GO WEST\n"
  "TAKE LAMP\n"
  "MOVE RUG\n"
  "OPEN TRAP DOOR\n"
  "TURN ON LAMP\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "TAKE PAINTING\n"
  "GO NORTH\n"
  "CLIMB UP CHIMNEY\n"
  "CLIMB STAIRS\n"
  "TAKE KNIFE AND ROPE\n"
  "GO DOWN\n"
  "GO WEST\n"
  "OPEN CASE\n"
  "PUT PAINTING AND KNIFE INSIDE CASE\n"
  "TAKE SWORD\n"
  "OPEN TRAP DOOR\n"
  "ENTER TRAP DOOR\n"
  "GO NORTH\n"
  "KILL TROLL\n"
  "DROP SWORD\n"
  "GO EAST\n"
  "GO EAST\n"
  "GO SOUTHEAST\n"
  "GO EAST\n"
  "TIE ROPE TO RAILING\n"
  "CLIMB DOWN ROPE\n"
  "TURN OFF LAMP\n"
  "TAKE TORCH\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "OPEN COFFIN\n"
  "TAKE SCEPTRE OUT OF COFFIN\n"
  "PUT LAMP IN COFFIN\n"
  "PICK UP COFFIN\n"
  "GO WEST\n"
  "GO SOUTH\n"
  "PRAY\n"
  "GO SOUTH\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "GO NORTH\n"
  "WAVE SCEPTRE\n"
  "TAKE POT OF GOLD\n"
  "GO SOUTHWEST\n"
  "GO UP\n"
  "GO UP\n"
  "GO NORTHWEST\n"
  "GO WEST\n"
  "GO IN\n"
  "OPEN BAG\n"
  "TAKE GARLIC OUT OF BAG\n"
  "GO WEST\n"
  "PUT POT OF GOLD AND SCEPTRE IN CASE\n"
  "TAKE LAMP OUT OF COFFIN\n"
  "PUT COFFIN IN CASE\n"
  "OPEN TRAP DOOR\n"
  "GO DOWN STAIRS\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO NORTH\n"
  "GO NORTHEAST\n"
  "GO EAST\n"
  "GO NORTH\n"
  "TAKE MATCHES\n"
  "GO NORTH\n"
  "TAKE WRENCH AND SCREWDRIVER\n"
  "PUSH YELLOW BUTTON\n"
  "GO SOUTH\n"
  "GO SOUTH\n"
  "TURN BOLT WITH WRENCH\n"
  "DROP WRENCH\n"
  "GO SOUTH\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "GO DOWN\n"
  "ECHO\n"
  "TAKE BAR\n"
  "GO WEST\n"
  "GO SOUTHEAST\n"
  "GO EAST\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "TAKE BELL\n"
  "GO SOUTH\n"
  "TAKE CANDLES AND BOOK\n"
  "TAKE BOOK\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "RING BELL\n"
  "TAKE CANDLES\n"
  "LIGHT A MATCH\n"
  "LIGHT CANDLES WITH MATCH\n"
  "READ BOOK\n"
  "DROP BOOK AND CANDLES\n"
  "GO SOUTH\n"
  "TAKE SKULL\n"
  "GO NORTH\n"
  "GO UP\n"
  "GO NORTH\n"
  "TOUCH THE MIRROR\n"
  "GO NORTH\n"
  "GO WEST\n"
  "GO NORTH\n"
  "GO WEST\n"
  "GO NORTH\n"
  "GO EAST\n"
  "PUT TORCH AND SCREWDRIVER IN BASKET\n"
  "TURN ON LAMP\n"
  "GO NORTH\n"
  "GO DOWN\n"
  "GO EAST\n"
  "GO NORTHEAST\n"
  "GO SOUTHEAST\n"
  "GO SOUTHWEST\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "TAKE COAL\n"
  "GO NORTH\n"
  "GO UP\n"
  "GO UP\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO NORTH\n"
  "GO UP\n"
  "GO SOUTH\n"
  "PUT COAL IN BASKET\n"
  "LOWER BASKET\n"
  "GO NORTH\n"
  "GO DOWN\n"
  "GO EAST\n"
  "GO NORTHEAST\n"
  "GO SOUTHEAST\n"
  "GO SOUTHWEST\n"
  "GO DOWN\n"
  "GO DOWN\n"
  "GO WEST\n"
  "DROP ALL\n"
  "GO WEST\n"
  "TAKE COAL, SCREWDRIVER, AND TORCH\n"
  "GO SOUTH\n"
  "OPEN LID\n"
  "PUT COAL IN MACHINE\n"
  "CLOSE LID\n"
  "TURN SWITCH WITH SCREWDRIVER\n"
  "DROP SCREWDRIVER\n"
  "OPEN LID\n"
  "TAKE DIAMOND\n"
  "GO NORTH\n"
  "PUT TORCH AND DIAMOND IN BASKET\n"
  "GO EAST\n"
  "TAKE SKULL, BAR, LAMP, AND GARLIC\n"
  "GO EAST\n"
  "GO UP\n"
  "GO UP\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO NORTH\n"
  "TAKE BRACELET\n"
  "GO UP\n"
  "GO SOUTH\n"
  "RAISE BASKET\n"
  "TURN OFF LAMP\n"
  "TAKE DIAMOND AND TORCH\n"
  "GO WEST\n"
  "TAKE FIGURINE\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO DOWN\n"
  "GO UP\n"
  "PLACE ALL EXCEPT TORCH IN CASE\n"
  "GO DOWN\n"
  "GO NORTH\n"
  "GO EAST\n"
  "GO NORTH\n"
  "GO NORTHEAST\n"
  "GO NORTH\n"
  "TAKE TRUNK\n"
  "GO NORTH\n"
  "TAKE PUMP\n"
  "GO NORTH\n"
  "TAKE TRIDENT\n"
  "GO SOUTH\n"
  "GO SOUTH\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "GO EAST\n"
  "INFLATE PLASTIC WITH PUMP\n"
  "DROP PUMP\n"
  "ENTER BOAT\n"
  "LAUNCH\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "WAIT\n"
  "OPEN BUOY\n"
  "TAKE EMERALD OUT OF BUOY\n"
  "GO EAST\n"
  "LEAVE BOAT\n"
  "GO NORTH\n"
  "TAKE SHOVEL\n"
  "GO NORTHEAST\n"
  "DIG IN SAND\n"
  "DIG IN SAND\n"
  "DIG IN SAND\n"
  "DIG IN SAND\n"
  "DROP SHOVEL\n"
  "TAKE SCARAB\n"
  "GO SOUTHWEST\n"
  "GO SOUTH\n"
  "GO SOUTH\n"
  "GO WEST\n"
  "GO WEST\n"
  "GO SOUTHWEST\n"
  "CLIMB UP CLIFF\n"
  "CLIMB UP CLIFF\n"
  "GO NORTHWEST\n"
  "GO WEST\n"
  "ENTER HOUSE\n"
  "GO WEST\n"
  "DROP ALL BUT TORCH INTO CASE\n"
  "PICK UP THE KNIFE\n"
  "GO EAST\n"
  "CLIMB OUT OF WINDOW\n"
  "GO NORTH\n"
  "GO NORTH\n"
  "GO UP\n"
  "TAKE EGG\n"
  "GO DOWN\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "GO WEST\n"
  "GO WEST\n"
  "GO DOWN\n"
  "GO NORTH\n"
  "GO WEST\n"
  "GO SOUTH\n"
  "GO EAST\n"
  "GO UP\n"
  "TAKE COINS AND KEY\n"
  "GO SOUTHWEST\n"
  "GO EAST\n"
  "GO SOUTH\n"
  "GO SOUTHEAST\n"
  "ODYSSEUS\n"
  "CLIMB STAIRS\n"
  "GIVE EGG TO THIEF\n"
  "KILL THIEF\n"
  "TAKE KNIFE\n"
  "KILL THIEF\n"
  "DROP KNIFE\n"
  "TAKE ALL EXCEPT STILETTO AND KNIFE\n"
  "TAKE CANARY OUT OF EGG\n"
  "GO DOWN\n"
  "GO EAST\n"
  "GO EAST\n"
  "PUT EGG, COINS AND CHALICE IN CASE\n"
  "GO EAST\n"
  "GO EAST\n"
  "GO NORTH\n"
  "GO NORTH\n"
  "WIND CANARY\n"
  "TAKE BAUBLE\n"
  "GO SOUTH\n"
  "GO SOUTHEAST\n"
  "GO WEST\n"
  "GO WEST\n"
  "PUT BAUBLE, CANARY AND TORCH IN CASE\n"
  "READ MAP\n"
  "GO EAST\n"
  "GO THROUGH WINDOW\n"
  "GO NORTHWEST\n"
  "GO SOUTHWEST\n"
  "GO SOUTHWEST\n"
  "ENTER BARROW\n"
*/


  "\x00"  // not necessary because this is a string literal
;



const char * const AutoPlayText[1] PROGMEM = {AutoPlayTextA};



void ActivateAutoPlay(void)
{
  AutoPlayMode = 1;
  AutoPlayP = AutoPlayText[0];

  SetRandomSeed(AUTOPLAY_SEED);
}



void DeactivateAutoPlay(void)
{
  AutoPlayMode = 0;

  SetRandomSeed(0); // set seed to clock
}

//*****************************************************************************



//****************************************************************************

#ifdef __AVR_ATmega2560__
uint16_t EEPROM_Ptr;
#else
FILE *ReadWriteFile;
#endif



// mode: 0 read  1 write; 
// returns 1 on error
int ReadWrite(int mode, void *p, size_t size)
{
#ifdef __AVR_ATmega2560__
  size_t i;
#endif

  switch (mode)
  {
    case 0:
#ifdef __AVR_ATmega2560__
      for (i=0; i<size; i++)
        *((uint8_t*)p+i) = EEPROM.read(EEPROM_Ptr++);
      return 0;
#else
      return (fread(p, size, 1, ReadWriteFile) != 1);
#endif

    case 1:
#ifdef __AVR_ATmega2560__
      for (i=0; i<size; i++)
        EEPROM.write(EEPROM_Ptr++, *((uint8_t*)p+i));
      return 0;
#else
      return (fwrite(p, size, 1, ReadWriteFile) != 1);
#endif

    default:
#ifdef __AVR_ATmega2560__
      EEPROM_Ptr += size;
#endif
      return 0;
  }
}



// mode: 0 read  1 write
// returns 1 on error
int ReadWriteSaveState(int mode)
{
  int error = 1, i;
  char signature[4] = {'Z', 'O', 'R', 'K'};

  if (ReadWrite(mode, signature, sizeof(signature))) goto done;

  if (ReadWrite(mode, &GameFlags, sizeof(GameFlags))) goto done;
  if (ReadWrite(mode, &GameVars , sizeof(GameVars ))) goto done;

  for (i=0; i<NUM_ROOMS; i++)
    if (ReadWrite(mode, &Room[i].prop, sizeof(unsigned char))) goto done;

  for (i=0; i<NUM_OBJECTS; i++)
  {
    if (ReadWrite(mode, &Obj[i].loc       , sizeof(unsigned short))) goto done;
    if (ReadWrite(mode, &Obj[i].order     , sizeof(unsigned short))) goto done;
    if (ReadWrite(mode, &Obj[i].prop      , sizeof(unsigned short))) goto done;
    if (ReadWrite(mode, &Obj[i].thiefvalue, sizeof(unsigned char ))) goto done;
  }

  error = 0;
done:
  return error;
}



// mode: 0 read  1 write
// returns 1 on error
int ReadWriteSaveSlot(char *filename, int mode, int slot)
{
  int error = 1;

#ifdef __AVR_ATmega2560__
  EEPROM_Ptr = 0;
  ReadWriteSaveState(2); // count bytes
  EEPROM_Ptr *= slot;
  error = ReadWriteSaveState(mode);
#else
  (void)slot; // suppress unused parameter warning
  ReadWriteFile = fopen(filename, mode ? "wb" : "rb");
  if (ReadWriteFile)
  {
    error = ReadWriteSaveState(mode);
    fclose(ReadWriteFile);
  }
#endif

  return error;
}



#ifdef __AVR_ATmega2560__

int CheckSaveSignature(int slot)
{
  char signature[4];

  EEPROM_Ptr = 0;
  ReadWriteSaveState(2); // count bytes
  EEPROM_Ptr *= slot;

  if (ReadWrite(0, signature, sizeof(signature)) == 0)
    if (signature[0] == 'Z' &&
        signature[1] == 'O' &&
        signature[2] == 'R' &&
        signature[3] == 'K')
      return 1;

  return 0;
}

#endif

//****************************************************************************



//****************************************************************************

uint32_t RandomSeed;



void SetRandomSeed(uint32_t seed)
{
  if (seed == 0)
#ifdef __AVR_ATmega2560__
    seed = analogRead(0);
#else
    seed = time(NULL);
#endif

  RandomSeed = seed;
}



// Linear congruential generator; compatible with Borland Delphi, Virtual Pascal
uint32_t GetRandom(uint32_t range)
{
  RandomSeed = RandomSeed * 134775813 + 1;

  return (uint32_t)(((uint64_t)RandomSeed * range) >> 32);
}

//****************************************************************************
