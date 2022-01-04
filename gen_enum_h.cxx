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



void PrintLabels(FILE *f, struct LABEL_STRUCT list[], int listsize)
{
  int i;

  fputs("enum\n{\n", f);
  for (i=0; i<listsize; i++)
  {
    fputs("  ", f);
    fputs(list[i].label, f);
    if (i < listsize-1) fputs(",\n", f); else fputs("\n", f);
  }
  fputs("};\n\n", f);
}



int main(void)
{
  FILE *f;

  f = fopen("_enum.h", "w");
  PrintLabels(f, RoomDesc,   NUM_ROOMS);
  PrintLabels(f, BlockMsg,   NUM_BLOCK_MESSAGES);
  PrintLabels(f, ObjectDesc, NUM_OBJECTS);
  fclose(f);

  return 0;
}
