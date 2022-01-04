// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"



struct LABEL_STRUCT {const char *label; const char *text;};

extern struct LABEL_STRUCT RoomDesc[NUM_ROOMS];
extern struct LABEL_STRUCT BlockMsg[NUM_BLOCK_MESSAGES];
extern struct LABEL_STRUCT ObjectDesc[NUM_OBJECTS];



//*****************************************************************************

typedef struct NODE_TYPE
{
  struct NODE_TYPE *left, *right;
  int freq;
  char c;
} *NODE;


 
struct NODE_TYPE NodePool[256];
NODE NodeQueueArray[255];
NODE *NodeQueue = NodeQueueArray - 1; // node indexes start at 1
int NumNodes, EndNodeIndex = 1;

int CharFrequency[128];
char HuffmanCode[128][32];



NODE NewNode(int freq, char c, NODE a, NODE b)
{
  NODE n = NodePool + NumNodes++;

  if (freq) n->c = c, n->freq = freq;
  else
  {
    n->left = a, n->right = b;
    n->freq = a->freq + b->freq;
  }
  return n;
}
 


// priority queue
void InsertNode(NODE n)
{
  int j, i = EndNodeIndex++;

  while ((j = i / 2))
  {
    if (NodeQueue[j]->freq <= n->freq) break;
    NodeQueue[i] = NodeQueue[j], i = j;
  }
  NodeQueue[i] = n;
}
 


NODE RemoveNode(void)
{
  int i, l;
  NODE n = NodeQueue[i = 1];
 
  if (EndNodeIndex < 2) return 0;
  EndNodeIndex--;
  while ((l = i * 2) < EndNodeIndex)
  {
    if (l + 1 < EndNodeIndex && NodeQueue[l + 1]->freq < NodeQueue[l]->freq) l++;
    NodeQueue[i] = NodeQueue[l], i = l;
  }
  NodeQueue[i] = NodeQueue[EndNodeIndex];
  return n;
}
 


// walk down tree
void BuildCode(NODE n, char *s, int len)
{
  if (n->c)
  {
    s[len] = 0;
    strcpy(HuffmanCode[n->c], s);
    return;
  }
 
  s[len] = '0'; BuildCode(n->left,  s, len + 1); // go left
  s[len] = '1'; BuildCode(n->right, s, len + 1); // go right
}
 


void BuildHuffmanTree(void)
{
  int i;
  char c[16];
 
  for (i=0; i<128; i++)
    if (CharFrequency[i]) InsertNode(NewNode(CharFrequency[i], i, 0, 0));
 
  while (EndNodeIndex > 2) 
    InsertNode(NewNode(0, 0, RemoveNode(), RemoveNode()));

  BuildCode(NodeQueue[1], c, 0);
}

//*****************************************************************************



//*****************************************************************************

int Count, bit, byte, bytes_out, bytes_in;
char PrintStringMode;

char String[2048][2048];

unsigned short StartP[2048];
unsigned char StartB[2048];



#define SUBST \
  if (size == 1) {c = 1; size--;} \
  else if (size >= 4 && p[0] == 't' && p[1] == 'h' && p[2] == 'e' && p[3] == ' ') {c =  2; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'T' && p[1] == 'h' && p[2] == 'e' && p[3] == ' ') {c =  3; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'i' && p[1] == 'n' && p[2] == 'g' && p[3] == ' ') {c =  4; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'y' && p[1] == 'o' && p[2] == 'u' && p[3] == ' ') {c =  5; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'Y' && p[1] == 'o' && p[2] == 'u' && p[3] == ' ') {c =  6; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'a' && p[1] == 'n' && p[2] == 'd' && p[3] == ' ') {c =  7; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == ' ' && p[1] == 'i' && p[2] == 's' && p[3] == ' ') {c =  8; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == ' ' && p[1] == 't' && p[2] == 'o' && p[3] == ' ') {c =  9; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == ' ' && p[1] == 'o' && p[2] == 'f' && p[3] == ' ') {c = 11; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'h' && p[1] == 'e' && p[2] == 'r' && p[3] == 'e') {c = 12; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'y' && p[1] == 'o' && p[2] == 'u' && p[3] == 'r') {c = 13; p += 4; size -= 4;} \
  else if (size >= 4 && p[0] == 'w' && p[1] == 'i' && p[2] == 't' && p[3] == 'h') {c = 14; p += 4; size -= 4;} \
  else {c = *p++; size--;}



void PrintString(FILE *out2, FILE *out1, const char *p, int size)
{
  int j;

  for (j=0; j<Count; j++)
    if (memcmp(String[j], p, size) == 0)
  {
    fprintf(out1, "%i", j);
    return;
  }
  memcpy(String[Count], p, size);
  Count++;


  if (PrintStringMode)
  {
    fprintf(out1, "%i", Count-1);

    StartP[Count-1] = bytes_out;
    StartB[Count-1] = bit;

    bytes_in += size;

    fputs("\n  ", out2);
    while (size)
    {
      int i, len;
      unsigned char c;

      SUBST

      len = strlen(HuffmanCode[c]);
      for (i=0; i<len; i++)
      {
        if (HuffmanCode[c][i] == '1') byte |= 1<<bit;
        if (bit == 7)
        {
          fprintf(out2, "%i,", byte);
          bytes_out++;
          bit = 0;
          byte = 0;
        }
        else bit++;
      }
    }
  }
  else
    while (size)
  {
    unsigned char c;

    SUBST

    CharFrequency[(int)c]++;

    if (c == 1) c = '\n';
    else if (c == 10) c = '|';
    else if (c < 32) c = '_';
    fputc(c, out2);
  }
}



void ProcessText(const char *filename_in, const char *filename_out1, FILE *out2)
{
  FILE *f, *out1;
  int size;
  char *buffer, *p, *string, *s;

  const char *match1 = "PrintLine(\"", *match1c = "PrintLineIndex(";
  int match1_len = 11;

  const char *match2 = "PrintText(\"", *match2c = "PrintTextIndex(";
  int match2_len = 11;

  f = fopen(filename_in, "rb");
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fclose(f);

  buffer = (char *)malloc(size);
  string = (char *)malloc(size);

  f = fopen(filename_in, "rb");
  fread(buffer, 1, size, f);
  fclose(f);

  out1 = fopen(filename_out1, "wb");

  p = buffer;
  while (p < buffer+size)
  {
    if ((p-buffer <= size-match1_len && memcmp(p, match1, match1_len) == 0) ||
        (p-buffer <= size-match2_len && memcmp(p, match2, match2_len) == 0))
    {
      if (memcmp(p, match1, match1_len) == 0) {fputs(match1c, out1); p += match1_len;}
      else                                    {fputs(match2c, out1); p += match2_len;}

      s = string;
      while (p < buffer+size)
      {
        if (*p == '\\')  // escape sequence
        {
          p++;
          if (p < buffer+size)
          {
                 if (*p ==  'n') *s++ = '\n';
            else if (*p == '\"') *s++ = '\"';
            else if (*p == '\\') *s++ = '\\';
            p++;
          }
        }
        else if (*p == '\"')  // end of string
        {
          p++;
          *s++ = 0;
          PrintString(out2, out1, string, s-string);
          break;
        }
        else
          *s++ = *p++;
      }
    }
    else putc(*p++, out1);
  }

  fclose(f);
  fclose(out1);

  free(buffer);
  free(string);
}



void ProcessData(FILE *f, const char *listname, struct LABEL_STRUCT list[], int listsize)
{
  if (PrintStringMode)
  {
    int j;

    bit = 0;
    byte = 0;
    bytes_out = 0;
    bytes_in = 0;

    Count = 0;

    fprintf(f, "\nextern const uint8_t %s[] PROGMEM =\n{", listname);
    for (j=0; j<listsize; j++)
    {
      unsigned char *p = (unsigned char *)list[j].text;
      int size = strlen(list[j].text)+1;

      StartP[Count] = bytes_out;
      StartB[Count] = bit;
      Count++;

      bytes_in += size;

      fputs("\n  ", f);
      while (size)
      {
        int i, len;
        unsigned char c;

        SUBST

        len = strlen(HuffmanCode[c]);
        for (i=0; i<len; i++)
        {
          if (HuffmanCode[c][i] == '1') byte |= 1<<bit;
          if (bit == 7)
          {
            fprintf(f, "%i,", byte);
            bytes_out++;
            bit = 0;
            byte = 0;
          }
          else bit++;
        }
      }
    }

    if (bit > 0)
    {
      fprintf(f, "%i,", byte);
      bytes_out++;
    }

    fprintf(f, "\n};\n");

    printf("%s: Bytes Out = %i  %% Comp = %i\n", listname, bytes_out, 100*bytes_out/bytes_in);

    fprintf(f, "\nextern const uint16_t %sStartP[] PROGMEM =\n{\n", listname);
    for (j=5; j<Count; j+=5)
      fprintf(f, "  %i, // %i\n", StartP[j], j);
    fputs("};\n", f);

    fprintf(f, "\nextern const uint8_t %sStartB[] PROGMEM =\n{\n", listname);
    for (j=5; j<Count; j+=5)
      fprintf(f, "  %i, // %i\n", StartB[j], j);
    fputs("};\n", f);
  }
  else
  {
    int j;

    for (j=0; j<listsize; j++)
    {
      unsigned char *p = (unsigned char *)list[j].text;
      int size = strlen(list[j].text)+1;

      while (size)
      {
        unsigned char c;

        SUBST

        CharFrequency[(int)c]++;

        if (c == 1) c = '\n';
        else if (c == 10) c = '|';
        else if (c < 32) c = '_';
        fputc(c, f);
      }
    }
  }
}



void ProcessTextRoutine(FILE *out2)
{
  Count = 0;

  if (PrintStringMode)
  {
    fputs("#include \"def.h\"\n\nextern const uint8_t CompString[] PROGMEM =\n{", out2);

    bit = 0;
    byte = 0;
    bytes_out = 0; 
    bytes_in = 0;
  }

  ProcessText( "game.cxx"     , "_game.cpp"     , out2 );
  ProcessText( "parser.cxx"   , "_parser.cpp"   , out2 );
  ProcessText( "villains.cxx" , "_villains.cpp" , out2 );

  if (PrintStringMode)
  {
    int i;

    if (bit > 0)
    {
      fprintf(out2, "%i,", byte);
      bytes_out++;
    }
    fputs("\n};\n", out2);

    printf("CompString: Bytes Out = %i  %% Comp = %i\n", bytes_out, 100*bytes_out/bytes_in);

    fprintf(out2, "\nextern const uint16_t CompStringStartP[] PROGMEM =\n{\n");
    for (i=10; i<Count; i+=10)
      fprintf(out2, "  %i, // %i\n", StartP[i], i);
    fputs("};\n", out2);

    fprintf(out2, "\nextern const uint8_t CompStringStartB[] PROGMEM =\n{\n");
    for (i=10; i<Count; i+=10)
      fprintf(out2, "  %i, // %i\n", StartB[i], i);
    fputs("};\n", out2);
  }


  ProcessData(out2, "RoomDesc",   RoomDesc,   NUM_ROOMS);
  ProcessData(out2, "BlockMsg",   BlockMsg,   NUM_BLOCK_MESSAGES);
  ProcessData(out2, "ObjectDesc", ObjectDesc, NUM_OBJECTS);
}

//*****************************************************************************



//#############################################################################

int main(void)
{
  FILE *out2;
  int i;

  PrintStringMode = 0;

  out2 = fopen("_strings.txt", "wb");
  ProcessTextRoutine(out2);
  fclose(out2);
  BuildHuffmanTree();

  PrintStringMode = 1;

  out2 = fopen("_text.cpp", "wb");
  ProcessTextRoutine(out2);

  fputs("\nextern const uint8_t HuffmanTree[] PROGMEM =\n{\n", out2);
  for (i=0; i<256; i++)
  {
    int c = NodePool[i].c;
    int left = NodePool[i].left - NodePool;
    int right = NodePool[i].right - NodePool;

    if (left < 0) left = 0;
    if (right < 0) right = 0;
    if (c == 0 && left == 0 && right == 0) break;
    fprintf(out2, "  %i,%i,%i,\n", c, left, right);
  }
  fputs("};\n", out2);
  fprintf(out2, "\nextern const uint8_t TreeStartIndex = %i;\n", i-1);

  fclose(out2);

  return 0;
}

//#############################################################################
