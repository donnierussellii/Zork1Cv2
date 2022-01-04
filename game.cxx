// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"
#include "_enum.h"



//game state
struct GAMEFLAGS GameFlags;
struct GAMEVARS GameVars;
struct ROOM_STRUCT Room[NUM_ROOMS];
struct OBJ_STRUCT Obj[NUM_OBJECTS];



//from parser.c
extern int CurWord;
extern int NumStrWords;
extern unsigned int StrWord[];
extern int ItObj;
extern unsigned char TimePassed;
extern unsigned char GameOver;



//*****************************************************************************
// returns 1 if event of x% chance occurred
// second parameter is used instead if it is >=0 and you're not lucky
int PercentChance(int x, int x_not_lucky)
{
  if (NotLucky && x_not_lucky >= 0) x = x_not_lucky;

  if (GetRandom(100) < x) return 1;
  else return 0;
}
//*****************************************************************************



//*****************************************************************************
void ScatterInventory(void)
{
  int obj;

  if (Obj[OBJ_LAMP].loc == INSIDE + OBJ_YOU)
    Obj[OBJ_LAMP].loc = ROOM_LIVING_ROOM;

  if (Obj[OBJ_COFFIN].loc == INSIDE + OBJ_YOU)
    Obj[OBJ_COFFIN].loc = ROOM_EGYPT_ROOM;

  Obj[OBJ_SWORD].thiefvalue = 0;


  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_YOU)
  {
    int room = NUM_ROOMS;

    if (Obj[obj].thiefvalue > 0)
      for (room=1; room<NUM_ROOMS; room++)
        if ((Room[room].prop & R_BODYOFWATER) == 0 &&
            (Room[room].prop & R_LIT) == 0 &&
            GetRandom(2) == 0) break;

    if (room == NUM_ROOMS)
    {
      int above_ground[11] =
        { ROOM_WEST_OF_HOUSE, ROOM_NORTH_OF_HOUSE, ROOM_EAST_OF_HOUSE,
          ROOM_SOUTH_OF_HOUSE, ROOM_FOREST_1, ROOM_FOREST_2, ROOM_FOREST_3,
          ROOM_PATH, ROOM_CLEARING, ROOM_GRATING_CLEARING, ROOM_CANYON_VIEW };

      room = above_ground[GetRandom(11)];
    }

    Obj[obj].loc = room;
  }
}



void YoureDead(void)
{
  if (YouAreDead)
  {
    PrintLine("\nIt takes a talented person to be killed while already dead. YOU are such a talent. Unfortunately, it takes a talented person to deal with it. I am not such a talent. Sorry.");
    GameOver = 1;
    return;
  }

  if (NotLucky)
    PrintLine("Bad luck, huh?");

  PrintLine("\n    ****  You have died  ****\n\n\n");

  NumDeaths++;
  if (NumDeaths == 3)
  {
    PrintLine("You clearly are a suicidal maniac.  We don't allow psychotics in the cave, since they may harm other adventurers.  Your remains will be installed in the Land of the Living Dead, where your fellow adventurers may gloat over them.");
    GameOver = 1;
    return;
  }

  YouAreInBoat = 0; // in case you're in it
  ExitFound = 1;
  ScatterInventory();

  if (Room[ROOM_SOUTH_TEMPLE].prop & R_DESCRIBED)
  {
    PrintLine("As you take your last breath, you feel relieved of your burdens. The feeling passes as you find yourself before the gates of Hell, where the spirits jeer at you and deny you entry.  Your senses are disturbed.  The objects in the dungeon appear indistinct, bleached of color, even unreal.\n\n");

    YouAreDead = 1;
    TrollAllowsPassage = 1;
    Obj[OBJ_LAMP].prop |= PROP_INVISIBLE;
    Obj[OBJ_YOU].prop |= PROP_LIT;
    Obj[OBJ_YOU].loc = ROOM_ENTRANCE_TO_HADES;
    PrintPlayerRoomDesc(0);
  }
  else
  {
    PrintLine("Now, let's take a look here... Well, you probably deserve another chance.  I can't quite fix you up completely, but you can't have everything.\n\n");

    Obj[OBJ_YOU].loc = ROOM_FOREST_1;
    PrintPlayerRoomDesc(0);
  }
}
//*****************************************************************************



//*****************************************************************************
//these functions return 1 if action completed; otherwise fall through



int GoToRoutine(int newroom)
{
  int prev_darkness;

  if (YouAreInBoat)
  {
    PrintLine("You'll have to get out of the boat first.");
    return 1;
  }

  prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_YOU].loc = newroom;
  TimePassed = 1;

  if (IsPlayerInDarkness())
  {
    if (prev_darkness)
    {
      //kill player that tried to walk from dark to dark
      PrintLine("\n\n\n\n\nOh, no! You have walked into the slavering fangs of a lurking grue!");
      YoureDead(); // ##### RIP #####
      return 1;
    }
    else PrintLine("You have moved into a dark place.");
  }

  PrintPlayerRoomDesc(0);
  return 1;
}



int GoFrom_StoneBarrow_West(void)
{
  PrintLine("Inside the Barrow\nAs you enter the barrow, the door closes inexorably behind you. Around you it is dark, but ahead is an enormous cavern, brightly lit. Through its center runs a wide stream. Spanning the stream is a small wooden footbridge, and beyond a path leads into a dark tunnel. Above the bridge, floating in the air, is a large sign. It reads:  All ye who stand before this bridge have completed a great and perilous adventure which has tested your wit and courage. You have mastered the first part of the ZORK trilogy. Those who pass over this bridge must be prepared to undertake an even greater adventure that will severely test your skill and bravery!\n\nThe ZORK trilogy continues with \"ZORK II: The Wizard of Frobozz\" and is completed in \"ZORK III: The Dungeon Master.\"");
  GameOver = 1;
  return 1;
}



int GoFrom_WestOfHouse_Southwest(void)
{
  if (WonGame == 0) return 0;
  else return GoToRoutine(ROOM_STONE_BARROW);
}



int GoFrom_EastOfHouse_West(void)
{
  if (KitchenWindowOpen == 0)
  {
    PrintLine("The window is closed.");
    ItObj = FOBJ_KITCHEN_WINDOW;
    return 1;
  }
  else return GoToRoutine(ROOM_KITCHEN);
}



int GoFrom_Kitchen_East(void)
{
  if (KitchenWindowOpen == 0)
  {
    PrintLine("The window is closed.");
    ItObj = FOBJ_KITCHEN_WINDOW;
    return 1;
  }
  else return GoToRoutine(ROOM_EAST_OF_HOUSE);
}



int GoFrom_LivingRoom_West(void)
{
  if (CyclopsState == 4) // fled
    return GoToRoutine(ROOM_STRANGE_PASSAGE);
  else
    {PrintLine("The door is nailed shut."); return 1;}
}



int GoFrom_Cellar_Up(void)
{
  if (TrapOpen == 0)
  {
    PrintLine("The trap door is closed.");
    ItObj = FOBJ_TRAP_DOOR;
  }
  else
    return GoToRoutine(ROOM_LIVING_ROOM);

  return 1;
}



int GoFrom_TrollRoom_East(void)
{
  if (TrollAllowsPassage == 0) {PrintLine("The troll fends you off with a menacing gesture."); return 1;}
  else return GoToRoutine(ROOM_EW_PASSAGE);
}



int GoFrom_TrollRoom_West(void)
{
  if (TrollAllowsPassage == 0) {PrintLine("The troll fends you off with a menacing gesture."); return 1;}
  else return GoToRoutine(ROOM_MAZE_1);
}



int GoFrom_GratingRoom_Up(void)
{
  if (GratingOpen == 0)
  {
    PrintLine("The grating is closed.");
    ItObj = FOBJ_GRATE;
  }
  else
  {
    ExitFound = 1;
    return GoToRoutine(ROOM_GRATING_CLEARING);
  }

  return 1;
}



int GoFrom_CyclopsRoom_East(void)
{
  if (CyclopsState == 4) // fled
    return GoToRoutine(ROOM_STRANGE_PASSAGE);
  else
    {PrintLine("The east wall is solid rock."); return 1;}
}



int GoFrom_CyclopsRoom_Up(void)
{
  if (CyclopsState == 3 || Obj[OBJ_CYCLOPS].loc == 0) // sleeping or dead
  {
    if (YouAreInBoat == 0) ThiefProtectsTreasure();
    return GoToRoutine(ROOM_TREASURE_ROOM);
  }
  else
    PrintLine("The cyclops doesn't look like he'll let you past.");

  return 1;
}



int GoFrom_ReservoirSouth_North(void)
{
  if (LowTide == 0) {PrintLine("You would drown."); return 1;}
  else return GoToRoutine(ROOM_RESERVOIR);
}



int GoFrom_ReservoirNorth_South(void)
{
  if (LowTide == 0) {PrintLine("You would drown."); return 1;}
  else return GoToRoutine(ROOM_RESERVOIR);
}



int GoFrom_EntranceToHades_South(void)
{
  if (SpiritsBanished == 0) {PrintLine("Some invisible force prevents you from passing through the gate."); return 1;}
  else return GoToRoutine(ROOM_LAND_OF_LIVING_DEAD);
}



int GoFrom_DomeRoom_Down(void)
{
  if (RopeTiedToRail == 0) {PrintLine("You cannot go down without fracturing many bones."); return 1;}
  else return GoToRoutine(ROOM_TORCH_ROOM);
}



int GoFrom_OntoRainbowRoutine(void)
{
  if (RainbowSolid == 0) return 0;
  else return GoToRoutine(ROOM_ON_RAINBOW);
}



int GoFrom_Maze2_Down(void)
{
  PrintLine("You won't be able to get back up to the tunnel you are going through when it gets to the next room.\n\n");
  return GoToRoutine(ROOM_MAZE_4);
}



int GoFrom_Maze7_Down(void)
{
  PrintLine("You won't be able to get back up to the tunnel you are going through when it gets to the next room.\n\n");
  return GoToRoutine(ROOM_DEAD_END_1);
}



int GoFrom_Maze9_Down(void)
{
  PrintLine("You won't be able to get back up to the tunnel you are going through when it gets to the next room.\n\n");
  return GoToRoutine(ROOM_MAZE_11);
}



int GoFrom_Maze12_Down(void)
{
  PrintLine("You won't be able to get back up to the tunnel you are going through when it gets to the next room.\n\n");
  return GoToRoutine(ROOM_MAZE_5);
}



int GoFrom_GratingClearing_Down(void)
{
  if (GratingRevealed == 0)
    PrintBlockMsg(BL0);
  else
  {
    if (GratingOpen == 0)
    {
      PrintLine("The grating is closed.");
      ItObj = FOBJ_GRATE;
    }
    else
      return GoToRoutine(ROOM_GRATING_ROOM);
  }

  return 1;
}



int GoFrom_LivingRoom_Down(void)
{
  if (TrapOpen)
  {
    if (YouAreInBoat)
      PrintLine("You'll have to get out of the boat first.");
    else
    {
      GoToRoutine(ROOM_CELLAR);
      if (YouAreDead == 0 && ExitFound == 0)
      {
        TrapOpen = 0;
        PrintLine("The trap door crashes shut, and you hear someone barring it.");
      }
    }
  }
  else if (RugMoved == 0)
    PrintBlockMsg(BL0);
  else
  {
    PrintLine("The trap door is closed.");
    ItObj = FOBJ_TRAP_DOOR;
  }

  return 1;
}



int GoFrom_SouthTemple_Down(void)
{
  if (Obj[OBJ_COFFIN].loc == INSIDE + OBJ_YOU)
    {PrintLine("You haven't a prayer of getting the coffin down there."); return 1;}
  else return GoToRoutine(ROOM_TINY_CAVE);
}



int GoFrom_WhiteCliffsNorth_South(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc == INSIDE + OBJ_YOU)
    {PrintLine("The path is too narrow."); return 1;}
  else return GoToRoutine(ROOM_WHITE_CLIFFS_SOUTH);
}



int GoFrom_WhiteCliffsNorth_West(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc == INSIDE + OBJ_YOU)
    {PrintLine("The path is too narrow."); return 1;}
  else return GoToRoutine(ROOM_DAMP_CAVE);
}



int GoFrom_WhiteCliffsSouth_North(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc == INSIDE + OBJ_YOU)
    {PrintLine("The path is too narrow."); return 1;}
  else return GoToRoutine(ROOM_WHITE_CLIFFS_NORTH);
}



int GoFrom_TimberRoom_West(void)
{
  if (YouAreDead)
    {PrintLine("You cannot enter in your condition."); return 1;}
  else if (GetNumObjectsInLocation(INSIDE + OBJ_YOU) > 0)
    {PrintLine("You cannot fit through this passage with that load."); return 1;}
  else return GoToRoutine(ROOM_LOWER_SHAFT);
}



int GoFrom_LowerShaft_East(void)
{
  if (GetNumObjectsInLocation(INSIDE + OBJ_YOU) > 0)
    {PrintLine("You cannot fit through this passage with that load."); return 1;}
  else return GoToRoutine(ROOM_TIMBER_ROOM);
}



int GoFrom_Kitchen_Down(void)
{
  if (YouAreSanta == 0)
    PrintLine("Only Santa Claus climbs down chimneys.");
  else
    return GoToRoutine(ROOM_STUDIO);

  return 1;
}



int GoFrom_Studio_Up(void)
{
  int count = GetNumObjectsInLocation(INSIDE + OBJ_YOU);

  if (count == 0)
    PrintLine("Going up empty-handed is a bad idea.");
  else if (count < 3 && Obj[OBJ_LAMP].loc == INSIDE + OBJ_YOU)
    return GoToRoutine(ROOM_KITCHEN);
  else
    PrintLine("You can't get up there with what you're carrying.");

  return 1;
}



int GoFrom_LandOfLivingDead_North(void)
{
  return GoToRoutine(ROOM_ENTRANCE_TO_HADES);
}



int GoFrom_StrangePassage_West(void)
{
  return GoToRoutine(ROOM_CYCLOPS_ROOM);
}



int GoFrom_NorthTemple_North(void)
{
  return GoToRoutine(ROOM_TORCH_ROOM);
}



int GoFrom_MineEntrance_West(void)
{
  return GoToRoutine(ROOM_SQUEEKY_ROOM);
}



int GoFrom_DamLobby_North_Or_East(void)
{
  if (MaintenanceWaterLevel > 14) {PrintLine("The room is full of water and cannot be entered."); return 1;}
  else return GoToRoutine(ROOM_MAINTENANCE_ROOM);
}



int CallGoFrom(int i)
{
  switch (i)
  {
    case  0: return GoFrom_StoneBarrow_West       ();
    case  1: return GoFrom_StoneBarrow_West       ();
    case  2: return GoFrom_WestOfHouse_Southwest  ();
    case  3: return GoFrom_WestOfHouse_Southwest  ();
    case  4: return GoFrom_EastOfHouse_West       ();
    case  5: return GoFrom_EastOfHouse_West       ();
    case  6: return GoFrom_Kitchen_East           ();
    case  7: return GoFrom_Kitchen_East           ();
    case  8: return GoFrom_LivingRoom_West        ();
    case  9: return GoFrom_Cellar_Up              ();
    case 10: return GoFrom_TrollRoom_East         ();
    case 11: return GoFrom_TrollRoom_West         ();
    case 12: return GoFrom_GratingRoom_Up         ();
    case 13: return GoFrom_CyclopsRoom_East       ();
    case 14: return GoFrom_CyclopsRoom_Up         ();
    case 15: return GoFrom_ReservoirSouth_North   ();
    case 16: return GoFrom_ReservoirNorth_South   ();
    case 17: return GoFrom_EntranceToHades_South  ();
    case 18: return GoFrom_EntranceToHades_South  ();
    case 19: return GoFrom_DomeRoom_Down          ();
    case 20: return GoFrom_OntoRainbowRoutine     ();
    case 21: return GoFrom_OntoRainbowRoutine     ();
    case 22: return GoFrom_OntoRainbowRoutine     ();
    case 23: return GoFrom_OntoRainbowRoutine     ();
    case 24: return GoFrom_OntoRainbowRoutine     ();
    case 25: return GoFrom_Maze2_Down             ();
    case 26: return GoFrom_Maze7_Down             ();
    case 27: return GoFrom_Maze9_Down             ();
    case 28: return GoFrom_Maze12_Down            ();
    case 29: return GoFrom_GratingClearing_Down   ();
    case 30: return GoFrom_LivingRoom_Down        ();
    case 31: return GoFrom_SouthTemple_Down       ();
    case 32: return GoFrom_WhiteCliffsNorth_South ();
    case 33: return GoFrom_WhiteCliffsNorth_West  ();
    case 34: return GoFrom_WhiteCliffsSouth_North ();
    case 35: return GoFrom_TimberRoom_West        ();
    case 36: return GoFrom_LowerShaft_East        ();
    case 37: return GoFrom_LowerShaft_East        ();
    case 38: return GoFrom_Kitchen_Down           ();
    case 39: return GoFrom_Studio_Up              ();
    case 40: return GoFrom_LandOfLivingDead_North ();
    case 41: return GoFrom_StrangePassage_West    ();
    case 42: return GoFrom_NorthTemple_North      ();
    case 43: return GoFrom_MineEntrance_West      ();
    case 44: return GoFrom_DamLobby_North_Or_East ();
    case 45: return GoFrom_DamLobby_North_Or_East ();
  }

  return 0; // should never be reached
}
//*****************************************************************************



//*****************************************************************************
void PrintDesc_LivingRoom(void)
{
  if ((Room[ROOM_LIVING_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintText("You are in the living room. There is a doorway to the east");

    if (CyclopsState == 4) // fled
      PrintText(". To the west is a cyclops-shaped opening in an old wooden door, above which is some strange gothic lettering, ");
    else
      PrintText(", a wooden door with strange gothic lettering to the west, which appears to be nailed shut, ");

    PrintText("a trophy case, ");

    if (RugMoved)
    {
      if (TrapOpen)
        PrintLine("and an open trap door at your feet.");
      else
        PrintLine("and a closed trap door at your feet.");
    }
    else
    {
      if (TrapOpen)
        PrintLine("and a rug lying beside an open trap door.");
      else
        PrintLine("and a large oriental rug in the center of the room.");
    }
  }

  if (Obj[OBJ_TROPHY_CASE].prop & PROP_OPEN)
    PrintContents(OBJ_TROPHY_CASE, "Your collection of treasures consists of:", 0);
}



void PrintDesc_EastOfHouse(void)
{
  if ((Room[ROOM_EAST_OF_HOUSE].prop & R_DESCRIBED) == 0)
  {
    PrintText("You are behind the white house. A path leads into the forest to the east. In one corner of the house there is a small window which is ");

    if (KitchenWindowOpen)
      PrintLine("open.");
    else
      PrintLine("slightly ajar.");
  }
}



void PrintDesc_Kitchen(void)
{
  if ((Room[ROOM_KITCHEN].prop & R_DESCRIBED) == 0)
  {
    PrintText("You are in the kitchen of the white house. A table seems to have been used recently for the preparation of food. A passage leads to the west and a dark staircase can be seen leading upward. A dark chimney leads down and to the east is a small window which is ");

    if (KitchenWindowOpen)
      PrintLine("open.");
    else
      PrintLine("slightly ajar.");
  }

  PrintContents(OBJ_KITCHEN_TABLE, "On the table you see:", 0);
}



void PrintDesc_Attic(void)
{
  if ((Room[ROOM_ATTIC].prop & R_DESCRIBED) == 0)
    PrintLine("This is the attic. The only exit is a stairway leading down.");

  PrintContents(OBJ_ATTIC_TABLE, "On a table you see:", 0);
}



void PrintDesc_GratingClearing(void)
{
  if ((Room[ROOM_GRATING_CLEARING].prop & R_DESCRIBED) == 0)
    PrintLine("You are in a clearing, with a forest surrounding you on all sides. A path leads south.");

  if (GratingRevealed)
  {
    if (GratingOpen)
      PrintLine("There is an open grating, descending into darkness.");
    else
      PrintLine("There is a grating securely fastened into the ground.");
  }
}



void PrintDesc_GratingRoom(void)
{
  if ((Room[ROOM_GRATING_ROOM].prop & R_DESCRIBED) == 0)
    PrintLine("You are in a small room near the maze. There are twisty passages in the immediate vicinity.");

  if (GratingOpen)
    PrintLine("Above you is an open grating with sunlight pouring in.");
  else
    PrintLine("Above you is a grating with a skull-and-crossbones lock.");
}



void PrintDesc_DamRoom(void)
{
  if ((Room[ROOM_DAM_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintLine("You are standing on the top of the Flood Control Dam #3, which was quite a tourist attraction in times far distant. There are paths to the north, south, and west, and a scramble down.");

    if (GatesOpen)
    {
      if (LowTide)
        PrintLine("The water level behind the dam is low: The sluice gates have been opened. Water rushes through the dam and downstream.");
      else
        PrintLine("The sluice gates are open, and water rushes through the dam. The water level behind the dam is still high.");
    }
    else
    {
      if (LowTide)
        PrintLine("The sluice gates are closed. The water level in the reservoir is quite low, but the level is rising quickly.");
      else
        PrintLine("The sluice gates on the dam are closed. Behind the dam, there can be seen a wide reservoir. Water is pouring over the top of the now abandoned dam.");
    }
  }

  PrintText("There is a control panel here, on which a large metal bolt is mounted. Directly above the bolt is a small green plastic bubble");

  if (GatesButton)
    PrintText(" which is glowing serenely");

  PrintLine(".");
}



void PrintDesc_ReservoirSouth(void)
{
  if ((Room[ROOM_RESERVOIR_SOUTH].prop & R_DESCRIBED) == 0)
  {
    if (GatesOpen)
    {
      if (LowTide)
        PrintLine("You are in a long room, to the north of which was formerly a lake. However, with the water level lowered, there is merely a wide stream running through the center of the room.");
      else
        PrintLine("You are in a long room. To the north is a large lake, too deep to cross. You notice, however, that the water level appears to be dropping at a rapid rate. Before long, it might be possible to cross to the other side from here.");
    }
    else
    {
      if (LowTide)
        PrintLine("You are in a long room, to the north of which is a wide area which was formerly a reservoir, but now is merely a stream. You notice, however, that the level of the stream is rising quickly and that before long it will be impossible to cross here.");
      else
        PrintLine("You are in a long room on the south shore of a large lake, far too deep and wide for crossing.");
    }

    PrintLine("There is a path along the stream to the east or west, a steep pathway climbing southwest along the edge of a chasm, and a path leading into a canyon to the southeast.");
  }
}



void PrintDesc_Reservoir(void)
{
  if ((Room[ROOM_RESERVOIR].prop & R_DESCRIBED) == 0)
  {
    if (LowTide)
    {
      if (GatesOpen == 0 && YouAreInBoat == 0)
        PrintLine("You notice that the water level here is rising rapidly. The currents are also becoming stronger. Staying here seems quite perilous!");
      else
        PrintLine("You are on what used to be a large lake, but which is now a large mud pile. There are \"shores\" to the north and south.");
    }
    else
      PrintLine("You are on the lake. Beaches can be seen north and south. Upstream a small stream enters the lake through a narrow cleft in the rocks. The dam can be seen downstream.");
  }
}



void PrintDesc_ReservoirNorth(void)
{
  if ((Room[ROOM_RESERVOIR_NORTH].prop & R_DESCRIBED) == 0)
  {
    if (GatesOpen)
    {
      if (LowTide)
        PrintLine("You are in a large cavernous room, the south of which was formerly a lake. However, with the water level lowered, there is merely a wide stream running through there.");
      else
        PrintLine("You are in a large cavernous area. To the south is a wide lake, whose water level appears to be falling rapidly.");
    }
    else
    {
      if (LowTide)
        PrintLine("You are in a cavernous area, to the south of which is a very wide stream. The level of the stream is rising rapidly, and it appears that before long it will be impossible to cross to the other side.");
      else
        PrintLine("You are in a large cavernous room, north of a large lake.");
    }

    PrintLine("There is a slimy stairway leaving the room to the north.");
  }
}



void PrintDesc_LoudRoom(void)
{
  if ((Room[ROOM_LOUD_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintText("This is a large room with a ceiling which cannot be detected from the ground. There is a narrow passage from east to west and a stone stairway leading upward.");

    if (LoudRoomQuiet || (GatesOpen == 0 && LowTide))
      PrintLine(" The room is eerie in its quietness.");
    else
      PrintLine(" The room is deafeningly loud with an undetermined rushing sound. The sound seems to reverberate from all of the walls, making it difficult even to think.");
  }
}



void PrintDesc_DeepCanyon(void)
{
  if ((Room[ROOM_DEEP_CANYON].prop & R_DESCRIBED) == 0)
  {
    PrintText("You are on the south edge of a deep canyon. Passages lead off to the east, northwest and southwest. A stairway leads down.");

    if (GatesOpen)
    {
      if (LowTide)
        PrintLine(" You can hear the sound of flowing water from below.");
      else
        PrintLine(" You can hear a loud roaring sound, like that of rushing water, from below.");
    }
    else
    {
      if (LowTide)
        PrintNewLine();
      else
        PrintLine(" You can hear the sound of flowing water from below.");
    }
  }
}



void PrintDesc_MachineRoom(void)
{
  if ((Room[ROOM_MACHINE_ROOM].prop & R_DESCRIBED) == 0)
  {
    PrintText("This is a large, cold room whose sole exit is to the north. In one corner there is a machine which is reminiscent of a clothes dryer. On its face is a switch which is labelled \"START\". The switch does not appear to be manipulable by any human hand (unless the fingers are about 1/16 by 1/4 inch). On the front of the machine is a large lid, which is ");

    if (Obj[OBJ_MACHINE].prop & PROP_OPEN)
      PrintLine("open.");
    else
      PrintLine("closed.");
  }
}



void PrintDesc_AragainFalls(void)
{
  if ((Room[ROOM_ARAGAIN_FALLS].prop & R_DESCRIBED) == 0)
    PrintLine("You are at the top of Aragain Falls, an enormous waterfall with a drop of about 450 feet. The only path here is on the north end.");

  if (RainbowSolid)
    PrintLine("A solid rainbow spans the falls.");
  else
    PrintLine("A beautiful rainbow can be seen over the falls and to the west.");
}



void PrintDesc_WestOfHouse(void)
{
  if ((Room[ROOM_WEST_OF_HOUSE].prop & R_DESCRIBED) == 0)
  {
    PrintText("You are standing in an open field west of a white house, with a boarded front door.");

    if (WonGame)
      PrintLine(" A secret path leads southwest into the forest.");
    else
      PrintNewLine();
  }
}



void PrintDesc_MirrorRoom1(void)
{
  if ((Room[ROOM_MIRROR_ROOM_1].prop & R_DESCRIBED) == 0)
    PrintLine("You are in a large square room with tall ceilings. On the south wall is an enormous mirror which fills the entire wall. There are exits on the other three sides of the room.");

  if (MirrorBroken)
    PrintLine("Unfortunately, the mirror has been destroyed by your recklessness.");
}



void PrintDesc_MirrorRoom2(void)
{
  if ((Room[ROOM_MIRROR_ROOM_2].prop & R_DESCRIBED) == 0)
    PrintLine("You are in a large square room with tall ceilings. On the south wall is an enormous mirror which fills the entire wall. There are exits on the other three sides of the room.");

  if (MirrorBroken)
    PrintLine("Unfortunately, the mirror has been destroyed by your recklessness.");
}



void PrintDesc_TorchRoom(void)
{
  if ((Room[ROOM_TORCH_ROOM].prop & R_DESCRIBED) == 0)
    PrintLine("This is a large room with a prominent doorway leading to a down staircase. Above you is a large dome. Up around the edge of the dome (20 feet up) is a wooden railing. In the center of the room sits a white marble pedestal.");

  if (RopeTiedToRail)
    PrintLine("A piece of rope descends from the railing above, ending some five feet above your head.");
}



void PrintDesc_DomeRoom(void)
{
  if ((Room[ROOM_DOME_ROOM].prop & R_DESCRIBED) == 0)
    PrintLine("You are at the periphery of a large dome, which forms the ceiling of another room below. Protecting you from a precipitous drop is a wooden railing which circles the dome.");

  if (RopeTiedToRail)
    PrintLine("Hanging down from the railing is a rope which ends about ten feet from the floor below.");
}



void PrintDesc_CyclopsRoom(void)
{
  if ((Room[ROOM_CYCLOPS_ROOM].prop & R_DESCRIBED) == 0)
    PrintLine("This room has an exit on the northwest, and a staircase leading up.");

  if (CyclopsState == 4)
    PrintLine("The east wall, previously solid, now has a cyclops-sized opening in it.");
}



void PrintDesc_UpATree(void)
{
  if ((Room[ROOM_UP_A_TREE].prop & R_DESCRIBED) == 0)
    PrintLine("You are about 10 feet above the ground nestled among some large branches. The nearest branch above you is above your reach.");

  PrintPresentObjects(ROOM_PATH, "On the ground below you can see:", 1); // 1: list, no desc
}



int OverrideRoomDesc(int room)
{
  switch (room)
  {
    case ROOM_LIVING_ROOM       : PrintDesc_LivingRoom      (); return 1;
    case ROOM_EAST_OF_HOUSE     : PrintDesc_EastOfHouse     (); return 1;
    case ROOM_KITCHEN           : PrintDesc_Kitchen         (); return 1;
    case ROOM_ATTIC             : PrintDesc_Attic           (); return 1;
    case ROOM_GRATING_CLEARING  : PrintDesc_GratingClearing (); return 1;
    case ROOM_GRATING_ROOM      : PrintDesc_GratingRoom     (); return 1;
    case ROOM_DAM_ROOM          : PrintDesc_DamRoom         (); return 1;
    case ROOM_RESERVOIR_SOUTH   : PrintDesc_ReservoirSouth  (); return 1;
    case ROOM_RESERVOIR         : PrintDesc_Reservoir       (); return 1;
    case ROOM_RESERVOIR_NORTH   : PrintDesc_ReservoirNorth  (); return 1;
    case ROOM_LOUD_ROOM         : PrintDesc_LoudRoom        (); return 1;
    case ROOM_DEEP_CANYON       : PrintDesc_DeepCanyon      (); return 1;
    case ROOM_MACHINE_ROOM      : PrintDesc_MachineRoom     (); return 1;
    case ROOM_ARAGAIN_FALLS     : PrintDesc_AragainFalls    (); return 1;
    case ROOM_WEST_OF_HOUSE     : PrintDesc_WestOfHouse     (); return 1;
    case ROOM_MIRROR_ROOM_1     : PrintDesc_MirrorRoom1     (); return 1;
    case ROOM_MIRROR_ROOM_2     : PrintDesc_MirrorRoom2     (); return 1;
    case ROOM_TORCH_ROOM        : PrintDesc_TorchRoom       (); return 1;
    case ROOM_DOME_ROOM         : PrintDesc_DomeRoom        (); return 1;
    case ROOM_CYCLOPS_ROOM      : PrintDesc_CyclopsRoom     (); return 1;
    case ROOM_UP_A_TREE         : PrintDesc_UpATree         (); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
// end newline handled by calling function if desc_flag == 0



void PrintDesc_Ghosts(int desc_flag)
{
  if (desc_flag == 0)
    PrintText("a number of ghosts");
  else
  {
    if (YouAreDead == 0)
      PrintLine("The way through the gate is barred by evil spirits, who jeer at your attempts to pass.");
  }
}



void PrintDesc_Bat(int desc_flag)
{
  if (desc_flag == 0)
    PrintText("a bat");
  else
  {
    if (IsObjVisible(OBJ_GARLIC))
      PrintLine("In the corner of the room on the ceiling is a large vampire bat who is obviously deranged and holding his nose.");
    else
      PrintLine("A large vampire bat, hanging from the ceiling, swoops down at you!");
  }
}



void PrintDesc_Troll(int desc_flag)
{
  if (desc_flag == 0)
    PrintText("a troll");
  else
    switch (TrollDescType)
  {
    case 0: PrintLine("A nasty-looking troll, brandishing a bloody axe, blocks all passages out of the room."); break;
    case 1: PrintLine("An unconscious troll is sprawled on the floor. All passages out of the room are open."); break;
    case 2: PrintLine("A pathetically babbling troll is here."); break;
    case 3: PrintLine("A troll is here."); break;
  }
}



void PrintDesc_Thief(int desc_flag)
{
  if (desc_flag == 0)
    PrintText("a thief");
  else
    switch (ThiefDescType)
  {
    case 0: PrintLine("There is a suspicious-looking individual, holding a bag, leaning against one wall. He is armed with a vicious-looking stiletto."); break;
    case 1: PrintLine("There is a suspicious-looking individual lying unconscious on the ground."); break;
  }
}



void PrintDesc_Cyclops(int desc_flag)
{
  if (desc_flag == 0)
    PrintText("a cyclops");
  else
    switch (CyclopsState)
  {
    case 0: PrintLine("A cyclops, who looks prepared to eat horses (much less mere adventurers), blocks the staircase. From his state of health, and the bloodstains on the walls, you gather that he is not very friendly, though he likes people."); break;
    case 1: PrintLine("The cyclops is standing in the corner, eyeing you closely. I don't think he likes you very much. He looks extremely hungry, even for a cyclops."); break;
    case 2: PrintLine("The cyclops, having eaten the hot peppers, appears to be gasping. His enflamed tongue protrudes from his man-sized mouth."); break;
    case 3: PrintLine("The cyclops is sleeping blissfully at the foot of the stairs."); break;
  }
}



void PrintDesc_InflatedBoat(int desc_flag)
{
  if (desc_flag == 0)
    PrintText("a magic boat");
  else
  {
    if (YouAreInBoat)
      PrintLine("You are sitting in a magic boat.");
    else
      PrintLine("There is a magic boat here.");
  }
}



void PrintDesc_Lamp(int desc_flag)
{
  const char *name;

  if (Obj[OBJ_LAMP].prop & PROP_LIT)
    name = "a lit brass lantern";
  else
    name = "a brass lantern";

  if (desc_flag == 0)
    PrintText(name);
  else
  {
    if (Obj[OBJ_LAMP].prop & PROP_MOVEDDESC)
      {PrintText("There is "); PrintText(name); PrintLine(" (battery-powered) here.");}
    else
      PrintLine("A battery-powered brass lantern is on the trophy case.");
  }
}



void PrintDesc_Candles(int desc_flag)
{
  const char *name;

  if (Obj[OBJ_CANDLES].prop & PROP_LIT)
    name = "a pair of burning candles";
  else
    name = "a pair of candles";

  if (desc_flag == 0)
    PrintText(name);
  else
  {
    if (Obj[OBJ_CANDLES].prop & PROP_MOVEDDESC)
      {PrintText("There is "); PrintText(name); PrintLine(" here.");}
    else
      PrintLine("On the two ends of the altar are burning candles.");
  }
}



int OverrideObjectDesc(int obj, int desc_flag)
{
  switch (obj)
  {
    case OBJ_GHOSTS        : PrintDesc_Ghosts       (desc_flag); return 1;
    case OBJ_BAT           : PrintDesc_Bat          (desc_flag); return 1;
    case OBJ_TROLL         : PrintDesc_Troll        (desc_flag); return 1;
    case OBJ_THIEF         : PrintDesc_Thief        (desc_flag); return 1;
    case OBJ_CYCLOPS       : PrintDesc_Cyclops      (desc_flag); return 1;
    case OBJ_INFLATED_BOAT : PrintDesc_InflatedBoat (desc_flag); return 1;
    case OBJ_LAMP          : PrintDesc_Lamp         (desc_flag); return 1;
    case OBJ_CANDLES       : PrintDesc_Candles      (desc_flag); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
void PrintUsingMsg(int obj)
{
  PrintText("(using ");
  PrintObjectDesc(obj, 0);
  PrintText(")\n");
}



void PrintFutileMsg(int obj)
{
  PrintText("Using ");

  if (obj > 0 && obj < NUM_OBJECTS)
    PrintObjectDesc(obj, 0);
  else
    PrintText("that");

  PrintText(" would be futile.\n");
}



void TieRopeToRailingRoutine(void)
{
  if (RopeTiedToRail) {PrintLine("The rope is already tied to it."); return;}

  RopeTiedToRail = 1;
  Obj[OBJ_ROPE].loc = ROOM_DOME_ROOM;
  Obj[OBJ_ROPE].prop |= PROP_NODESC;
  Obj[OBJ_ROPE].prop |= PROP_NOTTAKEABLE;

  PrintLine("The rope drops over the side and comes within ten feet of the floor.");

  TimePassed = 1;
}



void DoMiscWithTo_tie_rope(int with_to)
{
  if (with_to == 0 && Obj[OBJ_YOU].loc == ROOM_DOME_ROOM) {with_to = FOBJ_RAILING; PrintLine("(to railing)");}
  if (with_to == 0) {PrintLine("Please specify what to tie it to."); return;}
  if (with_to != FOBJ_RAILING) {PrintLine("You can't tie the rope to that."); return;}

  TieRopeToRailingRoutine();
}



void DoMiscWithTo_tie_railing(int with_to)
{
  if (with_to == 0 && (Obj[OBJ_ROPE].loc == INSIDE + OBJ_YOU || Obj[OBJ_ROPE].loc == ROOM_DOME_ROOM))
  {
    with_to = OBJ_ROPE;
    PrintUsingMsg(with_to);
  }
  if (with_to == 0) {PrintLine("Please specify what to tie it with."); return;}
  if (with_to != OBJ_ROPE) {PrintLine("You can't tie the railing with that."); return;}

  TieRopeToRailingRoutine();
}



void DoMiscWithTo_untie_rope(int with_to)
{
  if (with_to == 0 && Obj[OBJ_YOU].loc == ROOM_DOME_ROOM && RopeTiedToRail) PrintLine("(from railing)");
  if (with_to != 0 && with_to != FOBJ_RAILING) {PrintLine("The rope isn't tied to that."); return;}

  if (RopeTiedToRail == 0) {PrintLine("It is not tied to anything."); return;}

  RopeTiedToRail = 0;
  Obj[OBJ_ROPE].prop &= ~PROP_NODESC;
  Obj[OBJ_ROPE].prop &= ~PROP_NOTTAKEABLE;

  PrintLine("The rope is now untied.");

  TimePassed = 1;
}



void DoMiscWithTo_turn_bolt(int with_to)
{
  int need = OBJ_WRENCH;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLine("The bolt won't turn with your best effort."); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLine("You're not holding it."); return;}

  if (GatesButton)
  {
    TimePassed = 1;
    Room[ROOM_RESERVOIR_SOUTH].prop &= ~R_DESCRIBED;

    if (GatesOpen)
    {
      GatesOpen = 0;
      Room[ROOM_LOUD_ROOM].prop &= ~R_DESCRIBED;
      ReservoirFillCountdown = 8;
      ReservoirDrainCountdown = 0;
      PrintLine("The sluice gates close and water starts to collect behind the dam.");
    }
    else
    {
      GatesOpen = 1;
      ReservoirFillCountdown = 0;
      ReservoirDrainCountdown = 8;
      PrintLine("The sluice gates open and water pours through the dam.");
    }
  }
  else
    PrintLine("The bolt won't turn with your best effort.");
}



void DoMiscWithTo_fix_leak(int with_to)
{
  int need = OBJ_PUTTY;

  if (MaintenanceWaterLevel <= 0) {PrintLine("At least one of those objects isn't visible here!"); return;}

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLine("Fix it with what?"); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLine("You're not holding it."); return;}

  TimePassed = 1;
  MaintenanceWaterLevel = -1;
  PrintLine("By some miracle of Zorkian technology, you have managed to stop the leak in the dam.");
}



void DoMiscWithTo_inflate_fill_inflatable_boat(int with_to)
{
  int need = OBJ_PUMP;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLine("You don't have enough lung power to inflate it."); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLine("You're not holding the pump."); return;}

  if (Obj[OBJ_INFLATABLE_BOAT].loc != Obj[OBJ_YOU].loc) {PrintLine("The boat must be on the ground to be inflated."); return;}

  TimePassed = 1;

  PrintLine("The boat inflates and appears seaworthy.");
  ItObj = OBJ_INFLATED_BOAT;

  if ((Obj[OBJ_BOAT_LABEL].prop & PROP_MOVEDDESC) == 0)
    PrintLine("A tan label is lying inside the boat.");

  Obj[OBJ_INFLATED_BOAT].loc = Obj[OBJ_INFLATABLE_BOAT].loc;
  Obj[OBJ_INFLATABLE_BOAT].loc = 0;
}



void DoMiscWithTo_inflate_fill_inflated_boat(int /* with_to */ )
{
  PrintLine("Inflating it further would probably burst it.");
}



void DoMiscWithTo_inflate_fill_punctured_boat(int /* with_to */ )
{
  PrintLine("No chance. Some moron punctured it.");
}



void DoMiscWithTo_deflate_inflated_boat(int /* with_to */ )
{
  if (YouAreInBoat) {PrintLine("You can't deflate the boat while you're in it."); return;}

  if (Obj[OBJ_INFLATED_BOAT].loc != Obj[OBJ_YOU].loc) {PrintLine("The boat must be on the ground to be deflated."); return;}

  TimePassed = 1;

  PrintLine("The boat deflates.");
  ItObj = OBJ_INFLATABLE_BOAT;

  Obj[OBJ_INFLATABLE_BOAT].loc = Obj[OBJ_INFLATED_BOAT].loc;
  Obj[OBJ_INFLATED_BOAT].loc = 0;
}



void DoMiscWithTo_deflate_inflatable_boat(int /* with_to */ )
{
  PrintLine("It's already deflated.");
}



void DoMiscWithTo_deflate_punctured_boat(int /* with_to */ )
{
  PrintLine("It's already deflated.");
}



void DoMiscWithTo_fix_punctured_boat(int with_to)
{
  int need = OBJ_PUTTY;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLine("Fix it with what?"); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLine("You're not holding it."); return;}

  TimePassed = 1;

  Obj[OBJ_INFLATABLE_BOAT].loc = Obj[OBJ_PUNCTURED_BOAT].loc;
  Obj[OBJ_PUNCTURED_BOAT].loc = 0;

  PrintLine("Well done. The boat is repaired.");
}



void DoMisc_open_grate(void);



void LockUnlockGrating(int with_to, int lock_flag)
{
  int need = OBJ_KEYS;

  if (GratingRevealed == 0) {PrintLine("At least one of those objects isn't visible here!"); return;}

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLine("You'll need to use something."); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLine("You're not holding it."); return;}

  if (lock_flag)
  {
    if (GratingUnlocked == 0)
      PrintLine("The grating is already locked.");
    else if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
      PrintLine("You can't lock it from this side.");
    else
    {
      int prev_darkness;

      PrintLine("The grating is locked.");

      TimePassed = 1;
      GratingUnlocked = 0;
      GratingOpen = 0; // grating may already be closed here

      prev_darkness = IsPlayerInDarkness();
      Room[ROOM_GRATING_ROOM].prop &= ~R_LIT; // no light spilling from grate opening
      if (IsPlayerInDarkness() != prev_darkness)
      {
        PrintNewLine();
        PrintPlayerRoomDesc(0);
      }
    }
  }
  else //unlock
  {
    if (GratingUnlocked)
      PrintLine("The grating is already unlocked.");
    else if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
      PrintLine("You can't reach the lock from here.");
    else
    {
      TimePassed = 1;
      GratingUnlocked = 1;
      // grating is closed here

      DoMisc_open_grate();
    }
  }
}



void DoMiscWithTo_lock_grate(int with_to)
{
  LockUnlockGrating(with_to, 1); //1: lock
}



void DoMiscWithTo_unlock_grate(int with_to)
{
  LockUnlockGrating(with_to, 0); //0: unlock
}



void ActivateObj(int obj)
{
  int prev_darkness;

  if (Obj[obj].prop & PROP_LIT)
  {
    PrintLine("It's already on!");
    return;
  }

  TimePassed = 1;
  PrintLine("It's on.");

  prev_darkness = IsPlayerInDarkness();
  Obj[obj].prop |= PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DeactivateObj(int obj)
{
  int prev_darkness;

  if ((Obj[obj].prop & PROP_LIT) == 0)
  {
    PrintLine("It's already off!");
    return;
  }

  TimePassed = 1;
  PrintLine("It's off.");

  prev_darkness = IsPlayerInDarkness();
  Obj[obj].prop &= ~PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_activate_lamp(int with_to)
{
  if (with_to != 0) PrintLine("You can't use that.");
  else if (LampTurnsLeft == 0) PrintLine("A burned-out lamp won't light.");
  else ActivateObj(OBJ_LAMP);
}



void DoMiscWithTo_deactivate_lamp(int with_to)
{
  if (with_to != 0) PrintLine("You can't use that.");
  else if (LampTurnsLeft == 0) PrintLine("The lamp has already burned out.");
  else DeactivateObj(OBJ_LAMP);
}



void DoMiscWithTo_activate_match(int with_to)
{
  int prev_darkness;

  if (with_to != 0)
  {
    PrintLine("You can't use that.");
    return;
  }

  if (Obj[OBJ_MATCH].loc != INSIDE + OBJ_YOU)
  {
    PrintLine("You're not holding it.");
    return;
  }

  if (Obj[OBJ_MATCH].prop & PROP_LIT)
  {
    PrintLine("A match is already lit.");
    return;
  }

  if (MatchesLeft <= 1)
  {
    PrintLine("I'm afraid that you have run out of matches.");
    if (MatchesLeft == 0) return;
  }
  MatchesLeft--;

  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_LOWER_SHAFT ||
      Obj[OBJ_YOU].loc == ROOM_TIMBER_ROOM)
  {
    PrintLine("This room is drafty, and the match goes out instantly.");
    return;
  }

  MatchTurnsLeft = 2;
  PrintLine("One of the matches starts to burn.");

  prev_darkness = IsPlayerInDarkness();
  Obj[OBJ_MATCH].prop |= PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_deactivate_match(int with_to)
{
  int prev_darkness;

  if (with_to != 0)
  {
    PrintLine("You can't use that.");
    return;
  }

  if ((Obj[OBJ_MATCH].prop & PROP_LIT) == 0)
  {
    PrintLine("No match is lit.");
    return;
  }

  TimePassed = 1;
  MatchTurnsLeft = 0;
  PrintLine("The match is out.");

  prev_darkness = IsPlayerInDarkness();
  Obj[OBJ_MATCH].prop &= ~PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_activate_candles(int with_to)
{
  if (CandleTurnsLeft == 0)
  {
    PrintLine("Alas, there's not much left of the candles. Certainly not enough to burn.");
    return;
  }

  if (Obj[OBJ_CANDLES].loc != INSIDE + OBJ_YOU)
  {
    PrintLine("You're not holding the candles.");
    return;
  }

  if (with_to == 0 &&
      Obj[OBJ_MATCH].loc == INSIDE + OBJ_YOU &&
      (Obj[OBJ_MATCH].prop & PROP_LIT))
  {
    with_to = OBJ_MATCH;
    PrintLine("(with the match)");
  }

  if (with_to == 0)
  {
    PrintLine("You should say what to light them with.");
    return;
  }

  if (with_to == OBJ_MATCH && (Obj[OBJ_MATCH].prop & PROP_LIT))
  {
    if (Obj[OBJ_MATCH].loc != INSIDE + OBJ_YOU)
      PrintLine("You're not holding the match.");
    else if (Obj[OBJ_CANDLES].prop & PROP_LIT)
      PrintLine("The candles are already lit.");
    else
    {
      int prev_darkness;

      TimePassed = 1;
      PrintLine("The candles are lit.");

      prev_darkness = IsPlayerInDarkness();
      Obj[OBJ_CANDLES].prop |= PROP_LIT;
      if (IsPlayerInDarkness() != prev_darkness)
      {
        PrintNewLine();
        PrintPlayerRoomDesc(1);
      }
    }
  }
  else if (with_to == OBJ_TORCH && (Obj[OBJ_TORCH].prop & PROP_LIT))
  {
    if (Obj[OBJ_TORCH].loc != INSIDE + OBJ_YOU)
      PrintLine("You're not holding the torch.");
    else if (Obj[OBJ_CANDLES].prop & PROP_LIT)
      PrintLine("You realize, just in time, that the candles are already lighted.");
    else
    {
      TimePassed = 1;
      Obj[OBJ_CANDLES].loc = 0;

      PrintLine("The heat from the torch is so intense that the candles are vaporized.");
    }
  }
  else
    PrintLine("You have to light them with something that's burning, you know.");
}



void DoMiscWithTo_deactivate_candles(int with_to)
{
  int prev_darkness;

  if (with_to != 0)
  {
    PrintLine("You can't use that.");
    return;
  }

  if ((Obj[OBJ_CANDLES].prop & PROP_LIT) == 0)
  {
    PrintLine("The candles are not lighted.");
    return;
  }

  TimePassed = 1;
  PrintLine("The flame is extinguished.");
  Obj[OBJ_CANDLES].prop |= PROP_MOVEDDESC; // needed since unmoved description of candles says they are burning

  prev_darkness = IsPlayerInDarkness();
  Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscWithTo_activate_machine(int with_to)
{
  int found, obj;

  if (with_to == 0)
    {PrintLine("It's not clear how to turn it on with your bare hands."); return;}

  if (with_to != OBJ_SCREWDRIVER)
    {PrintLine("It seems that won't do."); return;}

  if (Obj[OBJ_SCREWDRIVER].loc != INSIDE + OBJ_YOU)
    {PrintLine("You're not holding the screwdriver."); return;}

  if (Obj[OBJ_MACHINE].prop & PROP_OPEN)
    {PrintLine("The machine doesn't seem to want to do anything."); return;}

  TimePassed = 1;
  PrintLine("The machine comes to life (figuratively) with a dazzling display of colored lights and bizarre noises. After a few moments, the excitement abates.");

  found = 0;
  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_MACHINE)
  {
    if (found == 0) found = 1;
    if (obj == OBJ_COAL) found = 2;
    Obj[obj].loc = 0;
  }

  if (found == 2)
    Obj[OBJ_DIAMOND].loc = INSIDE + OBJ_MACHINE;
  else if (found == 1)
    Obj[OBJ_GUNK].loc = INSIDE + OBJ_MACHINE;
}



void DoMiscWithTo_dig_sand(int with_to)
{
  int need = OBJ_SHOVEL;

  if (with_to == 0 && Obj[need].loc == INSIDE + OBJ_YOU) {with_to = need; PrintUsingMsg(with_to);}
  if (with_to == 0) {PrintLine("Digging with your hands is silly."); return;}
  if (with_to != need) {PrintFutileMsg(with_to); return;}
  if (Obj[with_to].loc != INSIDE + OBJ_YOU) {PrintLine("You're not holding it."); return;}

  TimePassed = 1;
  CaveHoleDepth++;
  switch (CaveHoleDepth)
  {
    case 1: PrintLine("You seem to be digging a hole here.");                break;
    case 2: PrintLine("The hole is getting deeper, but that's about it.");   break;
    case 3: PrintLine("You are surrounded by a wall of sand on all sides."); break;

    case 4:
      if (Obj[OBJ_SCARAB].prop & PROP_INVISIBLE)
      {
        Obj[OBJ_SCARAB].prop &= ~PROP_INVISIBLE;

        PrintLine("You can see a scarab here in the sand.");
        ItObj = OBJ_SCARAB;
      }
      else
        PrintLine("You find nothing else.");
    break;

    default:
      CaveHoleDepth = 0;
      if (Obj[OBJ_SCARAB].loc == ROOM_SANDY_CAVE)
        Obj[OBJ_SCARAB].prop |= PROP_INVISIBLE;
      PrintLine("The hole collapses, smothering you.");
      YoureDead(); // ##### RIP #####
    break;
  }
}



void DoMiscWithTo_fill_bottle(int with_to)
{
  if (with_to == 0 && (Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE))
    {with_to = OBJ_WATER; PrintLine("(with water)");}

  if (with_to == 0) {PrintLine("Fill it with what?"); return;}
  if (with_to != OBJ_WATER) {PrintLine("You can't fill it with that!"); return;}
  if ((Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE) == 0) {PrintLine("There's no water here!"); return;}
  if ((Obj[OBJ_BOTTLE].prop & PROP_OPEN) == 0)
  {
    PrintLine("The bottle is closed.");
    ItObj = OBJ_BOTTLE;
    return;
  }
  if (Obj[OBJ_WATER].loc == INSIDE + OBJ_BOTTLE) {PrintLine("The bottle is already full of water."); return;}

  TimePassed = 1;
  Obj[OBJ_WATER].loc = INSIDE + OBJ_BOTTLE;
  PrintLine("The bottle is now full of water.");
}



void AttackVillain(int obj, int with_to)
{
  if (with_to >= NUM_OBJECTS)
    {PrintLine("You aren't holding that!"); return;}

  if (with_to == 0)
  {
    int i;

    for (i=2; i<NUM_OBJECTS; i++)
    {
      with_to = Obj[i].order;
      if (Obj[with_to].loc == INSIDE + OBJ_YOU &&
          (Obj[with_to].prop & PROP_WEAPON)) break;
    }

    if (i == NUM_OBJECTS) with_to = 0;
    else PrintUsingMsg(with_to);
  }

  if (obj == OBJ_BAT)
  {
    PrintLine("You can't reach him; he's on the ceiling.");
    return;
  }
  else if (obj == OBJ_GHOSTS)
  {
    if (with_to == 0) PrintLine("You seem unable to interact with these spirits.");
    else              PrintLine("How can you attack a spirit with material objects?");
    return;
  }
  else if (obj == OBJ_THIEF && (Obj[OBJ_THIEF].prop & PROP_INVISIBLE))
  {
    PrintLine("You sense someone nearby, but can't see them.");
    return;
  }

  if (with_to == 0 || with_to == OBJ_YOU)
  {
    PrintText("Trying to attack "); if (obj == OBJ_YOU) PrintText("yourself"); else PrintText("it");
    PrintLine(" with your bare hands is suicidal.");
    return;
  }

  if ((Obj[with_to].prop & PROP_WEAPON) == 0)
  {
    PrintText("Trying to attack "); if (obj == OBJ_YOU) PrintText("yourself"); else PrintText("it");
    PrintLine(" with that is suicidal.");
    return;
  }

  TimePassed = 1;

  if (with_to == OBJ_RUSTY_KNIFE)
  {
    Obj[OBJ_RUSTY_KNIFE].loc = 0;
    PrintLine("As the knife approaches its victim, your mind is submerged by an overmastering will. Slowly, your hand turns, until the rusty blade is an inch from your neck. The knife seems to sing as it savagely slits your throat.");
    YoureDead(); // ##### RIP #####
    return;
  }

  if (obj == OBJ_CYCLOPS && CyclopsState == 3) // asleep
  {
    CyclopsState = 0;
    VillainAttacking[VILLAIN_CYCLOPS] = 1;
    PrintLine("The cyclops yawns and stares at the thing that woke him up.");
    return;
  }

  PlayerBlow(obj, with_to);
}



void DoMiscWithTo_attack_bat     (int with_to) {AttackVillain(OBJ_BAT    , with_to);}
void DoMiscWithTo_attack_ghosts  (int with_to) {AttackVillain(OBJ_GHOSTS , with_to);}
void DoMiscWithTo_attack_cyclops (int with_to) {AttackVillain(OBJ_CYCLOPS, with_to);}
void DoMiscWithTo_attack_thief   (int with_to) {AttackVillain(OBJ_THIEF  , with_to);}
void DoMiscWithTo_attack_troll   (int with_to) {AttackVillain(OBJ_TROLL  , with_to);}
void DoMiscWithTo_attack_yourself(int with_to) {AttackVillain(OBJ_YOU    , with_to);}



int CheckFlameSource(int obj, const char *msg)
{
  if (Obj[obj].loc == INSIDE + OBJ_YOU &&
      (Obj[obj].prop & PROP_LIT))
  {
    PrintLine(msg);
    return obj;
  }
  return 0;
}



void BurnObj(int obj, int with)
{
  if (with == 0) with = CheckFlameSource(OBJ_MATCH  , "(with the match)");
  if (with == 0) with = CheckFlameSource(OBJ_CANDLES, "(with the candles)");
  if (with == 0) with = CheckFlameSource(OBJ_TORCH  , "(with the torch)");

  if (with == 0)
    {PrintLine("You should say what to light it with."); return;}

  if (Obj[with].loc != INSIDE + OBJ_YOU)
  {
    switch (with)
    {
      case OBJ_MATCH:   PrintLine("You're not holding the match.");   break;
      case OBJ_CANDLES: PrintLine("You're not holding the candles."); break;
      case OBJ_TORCH:   PrintLine("You're not holding the torch.");   break;
      default:          PrintLine("You can't light it with that!");   break;
    }
    return;
  }

  if ((Obj[with].prop & PROP_LIT) == 0)
    {PrintLine("You have to light it with something that's burning, you know."); return;}

  if (obj == FOBJ_WHITE_HOUSE)
    {PrintLine("You must be joking."); return;}
  else if (obj == FOBJ_FRONT_DOOR)
    {PrintLine("You cannot burn this door."); return;}
  else if (obj >= NUM_OBJECTS)
    {PrintLine("You can't burn that!"); return;}

  TimePassed = 1;

  if (obj == OBJ_INFLATED_BOAT && YouAreInBoat)
  {
    PrintLine("It catches fire. Unfortunately, you were in it at the time.");
    YouAreInBoat = 0;
    Obj[obj].loc = 0;
    YoureDead(); // ##### RIP #####
    return;
  }

  if (Obj[obj].loc == INSIDE + OBJ_YOU)
  {
    if (obj == OBJ_LEAVES)
      PrintLine("The leaves burn, and so do you.");
    else
      PrintLine("It catches fire. Unfortunately, you were holding it at the time.");
    Obj[obj].loc = 0;
    YoureDead(); // ##### RIP #####
    return;
  }

  Obj[obj].loc = 0;

  if (obj == OBJ_LEAVES)
  {
    PrintLine("The leaves burn.");
    if (GratingRevealed == 0)
    {
      GratingRevealed = 1;
      PrintLine("In disturbing the pile of leaves, a grating is revealed.");
    }
  }
  else if (obj == OBJ_BOOK)
  {
    PrintLine("A booming voice says \"Wrong, cretin!\" and you notice that you have turned into a pile of dust. How, I can't imagine.");
    YoureDead(); // ##### RIP #####
  }
  else
    PrintLine("It catches fire and is consumed.");
}



void DoMiscWithTo_activate_leaves         (int with_to) {BurnObj(OBJ_LEAVES         , with_to);}
void DoMiscWithTo_activate_book           (int with_to) {BurnObj(OBJ_BOOK           , with_to);}
void DoMiscWithTo_activate_sandwich_bag   (int with_to) {BurnObj(OBJ_SANDWICH_BAG   , with_to);}
void DoMiscWithTo_activate_advertisement  (int with_to) {BurnObj(OBJ_ADVERTISEMENT  , with_to);}
void DoMiscWithTo_activate_inflated_boat  (int with_to) {BurnObj(OBJ_INFLATED_BOAT  , with_to);}
void DoMiscWithTo_activate_painting       (int with_to) {BurnObj(OBJ_PAINTING       , with_to);}
void DoMiscWithTo_activate_punctured_boat (int with_to) {BurnObj(OBJ_PUNCTURED_BOAT , with_to);}
void DoMiscWithTo_activate_inflatable_boat(int with_to) {BurnObj(OBJ_INFLATABLE_BOAT, with_to);}
void DoMiscWithTo_activate_coal           (int with_to) {BurnObj(OBJ_COAL           , with_to);}
void DoMiscWithTo_activate_boat_label     (int with_to) {BurnObj(OBJ_BOAT_LABEL     , with_to);}
void DoMiscWithTo_activate_guide          (int with_to) {BurnObj(OBJ_GUIDE          , with_to);}
void DoMiscWithTo_activate_nest           (int with_to) {BurnObj(OBJ_NEST           , with_to);}
void DoMiscWithTo_activate_white_house    (int with_to) {BurnObj(FOBJ_WHITE_HOUSE   , with_to);}
void DoMiscWithTo_activate_front_door     (int with_to) {BurnObj(FOBJ_FRONT_DOOR    , with_to);}



void DoMiscWithTo_activate_torch(int /* with_to */ )
{
  PrintLine("It's already burning.");
}



void DoMiscWithTo_deactivate_torch(int /* with_to */ )
{
  PrintLine("You nearly burn your hand trying to extinguish the flame.");
}



void DoMiscWithTo_turn_book(int /* with_to */ )
{
  PrintLine("Beside page 569, there is only one other page with any legible printing on it. Most of it is unreadable, but the subject seems to be the banishment of evil. Apparently, certain noises, lights, and prayers are efficacious in this regard.");
}



void DoMiscWithTo_pour_water(int with_to)
{
  if (Obj[OBJ_BOTTLE].loc != INSIDE + OBJ_YOU ||
      Obj[OBJ_WATER].loc != INSIDE + OBJ_BOTTLE)
    PrintLine("You don't have any water.");
  else if ((Obj[OBJ_BOTTLE].prop & PROP_OPEN) == 0)
    PrintLine("You'll have to open the bottle first.");
  else if (with_to == 0)
    PrintLine("You need to pour it on something.");
  else
  {
    TimePassed = 1;
    Obj[OBJ_WATER].loc = 0;

    switch (with_to)
    {
      case OBJ_HOT_BELL:
        PrintLine("The water cools the bell and is evaporated.");
        BellHotCountdown = 0;
        Obj[OBJ_BELL].loc = ROOM_ENTRANCE_TO_HADES;
        Obj[OBJ_HOT_BELL].loc = 0;
      break;

      case OBJ_TORCH:
        PrintLine("The water evaporates before it gets close.");
      break;

      case OBJ_MATCH:
      case OBJ_CANDLES:
        if (Obj[with_to].prop & PROP_LIT)
        {
          int prev_darkness = IsPlayerInDarkness();

          PrintLine("It is extinguished.");
          Obj[with_to].prop &= ~PROP_LIT;
          if (with_to == OBJ_MATCH) MatchTurnsLeft = 0;

          if (IsPlayerInDarkness() != prev_darkness)
          {
            PrintNewLine();
            PrintPlayerRoomDesc(1);
          }
        }
        else
          PrintLine("The water spills over it, to the floor, and evaporates.");
      break;

      default: // note that this includes with_to >= NUM_OBJECTS
        PrintLine("The water spills over it, to the floor, and evaporates.");
      break;
    }
  }
}



void DoMiscWithTo_pour_putty(int with_to)
{
  if (Obj[OBJ_PUTTY].loc != INSIDE + OBJ_YOU &&
      ( Obj[OBJ_TUBE].loc != INSIDE + OBJ_YOU ||
        Obj[OBJ_PUTTY].loc != INSIDE + OBJ_TUBE))
    PrintLine("You're not holding it.");
  else if ((Obj[OBJ_TUBE].prop & PROP_OPEN) == 0)
    PrintLine("The tube is closed.");
  else if (with_to == 0)
    PrintLine("You need to pour it on something.");
  else
    switch (with_to)
  {
    case FOBJ_LEAK:
      if (MaintenanceWaterLevel <= 0)
        PrintLine("At least one of those objects isn't visible here!");
      else
      {
        TimePassed = 1;
        MaintenanceWaterLevel = -1;
        PrintLine("By some miracle of Zorkian technology, you have managed to stop the leak in the dam.");
      }
    break;

    case OBJ_PUNCTURED_BOAT:
      TimePassed = 1;
      Obj[OBJ_INFLATABLE_BOAT].loc = Obj[OBJ_PUNCTURED_BOAT].loc;
      Obj[OBJ_PUNCTURED_BOAT].loc = 0;
      PrintLine("Well done. The boat is repaired.");
    break;

    default: // note that this includes with_to >= NUM_OBJECTS
      PrintLine("That would be futile.");
    break;
  }
}



void DoMiscWithTo_oil_bolt(int with_to)
{
  if (with_to == 0)
    PrintLine("Oil it with what?");
  else if (with_to != OBJ_PUTTY)
    PrintLine("You can't oil it with that!");
  else if (Obj[with_to].loc != INSIDE + OBJ_YOU)
    PrintLine("You're not holding it.");
  else
  {
    TimePassed = 1;
    PrintLine("Hmm. It appears the tube contained glue, not oil. Turning the bolt won't get any easier....");
  }
}



void DoMiscWithTo_brush_teeth(int with_to)
{
  if (with_to == 0)
    PrintLine("Dental hygiene is highly recommended, but I'm not sure what you want to brush them with.");
  else if (with_to != OBJ_PUTTY)
    PrintLine("A nice idea, but with that?");
  else if (Obj[with_to].loc != INSIDE + OBJ_YOU)
    PrintLine("You're not holding it.");
  else
  {
    TimePassed = 1;
    PrintLine("Well, you seem to have been brushing your teeth with some sort of glue. As a result, your mouth gets glued together (with your nose) and you die of respiratory failure.");
    YoureDead(); // ##### RIP #####
  }
}



void TieUpRoutine(int i, int with_to)
{
  if (with_to == 0 && Obj[OBJ_ROPE].loc == INSIDE + OBJ_YOU)
  {
    with_to = OBJ_ROPE;
    PrintUsingMsg(with_to);
  }
  if (with_to == 0) {PrintLine("Please specify what to tie him with."); return;}
  if (with_to != OBJ_ROPE) {PrintLine("You can't tie him with that!"); return;}


  if (i == VILLAIN_CYCLOPS)
    PrintLine("You cannot tie the cyclops, though he is fit to be tied.");
  else
  {
    const char *name;

    if (i == VILLAIN_THIEF) name = "thief"; else name = "troll";

    if (VillainStrength[i] < 0)
    {
      PrintText("Your attempt to tie up the "); PrintText(name); PrintLine(" awakens him.");
      VillainStrength[i] = -VillainStrength[i];
      VillainConscious(i);
    }
    else
      {PrintText("The "); PrintText(name); PrintLine(" struggles and you cannot tie him up.");}
  }
}



void DoMiscWithTo_tie_cyclops(int with_to) {TieUpRoutine(VILLAIN_CYCLOPS, with_to);}
void DoMiscWithTo_tie_thief  (int with_to) {TieUpRoutine(VILLAIN_THIEF  , with_to);}
void DoMiscWithTo_tie_troll  (int with_to) {TieUpRoutine(VILLAIN_TROLL  , with_to);}



void CallDoMiscWithTo(int i, int with_to)
{
  switch (i)
  {
    case  0: DoMiscWithTo_tie_rope                     (with_to); return;
    case  1: DoMiscWithTo_tie_railing                  (with_to); return;
    case  2: DoMiscWithTo_untie_rope                   (with_to); return;
    case  3: DoMiscWithTo_turn_bolt                    (with_to); return;
    case  4: DoMiscWithTo_fix_leak                     (with_to); return;
    case  5: DoMiscWithTo_inflate_fill_inflatable_boat (with_to); return;
    case  6: DoMiscWithTo_inflate_fill_inflated_boat   (with_to); return;
    case  7: DoMiscWithTo_inflate_fill_punctured_boat  (with_to); return;
    case  8: DoMiscWithTo_inflate_fill_inflatable_boat (with_to); return;
    case  9: DoMiscWithTo_inflate_fill_inflated_boat   (with_to); return;
    case 10: DoMiscWithTo_inflate_fill_punctured_boat  (with_to); return;
    case 11: DoMiscWithTo_deflate_inflated_boat        (with_to); return;
    case 12: DoMiscWithTo_deflate_inflatable_boat      (with_to); return;
    case 13: DoMiscWithTo_deflate_punctured_boat       (with_to); return;
    case 14: DoMiscWithTo_fix_punctured_boat           (with_to); return;
    case 15: DoMiscWithTo_lock_grate                   (with_to); return;
    case 16: DoMiscWithTo_unlock_grate                 (with_to); return;
    case 17: DoMiscWithTo_activate_lamp                (with_to); return;
    case 18: DoMiscWithTo_deactivate_lamp              (with_to); return;
    case 19: DoMiscWithTo_activate_match               (with_to); return;
    case 20: DoMiscWithTo_deactivate_match             (with_to); return;
    case 21: DoMiscWithTo_activate_candles             (with_to); return;
    case 22: DoMiscWithTo_deactivate_candles           (with_to); return;
    case 23: DoMiscWithTo_activate_machine             (with_to); return;
    case 24: DoMiscWithTo_activate_machine             (with_to); return;
    case 25: DoMiscWithTo_activate_machine             (with_to); return;
    case 26: DoMiscWithTo_dig_sand                     (with_to); return;
    case 27: DoMiscWithTo_fill_bottle                  (with_to); return;
    case 28: DoMiscWithTo_attack_bat                   (with_to); return;
    case 29: DoMiscWithTo_attack_ghosts                (with_to); return;
    case 30: DoMiscWithTo_attack_cyclops               (with_to); return;
    case 31: DoMiscWithTo_attack_thief                 (with_to); return;
    case 32: DoMiscWithTo_attack_troll                 (with_to); return;
    case 33: DoMiscWithTo_attack_yourself              (with_to); return;
    case 34: DoMiscWithTo_activate_leaves              (with_to); return;
    case 35: DoMiscWithTo_activate_book                (with_to); return;
    case 36: DoMiscWithTo_activate_sandwich_bag        (with_to); return;
    case 37: DoMiscWithTo_activate_advertisement       (with_to); return;
    case 38: DoMiscWithTo_activate_inflated_boat       (with_to); return;
    case 39: DoMiscWithTo_activate_painting            (with_to); return;
    case 40: DoMiscWithTo_activate_punctured_boat      (with_to); return;
    case 41: DoMiscWithTo_activate_inflatable_boat     (with_to); return;
    case 42: DoMiscWithTo_activate_coal                (with_to); return;
    case 43: DoMiscWithTo_activate_boat_label          (with_to); return;
    case 44: DoMiscWithTo_activate_guide               (with_to); return;
    case 45: DoMiscWithTo_activate_nest                (with_to); return;
    case 46: DoMiscWithTo_activate_white_house         (with_to); return;
    case 47: DoMiscWithTo_activate_front_door          (with_to); return;
    case 48: DoMiscWithTo_activate_torch               (with_to); return;
    case 49: DoMiscWithTo_deactivate_torch             (with_to); return;
    case 50: DoMiscWithTo_turn_book                    (with_to); return;
    case 51: DoMiscWithTo_pour_water                   (with_to); return;
    case 52: DoMiscWithTo_pour_putty                   (with_to); return;
    case 53: DoMiscWithTo_oil_bolt                     (with_to); return;
    case 54: DoMiscWithTo_brush_teeth                  (with_to); return;
    case 55: DoMiscWithTo_tie_cyclops                  (with_to); return;
    case 56: DoMiscWithTo_tie_thief                    (with_to); return;
    case 57: DoMiscWithTo_tie_troll                    (with_to); return;
  }
}
//*****************************************************************************



//*****************************************************************************
void GiveLunchToCyclops(void)
{
  TimePassed = 1;

  CyclopsCounter = 0;
  CyclopsState = 2; // thirsty

  Obj[OBJ_LUNCH].loc = 0;

  PrintLine("The cyclops says \"Mmm Mmm. I love hot peppers! But oh, could I use a drink. Perhaps I could drink the blood of that thing.\"  From the gleam in his eye, it could be surmised that you are \"that thing\".");
}



void GiveBottleToCyclops(void)
{
  TimePassed = 1;

  if (Obj[OBJ_WATER].loc != INSIDE + OBJ_BOTTLE)
    PrintLine("The cyclops refuses the empty bottle.");
  else if (CyclopsState != 2) // not thirsty
    PrintLine("The cyclops apparently is not thirsty and refuses your generous offer.");
  else
  {
    CyclopsState = 3; // asleep

    Obj[OBJ_WATER].loc = 0;
    Obj[OBJ_BOTTLE].loc = ROOM_CYCLOPS_ROOM;
    Obj[OBJ_BOTTLE].prop |= PROP_OPEN;

    PrintLine("The cyclops takes the bottle, checks that it's open, and drinks the water. A moment later, he lets out a yawn that nearly blows you over, and then falls fast asleep (what did you put in that drink, anyway?).");
  }
}



void DoMiscGiveTo_cyclops(int obj)
{
  if (obj == OBJ_WATER)
    obj = OBJ_BOTTLE;

  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    PrintLine("You aren't holding that!");
  else if (CyclopsState == 3)
    PrintLine("He's asleep.");
  else
    switch (obj)
  {
    case OBJ_LUNCH:  GiveLunchToCyclops();                                          break;
    case OBJ_BOTTLE: GiveBottleToCyclops();                                         break;
    case OBJ_GARLIC: PrintLine("The cyclops may be hungry, but there is a limit."); break;
    default:         PrintLine("The cyclops is not so stupid as to eat THAT!");     break;
  }
}



void DoMiscGiveTo_thief(int obj)
{
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    {PrintLine("You aren't holding that!"); return;}

  if (Obj[OBJ_THIEF].prop & PROP_INVISIBLE)
    {PrintLine("You can't see him, but he could be nearby."); return;}

  TimePassed = 1;

  if (VillainStrength[VILLAIN_THIEF] < 0)
  {
    VillainStrength[VILLAIN_THIEF] = -VillainStrength[VILLAIN_THIEF];
    VillainAttacking[VILLAIN_THIEF] = 1;
    ThiefRecoverStiletto();
    ThiefDescType = 0; // default
    PrintLine("Your proposed victim suddenly recovers consciousness.");
  }

  Obj[obj].loc = INSIDE + OBJ_THIEF;

  if (obj == OBJ_STILETTO)
  {
    Obj[OBJ_STILETTO].prop |= PROP_NODESC;
    Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;

    PrintLine("The thief takes his stiletto and salutes you with a small nod of his head.");
  }
  else if (Obj[obj].thiefvalue > 0)
  {
    Obj[obj].prop |= PROP_INVISIBLE;

    ThiefEngrossed = 1;
    PrintLine("The thief is taken aback by your unexpected generosity, but accepts it and stops to admire its beauty.");
  }
  else
  {
    Obj[obj].prop |= PROP_INVISIBLE;

    PrintLine("The thief places it in his bag and thanks you politely.");
  }
}



void DoMiscGiveTo_troll(int obj)
{
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    {PrintLine("You aren't holding that!"); return;}

  if (TrollDescType == 1) // unconscious
    {PrintLine("The unconscious troll ignores your gift."); return;}

  TimePassed = 1;

  if (obj == OBJ_AXE)
  {
    PrintLine("The troll scratches his head in confusion, then takes the axe.");

    Obj[OBJ_AXE].loc = INSIDE + OBJ_TROLL;

    Obj[OBJ_AXE].prop |= PROP_NODESC;
    Obj[OBJ_AXE].prop |= PROP_NOTTAKEABLE;
    Obj[OBJ_AXE].prop &= ~PROP_WEAPON;

    VillainAttacking[VILLAIN_TROLL] = 1;
  }
  else
  {
    PrintText("The troll, who is not overly proud, graciously accepts the gift");
    if (obj == OBJ_KNIFE || obj == OBJ_SWORD)
    {
      if (PercentChance(20, -1))
      {
        PrintLine(" and eats it hungrily. Poor troll, he dies from an internal hemorrhage and his carcass disappears in a sinister black fog.");
        Obj[obj].loc = 0;
        Obj[OBJ_TROLL].loc = 0;
        VillainDead(VILLAIN_TROLL);
      }
      else
      {
        PrintLine(" and, being for the moment sated, throws it back. Fortunately, the troll has poor control, and it falls to the floor. He does not look pleased.");
        Obj[obj].loc = Obj[OBJ_YOU].loc;
        MoveObjOrderToLast(obj);
        VillainAttacking[VILLAIN_TROLL] = 1;
      }
    }
    else
    {
      int prev_darkness;

      PrintLine(" and not having the most discriminating tastes, gleefully eats it.");

      prev_darkness = IsPlayerInDarkness();
      Obj[obj].loc = 0;
      if (IsPlayerInDarkness() != prev_darkness)
      {
        PrintNewLine();
        PrintPlayerRoomDesc(1);
      }
    }
  }
}



int CallDoMiscGiveTo(int to, int obj)
{
  switch (to)
  {
    case OBJ_CYCLOPS : DoMiscGiveTo_cyclops (obj); return 1;
    case OBJ_THIEF   : DoMiscGiveTo_thief   (obj); return 1;
    case OBJ_TROLL   : DoMiscGiveTo_troll   (obj); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
void ThrowObjRoutine(int obj, int to)
{
  int prev_darkness = IsPlayerInDarkness();

  switch (obj)
  {
    case OBJ_LAMP:
      PrintLine("The lamp has smashed into the floor, and the light has gone out.");
      TimePassed = 1;
      Obj[OBJ_LAMP].loc = 0;
      Obj[OBJ_BROKEN_LAMP].loc = Obj[OBJ_YOU].loc;
    break;

    case OBJ_EGG:
      PrintLine("Your rather indelicate handling of the egg has caused it some damage, although you have succeeded in opening it.");
      TimePassed = 1;
      Obj[OBJ_EGG].loc = 0;
      Obj[OBJ_BROKEN_EGG].loc = Obj[OBJ_YOU].loc;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPENABLE;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPEN;
    break;

    case OBJ_BOTTLE:
      PrintLine("The bottle hits the far wall and shatters.");
      TimePassed = 1;
      Obj[OBJ_BOTTLE].loc = 0;
    break;

    default:
      if (to == 0) PrintLine("It tumbles to the ground.");
      else         PrintLine("You miss.");
      TimePassed = 1;
      Obj[obj].loc = Obj[OBJ_YOU].loc;
      MoveObjOrderToLast(obj);
    break;
  }

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscThrowTo_chasm(int obj)
{
  int prev_darkness = IsPlayerInDarkness();

  PrintLine("It drops out of sight into the chasm.");
  TimePassed = 1;
  Obj[obj].loc = 0;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscThrowTo_river(int obj)
{
  int prev_darkness = IsPlayerInDarkness();

  PrintLine("It tumbles into the river and is seen no more.");
  TimePassed = 1;
  Obj[obj].loc = 0;
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMiscThrowTo_mirror(int obj)
{
  if (MirrorBroken)
    PrintLine("Haven't you done enough damage already?");
  else
  {
    PrintLine("You have broken the mirror. I hope you have a seven years' supply of good luck handy.");
    TimePassed = 1;

    MirrorBroken = 1;
    NotLucky = 1;

    ThrowObjRoutine(obj, 0);
  }
}



void DoMiscThrowTo_troll(int obj)
{
  if (TrollDescType == 1) // unconscious
    ThrowObjRoutine(obj, OBJ_TROLL);
  else
  {
    PrintLine("The troll, who is remarkably coordinated, catches it.");
    DoMiscGiveTo_troll(obj);
  }
}



void DoMiscThrowTo_cyclops(int obj)
{
  if (CyclopsState == 3) // sleeping
    ThrowObjRoutine(obj, OBJ_CYCLOPS);
  else
  {
    PrintLine("\"Do you think I'm as stupid as my father was?\", he says, dodging.");
    ThrowObjRoutine(obj, 0);
  }
}



void ThiefLoseBagContents(void)
{
  int flag = 0, obj;

  PrintText("You evidently frightened the robber, though you didn't hit him. He flees");

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_THIEF &&
        obj != OBJ_LARGE_BAG &&
        obj != OBJ_STILETTO)
  {
    flag = 1;
    Obj[obj].loc = Obj[OBJ_YOU].loc;
    Obj[obj].prop &= ~PROP_INVISIBLE;
  }

  if (flag)
    PrintLine(", but the contents of his bag fall on the floor.");
  else
    PrintLine(".");
}



void DoMiscThrowTo_thief(int obj)
{
  if (Obj[OBJ_THIEF].prop & PROP_INVISIBLE)
    {PrintLine("You can't see him, but he could be nearby."); return;}

  if (ThiefDescType == 1) // unconscious
    ThrowObjRoutine(obj, OBJ_THIEF);
  else
  {
    TimePassed = 1;

    if (obj == OBJ_KNIFE &&
        VillainAttacking[VILLAIN_THIEF] == 0)
    {
      Obj[OBJ_KNIFE].loc = Obj[OBJ_YOU].loc;

      if (PercentChance(10, 0))
      {
        ThiefLoseBagContents();
        Obj[OBJ_THIEF].prop |= PROP_INVISIBLE;
      }
      else
      {
        PrintLine("You missed. The thief makes no attempt to take the knife, though it would be a fine addition to the collection in his bag. He does seem angered by your attempt.");
        VillainAttacking[VILLAIN_THIEF] = 1;
      }
    }
    else
      ThrowObjRoutine(obj, OBJ_THIEF);
  }
}



int CallDoMiscThrowTo(int to, int obj)
{
  switch (to)
  {
    case FOBJ_CHASM           : DoMiscThrowTo_chasm           (obj); return 1;
    case FOBJ_CLIMBABLE_CLIFF : DoMiscThrowTo_river           (obj); return 1;
    case FOBJ_RIVER           : DoMiscThrowTo_river           (obj); return 1;
    case FOBJ_MIRROR1         : DoMiscThrowTo_mirror          (obj); return 1;
    case FOBJ_MIRROR2         : DoMiscThrowTo_mirror          (obj); return 1;
    case OBJ_TROLL            : DoMiscThrowTo_troll           (obj); return 1;
    case OBJ_CYCLOPS          : DoMiscThrowTo_cyclops         (obj); return 1;
    case OBJ_THIEF            : DoMiscThrowTo_thief           (obj); return 1;
  }

  return 0;
}
//*****************************************************************************



//*****************************************************************************
void DoMisc_open_kitchen_window(void)
{
  if (KitchenWindowOpen) PrintLine("It's already open.");
  else
  {
    KitchenWindowOpen = 1;
    TimePassed = 1;
    PrintLine("With great effort, you open the window far enough to allow entry.");
  }
}



void DoMisc_close_kitchen_window(void)
{
  if (KitchenWindowOpen == 0) PrintLine("It's already closed.");
  else
  {
    KitchenWindowOpen = 0;
    TimePassed = 1;
    PrintLine("The window closes (more easily than it opened).");
  }
}



void DoMisc_move_push_rug(void)
{
  if (RugMoved)
    PrintLine("Having moved the carpet previously, you find it impossible to move it again.");
  else
  {
    RugMoved = 1;
    TimePassed = 1;

    if (TrapOpen == 0)
    {
      PrintLine("With a great effort, the rug is moved to one side of the room, revealing the dusty cover of a closed trap door.");
      ItObj = FOBJ_TRAP_DOOR;
    }
    else
      PrintLine("With a great effort, the rug is moved to one side of the room.");
  }
}



void DoMisc_open_trap_door(void)
{
  if (TrapOpen)
    PrintLine("It's already open.");
  else if (Obj[OBJ_YOU].loc == ROOM_LIVING_ROOM)
  {
    if (RugMoved == 0)
      PrintLine("You don't see that here!");
    else
    {
      TrapOpen = 1;
      TimePassed = 1;
      PrintLine("The door reluctantly opens to reveal a rickety staircase descending into darkness.");
    }
  }
  else // cellar
  {
    if (ExitFound == 0)
      PrintLine("The door is locked from above.");
    else
    {
      TrapOpen = 1;
      TimePassed = 1;
      PrintLine("Okay.");
    }
  }
}



void DoMisc_close_trap_door(void)
{
  if (TrapOpen == 0)
    PrintLine("It's already closed.");
  else if (Obj[OBJ_YOU].loc == ROOM_LIVING_ROOM)
  {
    TrapOpen = 0;
    TimePassed = 1;
    PrintLine("The door swings shut and closes.");
  }
  else // cellar
  {
    TrapOpen = 0;
    TimePassed = 1;

    if (ExitFound)
      PrintLine("Okay.");
    else
      PrintLine("The door closes and locks.");
  }
}



void RaiseLowerBasketRoutine(int raise)
{
  int prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_RAISED_BASKET ].loc = raise ? ROOM_SHAFT_ROOM  : ROOM_LOWER_SHAFT;
  Obj[OBJ_LOWERED_BASKET].loc = raise ? ROOM_LOWER_SHAFT : ROOM_SHAFT_ROOM ;

  TimePassed = 1;

  if (raise) PrintLine("The basket is raised to the top of the shaft.");
  else       PrintLine("The basket is lowered to the bottom of the shaft.");

  if (Obj[OBJ_RAISED_BASKET].loc == Obj[OBJ_YOU].loc) ItObj = OBJ_RAISED_BASKET;
  else                                                ItObj = OBJ_LOWERED_BASKET;

  //did room become darkened when basket moved
  if (IsPlayerInDarkness() != prev_darkness && prev_darkness == 0)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMisc_raise_basket(void)
{
  if (Obj[OBJ_RAISED_BASKET].loc == Obj[OBJ_YOU].loc)
  {
    if (Obj[OBJ_YOU].loc == ROOM_LOWER_SHAFT)
      RaiseLowerBasketRoutine(1);
    else PrintLine("Playing in this way with the basket has no effect.");
  }
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_SHAFT_ROOM)
      RaiseLowerBasketRoutine(1);
    else PrintLine("The basket is at the other end of the chain.");
  }
}



void DoMisc_lower_basket(void)
{
  if (Obj[OBJ_RAISED_BASKET].loc == Obj[OBJ_YOU].loc)
  {
    if (Obj[OBJ_YOU].loc == ROOM_SHAFT_ROOM)
      RaiseLowerBasketRoutine(0);
    else PrintLine("Playing in this way with the basket has no effect.");
  }
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_LOWER_SHAFT)
      RaiseLowerBasketRoutine(0);
    else PrintLine("The basket is at the other end of the chain.");
  }
}



void DoMisc_push_blue_button(void)
{
  TimePassed = 1;

  if (MaintenanceWaterLevel == 0)
  {
    MaintenanceWaterLevel = 1;
    PrintLine("There is a rumbling sound and a stream of water appears to burst from the east wall of the room (apparently, a leak has occurred in a pipe).");
  }
  else
    PrintLine("The blue button appears to be jammed.");
}



void DoMisc_push_red_button(void)
{
  int prev_darkness = IsPlayerInDarkness();

  TimePassed = 1;

  PrintText("The lights within the room ");

  if (Room[ROOM_MAINTENANCE_ROOM].prop & R_LIT)
  {
    Room[ROOM_MAINTENANCE_ROOM].prop &= ~R_LIT;
    PrintLine("shut off.");
  }
  else
  {
    Room[ROOM_MAINTENANCE_ROOM].prop |= R_LIT;
    PrintLine("come on.");
  }

  //did room become darkened
  if (IsPlayerInDarkness() != prev_darkness && prev_darkness == 0)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void DoMisc_push_brown_button(void)
{
  PrintLine("Click.");
  Room[ROOM_DAM_ROOM].prop &= ~R_DESCRIBED;
  GatesButton = 0;
  TimePassed = 1;
}



void DoMisc_push_yellow_button(void)
{
  PrintLine("Click.");
  Room[ROOM_DAM_ROOM].prop &= ~R_DESCRIBED;
  GatesButton = 1;
  TimePassed = 1;
}



void DoMisc_enter_inflated_boat(void)
{
  if (Obj[OBJ_INFLATED_BOAT].loc != Obj[OBJ_YOU].loc)
    PrintLine("The boat must be on the ground to be boarded.");
  else if (YouAreInBoat)
    PrintLine("You're already in it!");
  else
  {
    int loc = INSIDE + OBJ_YOU;

    TimePassed = 1;

    if (Obj[OBJ_SCEPTRE].loc == loc || Obj[OBJ_KNIFE].loc == loc || Obj[OBJ_SWORD].loc == loc ||
        Obj[OBJ_RUSTY_KNIFE].loc == loc || Obj[OBJ_AXE].loc == loc || Obj[OBJ_STILETTO].loc == loc)
    {
      PrintLine("Oops! Something sharp seems to have slipped and punctured the boat. The boat deflates to the sounds of hissing, sputtering, and cursing.");
      ItObj = OBJ_PUNCTURED_BOAT;

      Obj[OBJ_PUNCTURED_BOAT].loc = Obj[OBJ_INFLATED_BOAT].loc;
      Obj[OBJ_INFLATED_BOAT].loc = 0;
    }
    else
    {
      YouAreInBoat = 1;
      Obj[OBJ_INFLATED_BOAT].prop |= PROP_NOTTAKEABLE;
      PrintLine("Okay.");
    }
  }
}



void DoMisc_exit_inflated_boat(void)
{
  if (YouAreInBoat == 0)
    PrintLine("You're not in it!");
  else if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
    PrintLine("You should land before disembarking.");
  else
  {
    YouAreInBoat = 0;
    Obj[OBJ_INFLATED_BOAT].prop &= ~PROP_NOTTAKEABLE;
    PrintLine("Okay.");
    TimePassed = 1;
  }
}



void DoMisc_move_leaves(void)
{
  if (GratingRevealed == 0)
  {
    Obj[OBJ_LEAVES].prop |= PROP_MOVEDDESC;
    GratingRevealed = 1;
    TimePassed = 1;
    PrintLine("In disturbing the pile of leaves, a grating is revealed.");
  }
  else
    PrintLine("Moving the leaves reveals nothing.");
}



void DoMisc_open_grate(void)
{
  int leaves_fall = 0, prev_darkness;

  if (GratingRevealed == 0) {PrintLine("At least one of those objects isn't visible here!"); return;}
  if (GratingOpen) {PrintLine("The grating is already open."); return;}
  if (GratingUnlocked == 0) {PrintLine("The grating is locked."); return;}

  TimePassed = 1;
  GratingOpen = 1;

  if ((Obj[OBJ_LEAVES].prop & PROP_MOVEDDESC) == 0)
  {
    leaves_fall = 1;
    Obj[OBJ_LEAVES].prop |= PROP_MOVEDDESC;
    Obj[OBJ_LEAVES].loc = ROOM_GRATING_ROOM;
  }

  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
    PrintLine("The grating opens.");
  else
  {
    PrintLine("The grating opens to reveal trees above you.");
    if (leaves_fall)
      PrintLine("A pile of leaves falls onto your head and to the ground.");
  }

  prev_darkness = IsPlayerInDarkness();
  Room[ROOM_GRATING_ROOM].prop |= R_LIT; // light spilling from grate opening
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(0);
  }
}



void DoMisc_close_grate(void)
{
  int prev_darkness;

  if (GratingRevealed == 0) {PrintLine("At least one of those objects isn't visible here!"); return;}
  if (GratingOpen == 0) {PrintLine("The grating is already closed."); return;}

  TimePassed = 1;
  GratingOpen = 0;

  PrintLine("The grating is closed.");

  prev_darkness = IsPlayerInDarkness();
  Room[ROOM_GRATING_ROOM].prop &= ~R_LIT; // no light spilling from grate opening
  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(0);
  }
}



void DoMisc_ring_bell(void)
{
  TimePassed = 1;

  if (SpiritsBanished == 0 && Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES)
  {
    PrintLine("The bell suddenly becomes red hot and falls to the ground. The wraiths, as if paralyzed, stop their jeering and slowly turn to face you. On their ashen faces, the expression of a long-forgotten terror takes shape.");
    ItObj = OBJ_HOT_BELL;

    Obj[OBJ_BELL].loc = 0;
    Obj[OBJ_HOT_BELL].loc = ROOM_ENTRANCE_TO_HADES;

    if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU)
    {
      PrintLine("In your confusion, the candles drop to the ground (and they are out).");

      Obj[OBJ_CANDLES].loc = ROOM_ENTRANCE_TO_HADES;
      Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
    }

    BellRungCountdown = 6;
    BellHotCountdown = 20;
  }
  else
    PrintLine("Ding, dong.");
}



int AreYouInForest(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_FOREST_1:
    case ROOM_FOREST_2:
    case ROOM_FOREST_3:
    case ROOM_PATH:
    case ROOM_UP_A_TREE:
      return 1;

    default:
      return 0;
  }
}



void DoMisc_wind_canary(void)
{
  TimePassed = 1;

  if (SongbirdSang == 0 && AreYouInForest())
  {
    SongbirdSang = 1;
    PrintLine("The canary chirps, slightly off-key, an aria from a forgotten opera. From out of the greenery flies a lovely songbird. It perches on a limb just over your head and opens its beak to sing. As it does so a beautiful brass bauble drops from its mouth, bounces off the top of your head, and lands glimmering in the grass. As the canary winds down, the songbird flies away.");

    if (Obj[OBJ_YOU].loc == ROOM_UP_A_TREE)
      Obj[OBJ_BAUBLE].loc = ROOM_PATH;
    else
      Obj[OBJ_BAUBLE].loc = Obj[OBJ_YOU].loc;
  }
  else
    PrintLine("The canary chirps blithely, if somewhat tinnily, for a short time.");
}



void DoMisc_wind_broken_canary(void)
{
  TimePassed = 1;
  PrintLine("There is an unpleasant grinding noise from inside the canary.");
}



void DoMisc_wave_sceptre(void)
{
  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_ARAGAIN_FALLS ||
      Obj[OBJ_YOU].loc == ROOM_END_OF_RAINBOW)
  {
    if (RainbowSolid == 0)
    {
      RainbowSolid = 1;
      PrintLine("Suddenly, the rainbow appears to become solid and, I venture, walkable (I think the giveaway was the stairs and bannister).");

      if (Obj[OBJ_YOU].loc == ROOM_END_OF_RAINBOW &&
          (Obj[OBJ_POT_OF_GOLD].prop & PROP_INVISIBLE))
        PrintLine("A shimmering pot of gold appears at the end of the rainbow.");

      Obj[OBJ_POT_OF_GOLD].prop &= ~PROP_INVISIBLE;
    }
    else
    {
      RainbowSolid = 0;
      PrintLine("The rainbow seems to have become somewhat run-of-the-mill.");
    }
  }
  else if (Obj[OBJ_YOU].loc == ROOM_ON_RAINBOW)
  {
    RainbowSolid = 0;
    PrintLine("The structural integrity of the rainbow is severely compromised, leaving you hanging in midair, supported only by water vapor. Bye.");
    YoureDead(); // ##### RIP #####
  }
  else
    PrintLine("A dazzling display of color briefly emanates from the sceptre.");
}



void DoMisc_raise_sceptre(void)
{
  if (Obj[OBJ_SCEPTRE].loc != INSIDE + OBJ_YOU)
    PrintLine("You're not holding it.");
  else
    DoMisc_wave_sceptre();
}



void DoMisc_touch_mirror(void)
{
  int obj;

  if (MirrorBroken)
  {
    PrintText("Fiddling with that ");
    switch (GetRandom(3))
    {
      case 0: PrintLine("doesn't seem to work.");  break;
      case 1: PrintLine("isn't notably helpful."); break;
      case 2: PrintLine("has no effect.");         break;
    }
    return;
  }

  TimePassed = 1;
  PrintLine("There is a rumble from deep within the earth and the room shakes.");

  // note that this includes object 1: OBJ_YOU
  for (obj=1; obj<NUM_OBJECTS; obj++)
  {
         if (Obj[obj].loc == ROOM_MIRROR_ROOM_1) Obj[obj].loc = ROOM_MIRROR_ROOM_2;
    else if (Obj[obj].loc == ROOM_MIRROR_ROOM_2) Obj[obj].loc = ROOM_MIRROR_ROOM_1;
  }
}



void DoMisc_read_book(void)
{
  int obj = OBJ_BOOK;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES && CandlesLitCountdown > 0)
  {
    CandlesLitCountdown = 0;
    Obj[OBJ_GHOSTS].loc = 0;
    SpiritsBanished = 1;

    PrintLine("Each word of the prayer reverberates through the hall in a deafening confusion. As the last word fades, a voice, loud and commanding, speaks: \"Begone, fiends!\" A heart-stopping scream fills the cavern, and the spirits, sensing a greater power, flee through the walls.");
  }
  else
    PrintLine("Commandment #12592\n\nOh ye who go about saying unto each:  \"Hello sailor\":\nDost thou know the magnitude of thy sin before the gods?\nYea, verily, thou shalt be ground between two stones.\nShall the angry gods cast thy body into the whirlpool?\nSurely, thy eye shall be put out with a sharp stick!\nEven unto the ends of the earth shalt thou wander and\nUnto the land of the dead shalt thou be sent at last.\nSurely thou shalt repent of thy cunning.");
}



void DoMisc_read_advertisement(void)
{
  int obj = OBJ_ADVERTISEMENT;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLine("\"WELCOME TO ZORK!\n\nZORK is a game of adventure, danger, and low cunning. In it you will explore some of the most amazing territory ever seen by mortals. No computer should be without one!\"");
}



void DoMisc_read_match(void)
{
  int obj = OBJ_MATCH;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;

  PrintLine("\n(Close cover before striking)\n\nYOU too can make BIG MONEY in the exciting field of PAPER SHUFFLING!\n\nMr. Anderson of Muddle, Mass. says: \"Before I took this course I was a lowly bit twiddler. Now with what I learned at GUE Tech I feel really important and can obfuscate and confuse with the best.\"\n\nDr. Blank had this to say: \"Ten short days ago all I could look forward to was a dead-end job as a doctor. Now I have a promising future and make really big Zorkmids.\"\n\nGUE Tech can't promise these fantastic results to everyone. But when you earn your degree from GUE Tech, your future will be brighter.");
}



void DoMisc_read_map(void)
{
  int obj = OBJ_MAP;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLine("The map shows a forest with three clearings. The largest clearing contains a house. Three paths leave the large clearing. One of these paths, leading southwest, is marked \"To Stone Barrow\".");
}



void DoMisc_read_boat_label(void)
{
  int obj = OBJ_BOAT_LABEL;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLine("  !!!!FROBOZZ MAGIC BOAT COMPANY!!!!\n\nHello, Sailor!\n\nInstructions for use:\n\n   To get into a body of water, say \"Launch\".\n   To get to shore, say \"Land\" or the direction in which you want to maneuver the boat.\n\nWarranty:\n\n  This boat is guaranteed against all defects for a period of 76 milliseconds from date of purchase or until first used, whichever comes first.\n\nWarning:\n   This boat is made of thin plastic.\n   Good Luck!");
}



void DoMisc_read_guide(void)
{
  int obj = OBJ_GUIDE;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;

  PrintLine("\" Flood Control Dam #3\n\nFCD#3 was constructed in year 783 of the Great Underground Empire to harness the mighty Frigid River. This work was supported by a grant of 37 million zorkmids from your omnipotent local tyrant Lord Dimwit Flathead the Excessive. This impressive structure is composed of 370,000 cubic feet of concrete, is 256 feet tall at the center, and 193 feet wide at the top. The lake created behind the dam has a volume of 1.7 billion cubic feet, an area of 12 million square feet, and a shore line of 36 thousand feet.\n");
  PrintLine("The construction of FCD#3 took 112 days from ground breaking to the dedication. It required a work force of 384 slaves, 34 slave drivers, 12 engineers, 2 turtle doves, and a partridge in a pear tree. The work was managed by a command team composed of 2345 bureaucrats, 2347 secretaries (at least two of whom could type), 12,256 paper shufflers, 52,469 rubber stampers, 245,193 red tape processors, and nearly one million dead trees.\n");
  PrintLine("We will now point out some of the more interesting features of FCD#3 as we conduct you on a guided tour of the facilities:\n\n        1) You start your tour here in the Dam Lobby. You will notice on your right that....");
}



void DoMisc_read_tube(void)
{
  int obj = OBJ_TUBE;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLine("---> Frobozz Magic Gunk Company <---\n   All-Purpose Gunk");
}



void DoMisc_read_owners_manual(void)
{
  int obj = OBJ_OWNERS_MANUAL;

  //if not holding it, try to take it
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    if (TakeRoutine(obj, "(taking it first)")) return;

  TimePassed = 1;
  PrintLine("Congratulations!\n\nYou are the privileged owner of ZORK I: The Great Underground Empire, a self-contained and self-maintaining universe. If used and maintained in accordance with normal operating practices for small universes, ZORK will provide many months of trouble-free operation.");
}



void DoMisc_read_prayer(void)
{
  TimePassed = 1;
  PrintLine("The prayer is inscribed in an ancient script, rarely used today. It seems to be a philippic against small insects, absent-mindedness, and the picking up and dropping of small objects. The final verse consigns trespassers to the land of the dead. All evidence indicates that the beliefs of the ancient Zorkers were obscure.");
}



void DoMisc_read_wooden_door(void)
{
  TimePassed = 1;
  PrintLine("The engravings translate to \"This space intentionally left blank.\"");
}



void DoMisc_read_engravings(void)
{
  TimePassed = 1;
  PrintLine("The engravings were incised in the living rock of the cave wall by an unknown hand. They depict, in symbolic form, the beliefs of the ancient Zorkers. Skillfully interwoven with the bas reliefs are excerpts illustrating the major religious tenets of that time. Unfortunately, a later age seems to have considered them blasphemous and just as skillfully excised them.");
}



void DoMisc_open_egg(void)
{
  int with;

  with = GetWith(); if (with < 0) return;

  if (Obj[OBJ_EGG].loc != INSIDE + OBJ_YOU)
    {PrintLine("You aren't holding the egg."); return;}

  if (Obj[OBJ_EGG].prop & PROP_OPEN)
    {PrintLine("The egg is already open."); return;}

  if (with >= NUM_OBJECTS)
    {PrintLine("You aren't holding that!"); return;}

  if (with == 0)
    {PrintLine("You have neither the tools nor the expertise."); return;}

  if (with == OBJ_YOU)
    {PrintLine("I doubt you could do that without damaging it."); return;}

  if ((Obj[with].prop & PROP_WEAPON) ||
      with == OBJ_PUMP || with == OBJ_SCREWDRIVER || with == OBJ_KEYS ||
      with == OBJ_SHOVEL || with == OBJ_PUTTY || with == OBJ_WRENCH)
  {
    PrintLine("The egg is now open, but the clumsiness of your attempt has seriously compromised its esthetic appeal.");
    TimePassed = 1;

    Obj[OBJ_EGG].loc = 0;
    Obj[OBJ_BROKEN_EGG].loc = INSIDE + OBJ_YOU;
    Obj[OBJ_BROKEN_EGG].prop |= PROP_OPENABLE;
    Obj[OBJ_BROKEN_EGG].prop |= PROP_OPEN;
    return;
  }

  PrintLine("You can't open it with that!");
}



void DoMisc_climbthrough_kitchen_window(void)
{
  if (KitchenWindowOpen == 0)
  {
    PrintLine("The window is closed.");
    ItObj = FOBJ_KITCHEN_WINDOW;
  }
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_EAST_OF_HOUSE)
      GoToRoutine(ROOM_KITCHEN);
    else
      GoToRoutine(ROOM_EAST_OF_HOUSE);
  }
}



void DoMisc_climbthrough_trap_door(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LIVING_ROOM)
    GoFrom_LivingRoom_Down();
  else
    GoFrom_Cellar_Up();
}



void DoMisc_climbthrough_grate(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
    GoFrom_GratingClearing_Down();
  else
    GoFrom_GratingRoom_Up();
}



void DoMisc_climbthrough_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CELLAR)
    PrintBlockMsg(BLA);
  else
  {
    if (YouAreInBoat == 0)
      PrintLine("You tumble down the slide....\n\n");
    GoToRoutine(ROOM_CELLAR);
  }
}



void DoMisc_climbthrough_chimney(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN)
    GoFrom_Kitchen_Down();
  else
    GoFrom_Studio_Up();
}



void DoMisc_climbthrough_barrow_door(void)
{
  GoFrom_StoneBarrow_West();
}



void DoMisc_climbthrough_gate(void)
{
  if (SpiritsBanished == 0)
    PrintLine("The gate is protected by an invisible force. It makes your teeth ache to touch it.");
  else
    GoToRoutine(ROOM_LAND_OF_LIVING_DEAD);
}



void DoMisc_climbthrough_crack(void)
{
  PrintLine("You can't fit through the crack.");
}



void DoMisc_enter_white_house(void)
{
  if (Obj[OBJ_YOU].loc != ROOM_EAST_OF_HOUSE)
    PrintLine("I can't see how to get in from here.");
  else
    DoMisc_climbthrough_kitchen_window();
}



void DoMisc_slidedown_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CELLAR)
    PrintLine("You're already at the bottom.");
  else
  {
    if (YouAreInBoat == 0)
      PrintLine("You tumble down the slide....\n\n");
    GoToRoutine(ROOM_CELLAR);
  }
}



void DoMisc_climbup_mountain_range(void)
{
  PrintLine("Don't you believe me? The mountains are impassable!");
}



void DoMisc_climbup_white_cliff(void)
{
  PrintLine("The cliff is too steep for climbing.");
}



void DoMisc_climbup_tree(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_PATH)
    GoToRoutine(ROOM_UP_A_TREE);
  else
    PrintBlockMsg(BL9);
}



void DoMisc_climbdown_tree(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_PATH)
    PrintBlockMsg(BL0);
  else
    GoToRoutine(ROOM_PATH);
}



void DoMisc_climbup_chimney(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_STUDIO)
    GoFrom_Studio_Up();
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbdown_chimney(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN)
    GoFrom_Kitchen_Down();
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbup_ladder(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LADDER_BOTTOM)
    GoToRoutine(ROOM_LADDER_TOP);
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbdown_ladder(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LADDER_TOP)
    GoToRoutine(ROOM_LADDER_BOTTOM);
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbup_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CELLAR)
    PrintBlockMsg(BLA);
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbdown_slide(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_SLIDE_ROOM)
  {
    if (YouAreInBoat == 0)
      PrintLine("You tumble down the slide....\n\n");
    GoToRoutine(ROOM_CELLAR);
  }
  else
    PrintBlockMsg(BL0);
}



void DoMisc_climbup_climbable_cliff(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    default:                 PrintBlockMsg(BL0);             break;
    case ROOM_CLIFF_MIDDLE:  GoToRoutine(ROOM_CANYON_VIEW);  break;
    case ROOM_CANYON_BOTTOM: GoToRoutine(ROOM_CLIFF_MIDDLE); break;
  }
}



void DoMisc_climbdown_climbable_cliff(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_CANYON_VIEW:   GoToRoutine(ROOM_CLIFF_MIDDLE);  break;
    case ROOM_CLIFF_MIDDLE:  GoToRoutine(ROOM_CANYON_BOTTOM); break;
    default:                 PrintBlockMsg(BL0);              break;
  }
}



void DoMisc_climbup_stairs(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_CELLAR:          GoFrom_Cellar_Up();              break;
    case ROOM_CYCLOPS_ROOM:    GoFrom_CyclopsRoom_Up();         break;
    case ROOM_KITCHEN:         GoToRoutine(ROOM_ATTIC);         break;
    case ROOM_RESERVOIR_NORTH: GoToRoutine(ROOM_ATLANTIS_ROOM); break;
    case ROOM_ATLANTIS_ROOM:   GoToRoutine(ROOM_SMALL_CAVE);    break;
    case ROOM_LOUD_ROOM:       GoToRoutine(ROOM_DEEP_CANYON);   break;
    case ROOM_CHASM_ROOM:      GoToRoutine(ROOM_EW_PASSAGE);    break;
    case ROOM_EGYPT_ROOM:      GoToRoutine(ROOM_NORTH_TEMPLE);  break;
    case ROOM_GAS_ROOM:        GoToRoutine(ROOM_SMELLY_ROOM);   break;
    case ROOM_LADDER_TOP:      GoToRoutine(ROOM_MINE_4);        break;
    default:                   PrintBlockMsg(BL0);              break;
  }
}



void DoMisc_climbdown_stairs(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_LIVING_ROOM:   GoFrom_LivingRoom_Down();            break;
    case ROOM_ATTIC:         GoToRoutine(ROOM_KITCHEN);           break;
    case ROOM_TREASURE_ROOM: GoToRoutine(ROOM_CYCLOPS_ROOM);      break;
    case ROOM_SMALL_CAVE:    GoToRoutine(ROOM_ATLANTIS_ROOM);     break;
    case ROOM_TINY_CAVE:     GoToRoutine(ROOM_ENTRANCE_TO_HADES); break;
    case ROOM_EW_PASSAGE:    GoToRoutine(ROOM_CHASM_ROOM);        break;
    case ROOM_DEEP_CANYON:   GoToRoutine(ROOM_LOUD_ROOM);         break;
    case ROOM_TORCH_ROOM:    GoToRoutine(ROOM_NORTH_TEMPLE);      break;
    case ROOM_NORTH_TEMPLE:  GoToRoutine(ROOM_EGYPT_ROOM);        break;
    case ROOM_SMELLY_ROOM:   GoToRoutine(ROOM_GAS_ROOM);          break;
    default:                 PrintBlockMsg(BL0);                  break;
  }
}



void DoMisc_examine_sword(void)
{
  int glow = Obj[OBJ_SWORD].thiefvalue;

       if (glow == 1) PrintLine("Your sword is glowing with a faint blue glow.");
  else if (glow == 2) PrintLine("Your sword is glowing very brightly.");
  else                PrintLine("You don't see anything unusual.");
}



void DoMisc_examine_match(void)
{
  if (Obj[OBJ_MATCH].prop & PROP_LIT)
    PrintLine("The match is burning.");
  else
    PrintLine("The matchbook isn't very interesting, except for what's written on it.");
}



void DoMisc_examine_candles(void)
{
  PrintText("The candles are ");
  if (Obj[OBJ_CANDLES].prop & PROP_LIT)
    PrintLine("burning.");
  else
    PrintLine("out.");
}



void DoMisc_examine_torch(void)
{
  PrintLine("The torch is burning.");
}



void DoMisc_examine_thief(void)
{
  PrintLine("The thief is a slippery character with beady eyes that flit back and forth. He carries, along with an unmistakable arrogance, a large bag over his shoulder and a vicious stiletto, whose blade is aimed menacingly in your direction. I'd watch out if I were you.");
}



void DoMisc_examine_tool_chest(void)
{
  PrintLine("The chests are all empty.");
}



void DoMisc_examine_board(void)
{
  PrintLine("The boards are securely fastened.");
}



void DoMisc_examine_chain(void)
{
  PrintLine("The chain secures a basket within the shaft.");
}



void DoMisc_open_tool_chest(void)
{
  PrintLine("The chests are already open.");
}



void DoMisc_open_book(void)
{
  PrintLine("The book is already open to page 569.");
}



void DoMisc_close_book(void)
{
  PrintLine("As hard as you try, the book cannot be closed.");
}



void DoMisc_open_boarded_window(void)
{
  PrintLine("The windows are boarded and can't be opened.");
}



void DoMisc_break_boarded_window(void)
{
  PrintLine("You can't break the windows open.");
}



void DoMisc_open_close_dam(void)
{
  PrintLine("Sounds reasonable, but this isn't how.");
}



void DoMisc_ring_hot_bell(void)
{
  PrintLine("The bell is too hot to reach.");
}



void DoMisc_read_button(void)
{
  PrintLine("They're greek to you.");
}



void DoMisc_raise_lower_granite_wall(void)
{
  PrintLine("It's solid granite.");
}



void DoMisc_raise_lower_chain(void)
{
  PrintLine("Perhaps you should do that to the basket.");
}



void DoMisc_move_chain(void)
{
  PrintLine("The chain is secure.");
}



void DoMisc_count_candles(void)
{
  PrintLine("Let's see, how many objects in a pair? Don't tell me, I'll get it.");
}



void DoMisc_count_leaves(void)
{
  PrintLine("There are 69,105 leaves here.");
}



void DoMisc_examine_lamp(void)
{
  PrintText("The lamp ");

  if (LampTurnsLeft == 0)
    PrintLine("has burned out.");
  else if (Obj[OBJ_LAMP].prop & PROP_LIT)
    PrintLine("is on.");
  else
    PrintLine("is turned off.");
}



void DoMisc_examine_troll(void)
{
  PrintDesc_Troll(1);
  PrintNewLine(); // above omits end newline
}



void DoMisc_examine_cyclops(void)
{
  if (CyclopsState == 3)
    PrintLine("The cyclops is sleeping like a baby, albeit a very ugly one.");
  else
    PrintLine("A hungry cyclops is standing at the foot of the stairs.");
}



void DoMisc_examine_white_house(void)
{
  PrintLine("The house is a beautiful colonial house which is painted white. It is clear that the owners must have been extremely wealthy.");
}



void DoMisc_open_close_barrow_door(void)
{
  PrintLine("The door is too heavy.");
}



void DoMisc_open_close_studio_door(void)
{
  PrintLine("The door won't budge.");
}



void DoMisc_open_close_bag_of_coins(void)
{
  PrintLine("The coins are safely inside; there's no need to do that.");
}



void DoMisc_open_close_trunk(void)
{
  PrintLine("The jewels are safely inside; there's no need to do that.");
}



void DoMisc_open_close_large_bag(void)
{
  PrintLine("Getting close enough would be a good trick.");
}



void DoMisc_open_front_door(void)
{
  PrintLine("The door cannot be opened.");
}



void DoMisc_count_matches(void)
{
  PrintText("You have ");

  if (MatchesLeft == 0) PrintText("no");
  else PrintInteger(MatchesLeft);

  if (MatchesLeft == 1) PrintLine(" match.");
  else                  PrintLine(" matches.");
}



void EatFood(int obj, const char *msg)
{
  if (Obj[obj].loc != INSIDE + OBJ_YOU)
    PrintLine("You're not holding that.");
  else
  {
    PrintLine(msg);
    TimePassed = 1;
    Obj[obj].loc = 0;
  }
}



void DoMisc_eat_lunch(void)
{
  EatFood(OBJ_LUNCH, "Thank you very much. It really hit the spot.");
}



void DoMisc_eat_garlic(void)
{
  EatFood(OBJ_GARLIC, "What the heck! You won't make friends this way, but nobody around here is too friendly anyhow. Gulp!");
}



void DoMisc_drink_water(void)
{
  if (Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE)
  {
    PrintLine("Thank you very much. I was rather thirsty (from all this talking, probably).");
    TimePassed = 1;
  }
  else if (Obj[OBJ_BOTTLE].loc == Obj[OBJ_YOU].loc ||
           Obj[OBJ_BOTTLE].loc == INSIDE + OBJ_YOU)
  {
    if (Obj[OBJ_BOTTLE].loc != INSIDE + OBJ_YOU)
      PrintLine("You have to be holding the bottle first.");
    else if ((Obj[OBJ_BOTTLE].prop & PROP_OPEN) == 0)
      PrintLine("You'll have to open the bottle first.");
    else if (Obj[OBJ_WATER].loc != INSIDE + OBJ_BOTTLE)
      PrintLine("There isn't any water here.");
    else
    {
      PrintLine("Thank you very much. I was rather thirsty (from all this talking, probably).");
      TimePassed = 1;
      Obj[OBJ_WATER].loc = 0;
    }
  }
  else
    PrintLine("There isn't any water here.");
}



void DoMisc_climbdown_rope(void)
{
  if (RopeTiedToRail && Obj[OBJ_YOU].loc == ROOM_DOME_ROOM)
    GoToRoutine(ROOM_TORCH_ROOM);
  else
    PrintLine("The rope isn't tied to anything.");
}



void DoMisc_break_mirror(void)
{
  if (MirrorBroken)
    PrintLine("Haven't you done enough damage already?");
  else
  {
    PrintLine("You have broken the mirror. I hope you have a seven years' supply of good luck handy.");
    TimePassed = 1;
    MirrorBroken = 1;
    NotLucky = 1;
  }
}



void DoMisc_lookin_mirror(void)
{
  if (MirrorBroken)
    PrintLine("The mirror is broken into many pieces.");
  else
    PrintLine("There is an ugly person staring back at you.");
}



void DoMisc_lookthrough_kitchen_window(void)
{
  PrintText("You can see ");
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN)
    PrintLine("a clear area leading towards a forest.");
  else
    PrintLine("what appears to be a kitchen.");
}



void DoMisc_lookunder_rug(void)
{
  if (RugMoved == 0 && TrapOpen == 0)
  {
    PrintLine("Underneath the rug is a closed trap door. As you drop the corner of the rug, the trap door is once again concealed from view.");
    TimePassed = 1;
  }
  else
    PrintLine("You see nothing under it.");
}



void DoMisc_lookunder_leaves(void)
{
  if (GratingRevealed == 0)
  {
    PrintLine("Underneath the pile of leaves is a grating. As you release the leaves, the grating is once again concealed from view.");
    TimePassed = 1;
    GratingRevealed = 1;
    Obj[OBJ_LEAVES].prop |= PROP_MOVEDDESC;
  }
  else
    PrintLine("You see nothing under the leaves.");
}



void DoMisc_lookunder_rainbow(void)
{
  PrintLine("The Frigid River flows under the rainbow.");
}



void DoMisc_lookin_chimney(void)
{
  PrintText("The chimney leads ");
  if (Obj[OBJ_YOU].loc == ROOM_KITCHEN) PrintText("down");
  else                                  PrintText("up");
  PrintLine("ward, and looks climbable.");
}



void DoMisc_examine_kitchen_window(void)
{
  if (KitchenWindowOpen == 0)
    PrintLine("The window is slightly ajar, but not enough to allow entry.");
  else
    PrintLine("It's open.");
}



void DoMisc_lookin_bag_of_coins(void)
{
  PrintLine("There are lots of coins in there.");
}



void DoMisc_lookin_trunk(void)
{
  PrintLine("There are lots of jewels in there.");
}



void DoMisc_squeeze_tube(void)
{
  if (Obj[OBJ_TUBE].loc != INSIDE + OBJ_YOU)
    PrintLine("You aren't holding the tube.");
  else if ((Obj[OBJ_TUBE].prop & PROP_OPEN) == 0)
    PrintLine("The tube is closed.");
  else if (Obj[OBJ_PUTTY].loc != INSIDE + OBJ_TUBE)
    PrintLine("The tube is apparently empty.");
  else
  {
    PrintLine("The viscous material oozes into your hand.");
    TimePassed = 1;
    Obj[OBJ_PUTTY].loc = INSIDE + OBJ_YOU;
  }
}



void DoMisc_examine_raised_basket(void)
{
  PrintContents(OBJ_RAISED_BASKET, "It contains:", 1);
}



void DoMisc_examine_lowered_basket(void)
{
  PrintLine("The basket is at the other end of the shaft.");
}



void DoMisc_lookin_large_bag(void)
{
  if (ThiefDescType == 1) // unconscious
    PrintLine("The bag is underneath the thief, so one can't say what, if anything, is inside.");
  else
    PrintLine("Getting close enough would be a good trick.");
}



void DoMisc_lookthrough_grate(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING)
    PrintLine("You see darkness below.");
  else
    PrintLine("You see trees above you.");
}



void DoMisc_lookin_water(void)
{
  PrintLine("It's clear and might be potable.");
}



void DoMisc_whereis_granite_wall(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_NORTH_TEMPLE:  PrintLine("The west wall is solid granite here."); break;
    case ROOM_TREASURE_ROOM: PrintLine("The east wall is solid granite here."); break;
    case ROOM_SLIDE_ROOM:    PrintLine("It only SAYS \"Granite Wall\".");       break;
  }
}



void DoMisc_whereis_songbird(void)
{
  PrintLine("The songbird is not here but is probably nearby.");
}



void DoMisc_whereis_white_house(void)
{
  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_KITCHEN:
    case ROOM_LIVING_ROOM:
    case ROOM_ATTIC:
      PrintLine("Why not find your brains?"); // never printed because house is not in these locations
    break;

    case ROOM_EAST_OF_HOUSE:
    case ROOM_WEST_OF_HOUSE:
    case ROOM_NORTH_OF_HOUSE:
    case ROOM_SOUTH_OF_HOUSE:
      PrintLine("It's right here!");
    break;

    case ROOM_CLEARING: PrintLine("It seems to be to the west.");       break;
    default:            PrintLine("It was here just a minute ago...."); break;
  }
}



void DoMisc_whereis_forest(void)
{
  PrintLine("You cannot see the forest for the trees.");
}



void DoMisc_read_granite_wall(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_SLIDE_ROOM)
    PrintLine("It only SAYS \"Granite Wall\".");
  else
    PrintLine("There's nothing to read.");
}



void DoMisc_examine_zorkmid(void)
{
  PrintLine("The zorkmid is the unit of currency of the Great Underground Empire.");
}



void DoMisc_examine_grue(void)
{
  PrintLine("The grue is a sinister, lurking presence in the dark places of the earth. Its favorite diet is adventurers, but its insatiable appetite is tempered by its fear of light. No grue has ever been seen by the light of day, and few have survived its fearsome jaws to tell the tale.");
}



void DoMisc_whereis_zorkmid(void)
{
  PrintLine("The best way to find zorkmids is to go out and look for them.");
}



void DoMisc_whereis_grue(void)
{
  PrintLine("There is no grue here, but I'm sure there is at least one lurking in the darkness nearby. I wouldn't let my light go out if I were you!");
}



void DoMisc_listento_troll(void)
{
  PrintLine("Every so often the troll says something, probably uncomplimentary, in his guttural tongue.");
}



void DoMisc_listento_thief(void)
{
  PrintLine("The thief says nothing, as you have not been formally introduced.");
}



void DoMisc_listento_cyclops(void)
{
  PrintLine("You can hear his stomach rumbling.");
}



void DoMisc_listento_forest(void)
{
  PrintLine("The pines and the hemlocks seem to be murmuring.");
}



void DoMisc_listento_songbird(void)
{
  PrintLine("You can't hear the songbird now.");
}



void DoMisc_cross_rainbow(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_CANYON_VIEW)
    PrintLine("From here?!?");
  else if (RainbowSolid == 0)
    PrintLine("Can you walk on water vapor?");
  else
  {
    if (Obj[OBJ_YOU].loc == ROOM_ARAGAIN_FALLS)
      GoToRoutine(ROOM_END_OF_RAINBOW);
    else if (Obj[OBJ_YOU].loc == ROOM_END_OF_RAINBOW)
      GoToRoutine(ROOM_ARAGAIN_FALLS);
    else
      PrintLine("You'll have to say which way...");
  }
}



void DoMisc_cross_lake(void)
{
  PrintLine("It's too wide to cross.");
}



void DoMisc_cross_stream(void)
{
  PrintLine("The other side is a sheer rock cliff.");
}



void DoMisc_cross_chasm(void)
{
  PrintLine("It's too far to jump, and there's no bridge.");
}



void DoMisc_exorcise_ghosts(void)
{
  if (SpiritsBanished == 0 &&
      Obj[OBJ_BELL].loc == INSIDE + OBJ_YOU &&
      Obj[OBJ_BOOK].loc == INSIDE + OBJ_YOU &&
      Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU)
    PrintLine("You must perform the ceremony.");
  else
    PrintLine("You aren't equipped for an exorcism.");
}



void DoMisc_raise_rug(void)
{
  PrintText("The rug is too heavy to lift");
  if (RugMoved)
    PrintLine(".");
  else
    PrintLine(", but in trying to take it you have noticed an irregularity beneath it.");
}



void DoMisc_raise_trap_door(void)
{
  DoMisc_open_trap_door();
}



void DoMisc_smell_gas(void)
{
  PrintLine("It smells like coal gas in here.");
}



void DoMisc_smell_sandwich_bag(void)
{
  if (Obj[OBJ_LUNCH].loc == INSIDE + OBJ_SANDWICH_BAG)
    PrintLine("It smells of hot peppers.");
  else
    PrintLine("It smells as you would expect.");
}



void CallDoMisc(int i)
{
  switch (i)
  {
    case   0: DoMisc_open_kitchen_window         (); return;
    case   1: DoMisc_close_kitchen_window        (); return;
    case   2: DoMisc_move_push_rug               (); return;
    case   3: DoMisc_move_push_rug               (); return;
    case   4: DoMisc_open_trap_door              (); return;
    case   5: DoMisc_close_trap_door             (); return;
    case   6: DoMisc_raise_basket                (); return;
    case   7: DoMisc_raise_basket                (); return;
    case   8: DoMisc_lower_basket                (); return;
    case   9: DoMisc_lower_basket                (); return;
    case  10: DoMisc_push_blue_button            (); return;
    case  11: DoMisc_push_red_button             (); return;
    case  12: DoMisc_push_brown_button           (); return;
    case  13: DoMisc_push_yellow_button          (); return;
    case  14: DoMisc_enter_inflated_boat         (); return;
    case  15: DoMisc_exit_inflated_boat          (); return;
    case  16: DoMisc_move_leaves                 (); return;
    case  17: DoMisc_open_grate                  (); return;
    case  18: DoMisc_close_grate                 (); return;
    case  19: DoMisc_ring_bell                   (); return;
    case  20: DoMisc_wind_canary                 (); return;
    case  21: DoMisc_wind_broken_canary          (); return;
    case  22: DoMisc_wave_sceptre                (); return;
    case  23: DoMisc_raise_sceptre               (); return;
    case  24: DoMisc_touch_mirror                (); return;
    case  25: DoMisc_touch_mirror                (); return;
    case  26: DoMisc_read_book                   (); return;
    case  27: DoMisc_read_advertisement          (); return;
    case  28: DoMisc_read_match                  (); return;
    case  29: DoMisc_read_map                    (); return;
    case  30: DoMisc_read_boat_label             (); return;
    case  31: DoMisc_read_guide                  (); return;
    case  32: DoMisc_read_tube                   (); return;
    case  33: DoMisc_read_owners_manual          (); return;
    case  34: DoMisc_read_prayer                 (); return;
    case  35: DoMisc_read_wooden_door            (); return;
    case  36: DoMisc_read_engravings             (); return;
    case  37: DoMisc_open_egg                    (); return;
    case  38: DoMisc_open_egg                    (); return;
    case  39: DoMisc_open_egg                    (); return;
    case  40: DoMisc_climbthrough_kitchen_window (); return;
    case  41: DoMisc_climbthrough_kitchen_window (); return;
    case  42: DoMisc_climbthrough_kitchen_window (); return;
    case  43: DoMisc_climbthrough_trap_door      (); return;
    case  44: DoMisc_climbthrough_trap_door      (); return;
    case  45: DoMisc_climbthrough_grate          (); return;
    case  46: DoMisc_climbthrough_grate          (); return;
    case  47: DoMisc_climbthrough_slide          (); return;
    case  48: DoMisc_climbthrough_slide          (); return;
    case  49: DoMisc_climbthrough_chimney        (); return;
    case  50: DoMisc_climbthrough_chimney        (); return;
    case  51: DoMisc_climbthrough_barrow_door    (); return;
    case  52: DoMisc_climbthrough_barrow_door    (); return;
    case  53: DoMisc_climbthrough_barrow_door    (); return;
    case  54: DoMisc_climbthrough_gate           (); return;
    case  55: DoMisc_climbthrough_gate           (); return;
    case  56: DoMisc_climbthrough_crack          (); return;
    case  57: DoMisc_climbthrough_crack          (); return;
    case  58: DoMisc_enter_white_house           (); return;
    case  59: DoMisc_slidedown_slide             (); return;
    case  60: DoMisc_climbup_mountain_range      (); return;
    case  61: DoMisc_climbup_mountain_range      (); return;
    case  62: DoMisc_climbup_white_cliff         (); return;
    case  63: DoMisc_climbup_white_cliff         (); return;
    case  64: DoMisc_climbup_tree                (); return;
    case  65: DoMisc_climbup_tree                (); return;
    case  66: DoMisc_climbdown_tree              (); return;
    case  67: DoMisc_climbup_chimney             (); return;
    case  68: DoMisc_climbup_chimney             (); return;
    case  69: DoMisc_climbdown_chimney           (); return;
    case  70: DoMisc_climbup_ladder              (); return;
    case  71: DoMisc_climbup_ladder              (); return;
    case  72: DoMisc_climbdown_ladder            (); return;
    case  73: DoMisc_climbup_slide               (); return;
    case  74: DoMisc_climbup_slide               (); return;
    case  75: DoMisc_climbdown_slide             (); return;
    case  76: DoMisc_climbup_climbable_cliff     (); return;
    case  77: DoMisc_climbup_climbable_cliff     (); return;
    case  78: DoMisc_climbdown_climbable_cliff   (); return;
    case  79: DoMisc_climbup_stairs              (); return;
    case  80: DoMisc_climbup_stairs              (); return;
    case  81: DoMisc_climbdown_stairs            (); return;
    case  82: DoMisc_examine_sword               (); return;
    case  83: DoMisc_examine_match               (); return;
    case  84: DoMisc_examine_candles             (); return;
    case  85: DoMisc_examine_torch               (); return;
    case  86: DoMisc_examine_thief               (); return;
    case  87: DoMisc_examine_tool_chest          (); return;
    case  88: DoMisc_examine_board               (); return;
    case  89: DoMisc_examine_chain               (); return;
    case  90: DoMisc_open_tool_chest             (); return;
    case  91: DoMisc_open_book                   (); return;
    case  92: DoMisc_close_book                  (); return;
    case  93: DoMisc_open_boarded_window         (); return;
    case  94: DoMisc_break_boarded_window        (); return;
    case  95: DoMisc_open_close_dam              (); return;
    case  96: DoMisc_open_close_dam              (); return;
    case  97: DoMisc_ring_hot_bell               (); return;
    case  98: DoMisc_read_button                 (); return;
    case  99: DoMisc_read_button                 (); return;
    case 100: DoMisc_read_button                 (); return;
    case 101: DoMisc_read_button                 (); return;
    case 102: DoMisc_raise_lower_granite_wall    (); return;
    case 103: DoMisc_raise_lower_granite_wall    (); return;
    case 104: DoMisc_raise_lower_chain           (); return;
    case 105: DoMisc_raise_lower_chain           (); return;
    case 106: DoMisc_move_chain                  (); return;
    case 107: DoMisc_count_candles               (); return;
    case 108: DoMisc_count_leaves                (); return;
    case 109: DoMisc_examine_lamp                (); return;
    case 110: DoMisc_examine_troll               (); return;
    case 111: DoMisc_examine_cyclops             (); return;
    case 112: DoMisc_examine_white_house         (); return;
    case 113: DoMisc_open_close_barrow_door      (); return;
    case 114: DoMisc_open_close_barrow_door      (); return;
    case 115: DoMisc_open_close_studio_door      (); return;
    case 116: DoMisc_open_close_studio_door      (); return;
    case 117: DoMisc_open_close_bag_of_coins     (); return;
    case 118: DoMisc_open_close_bag_of_coins     (); return;
    case 119: DoMisc_open_close_trunk            (); return;
    case 120: DoMisc_open_close_trunk            (); return;
    case 121: DoMisc_open_close_large_bag        (); return;
    case 122: DoMisc_open_close_large_bag        (); return;
    case 123: DoMisc_open_front_door             (); return;
    case 124: DoMisc_count_matches               (); return;
    case 125: DoMisc_count_matches               (); return;
    case 126: DoMisc_eat_lunch                   (); return;
    case 127: DoMisc_eat_garlic                  (); return;
    case 128: DoMisc_drink_water                 (); return;
    case 129: DoMisc_climbdown_rope              (); return;
    case 130: DoMisc_break_mirror                (); return;
    case 131: DoMisc_break_mirror                (); return;
    case 132: DoMisc_lookin_mirror               (); return;
    case 133: DoMisc_lookin_mirror               (); return;
    case 134: DoMisc_lookin_mirror               (); return;
    case 135: DoMisc_lookin_mirror               (); return;
    case 136: DoMisc_lookthrough_kitchen_window  (); return;
    case 137: DoMisc_lookthrough_kitchen_window  (); return;
    case 138: DoMisc_lookunder_rug               (); return;
    case 139: DoMisc_lookunder_leaves            (); return;
    case 140: DoMisc_lookunder_rainbow           (); return;
    case 141: DoMisc_lookin_chimney              (); return;
    case 142: DoMisc_lookin_chimney              (); return;
    case 143: DoMisc_examine_kitchen_window      (); return;
    case 144: DoMisc_lookin_bag_of_coins         (); return;
    case 145: DoMisc_lookin_bag_of_coins         (); return;
    case 146: DoMisc_lookin_trunk                (); return;
    case 147: DoMisc_lookin_trunk                (); return;
    case 148: DoMisc_squeeze_tube                (); return;
    case 149: DoMisc_examine_raised_basket       (); return;
    case 150: DoMisc_examine_lowered_basket      (); return;
    case 151: DoMisc_lookin_large_bag            (); return;
    case 152: DoMisc_lookin_large_bag            (); return;
    case 153: DoMisc_lookthrough_grate           (); return;
    case 154: DoMisc_lookthrough_grate           (); return;
    case 155: DoMisc_lookin_water                (); return;
    case 156: DoMisc_lookin_water                (); return;
    case 157: DoMisc_whereis_granite_wall        (); return;
    case 158: DoMisc_whereis_songbird            (); return;
    case 159: DoMisc_whereis_white_house         (); return;
    case 160: DoMisc_whereis_forest              (); return;
    case 161: DoMisc_read_granite_wall           (); return;
    case 162: DoMisc_examine_zorkmid             (); return;
    case 163: DoMisc_examine_grue                (); return;
    case 164: DoMisc_whereis_zorkmid             (); return;
    case 165: DoMisc_whereis_grue                (); return;
    case 166: DoMisc_listento_troll              (); return;
    case 167: DoMisc_listento_thief              (); return;
    case 168: DoMisc_listento_cyclops            (); return;
    case 169: DoMisc_listento_forest             (); return;
    case 170: DoMisc_listento_songbird           (); return;
    case 171: DoMisc_cross_rainbow               (); return;
    case 172: DoMisc_cross_lake                  (); return;
    case 173: DoMisc_cross_stream                (); return;
    case 174: DoMisc_cross_chasm                 (); return;
    case 175: DoMisc_exorcise_ghosts             (); return;
    case 176: DoMisc_raise_rug                   (); return;
    case 177: DoMisc_raise_trap_door             (); return;
    case 178: DoMisc_smell_gas                   (); return;
    case 179: DoMisc_smell_sandwich_bag          (); return;
  }
}
//*****************************************************************************



//*****************************************************************************
void PrintRandomFun(void)
{
  switch (GetRandom(4))
  {
    case 0: PrintLine("Very good. Now you can go to the second grade."); break;
    case 1: PrintLine("Are you enjoying yourself?");                     break;
    case 2: PrintLine("Wheeeeeeeeee!!!!!");                              break;
    case 3: PrintLine("Do you expect me to applaud?");                   break;
  }
}



void PrintRandomJumpDeath(void)
{
  switch (GetRandom(3))
  {
    case 0: PrintLine("You should have looked before you leaped.");                   break;
    case 1: PrintLine("In the movies, your life would be passing before your eyes."); break;
    case 2: PrintLine("Geronimo...");                                                 break;
  }
}



void DoJump(void)
{
  int obj = 0;

  if (MatchCurWordIndex(WORD_ACROSS) || MatchCurWordIndex(WORD_FROM) || MatchCurWordIndex(WORD_IN) ||
      MatchCurWordIndex(WORD_INTO) || MatchCurWordIndex(WORD_OFF) || MatchCurWordIndex(WORD_OVER))
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;

    if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
      {PrintLine("That isn't visible here!"); return;}
    else if (obj == FOBJ_AMB)
      {PrintLine("You need to be more specific."); return;}
    else if (obj == OBJ_YOU)
      {PrintLine("Seriously?!"); return;}
  }

  if (obj == 0 || obj >= NUM_OBJECTS)
  {
    switch (Obj[OBJ_YOU].loc)
    {
      case ROOM_KITCHEN:
      case ROOM_EAST_OF_CHASM:
      case ROOM_RESERVOIR:
      case ROOM_CHASM_ROOM:
      case ROOM_DOME_ROOM:
      case ROOM_SOUTH_TEMPLE:
      case ROOM_ARAGAIN_FALLS:
      case ROOM_SHAFT_ROOM:
        PrintLine("This was not a very safe place to try jumping.");
        PrintRandomJumpDeath();
        YoureDead(); // ##### RIP #####
      break;

      case ROOM_UP_A_TREE:
        PrintLine("In a feat of unaccustomed daring, you manage to land on your feet without killing yourself.\n\n");
        GoToRoutine(ROOM_PATH);
      break;

      default:
        PrintRandomFun();
      break;
    }
  }
  else if (Obj[obj].loc == Obj[OBJ_YOU].loc)
  {
    if (Obj[obj].prop & PROP_ACTOR)
      PrintLine("It is too big to jump over.");
    else
      PrintRandomFun();
  }
  else
    PrintLine("That would be a good trick.");
}



void DoSleep(void)
{
  PrintLine("There's nothing to sleep on.");
}



void DoDisembark(void)
{
  if (YouAreInBoat == 0)
    PrintLine("You're not aboard anything!");
  else
    DoMisc_exit_inflated_boat();
}



void BoatGoToRoutine(int newroom)
{
  int prev_darkness;

  if ((Room[newroom].prop & R_BODYOFWATER) == 0)
    PrintLine("The magic boat comes to a rest on the shore.\n\n");

  Obj[OBJ_INFLATED_BOAT].loc = newroom;

  prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_YOU].loc = newroom;
  TimePassed = 1;

  if (IsPlayerInDarkness())
  {
    if (prev_darkness)
    {
      //kill player that tried to go from dark to dark
      PrintLine("\n\n\n\n\nOh, no! You have walked into the slavering fangs of a lurking grue!");
      YoureDead(); // ##### RIP #####
      return;
    }
    else PrintLine("You have moved into a dark place.");
  }

  PrintPlayerRoomDesc(0);
}



void DoLaunch(void)
{
  int i;
  int launch_from[8] = {ROOM_DAM_BASE, ROOM_WHITE_CLIFFS_NORTH, ROOM_WHITE_CLIFFS_SOUTH, ROOM_SHORE,
                        ROOM_SANDY_BEACH, ROOM_RESERVOIR_SOUTH, ROOM_RESERVOIR_NORTH, ROOM_STREAM_VIEW};
  int   launch_to[8] = {ROOM_RIVER_1, ROOM_RIVER_3, ROOM_RIVER_4, ROOM_RIVER_5, ROOM_RIVER_4, ROOM_RESERVOIR,
                        ROOM_RESERVOIR, ROOM_IN_STREAM};

  if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
  {
    PrintText("You are on the ");
    if (Obj[OBJ_YOU].loc == ROOM_RESERVOIR)
      PrintText("reservoir");
    else if (Obj[OBJ_YOU].loc == ROOM_IN_STREAM)
      PrintText("stream");
    else
      PrintText("river");
    PrintLine(", or have you forgotten?");
    return;
  }

  if (YouAreInBoat == 0) {PrintLine("You're not in the boat!"); return;}

  for (i=0; i<8; i++)
    if (Obj[OBJ_YOU].loc == launch_from[i]) break;
  if (i == 8) {PrintLine("You can't launch it here."); return;}

  DownstreamCounter = -1; // start at -1 to account for this turn
  BoatGoToRoutine(launch_to[i]);
}



void DoLand(void)
{
  if ((Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER) == 0)
    {PrintLine("You're not on the water!"); return;}

  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_RESERVOIR : PrintLine("You can land either to the north or the south."); break;
    case ROOM_RIVER_2   : PrintLine("There is no safe landing spot here.");            break;
    case ROOM_RIVER_4   : PrintLine("You can land either to the east or the west.");   break;

    case ROOM_IN_STREAM : BoatGoToRoutine(ROOM_STREAM_VIEW       ); break;
    case ROOM_RIVER_1   : BoatGoToRoutine(ROOM_DAM_BASE          ); break;
    case ROOM_RIVER_3   : BoatGoToRoutine(ROOM_WHITE_CLIFFS_NORTH); break;
    case ROOM_RIVER_5   : BoatGoToRoutine(ROOM_SHORE             ); break;

    default: PrintLine("You're not on the water!"); break;
  }
}



void DoEcho(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LOUD_ROOM &&
      LoudRoomQuiet == 0 &&
      (GatesOpen || LowTide == 0))
  {
    LoudRoomQuiet = 1;
    Obj[OBJ_BAR].prop &= ~PROP_SACRED;
    PrintLine("The acoustics of the room change subtly.");
    TimePassed = 1;
  }
  else
    PrintLine("Echo echo...");
}



void DoPray(void)
{
  TimePassed = 1;

  if (Obj[OBJ_YOU].loc == ROOM_SOUTH_TEMPLE)
  {
    if (YouAreDead)
    {
      PrintLine("From the distance the sound of a lone trumpet is heard. The room becomes very bright and you feel disembodied. In a moment, the brightness fades and you find yourself rising as if from a long sleep, deep in the woods. In the distance you can faintly hear a songbird and the sounds of the forest.\n\n");

      YouAreDead = 0;
      if (Obj[OBJ_TROLL].loc == ROOM_TROLL_ROOM)
        TrollAllowsPassage = 0;
      Obj[OBJ_LAMP].prop &= ~PROP_INVISIBLE;
      Obj[OBJ_YOU].prop &= ~PROP_LIT;
    }
    else
      YouAreInBoat = 0; // in case you're in it

    ExitFound = 1;
    GoToRoutine(ROOM_FOREST_1);
  }
  else
  {
    if (YouAreDead)
      PrintLine("Your prayers are not heard.");
    else
      PrintLine("If you pray enough, your prayers may be answered.");
  }
}



void DoVersion(void)
{
  PrintText("Compiled on ");
  PrintText(__DATE__);
  PrintText(" @ ");
  PrintLine(__TIME__);
}



void DoDiagnose(void)
{
  int wounds, count, death_dist = PlayerFightStrength(0) + PlayerStrength;

  if (EnableCureRoutine == 0) wounds = 0;
  else                        wounds = -PlayerStrength;

  if (wounds == 0)
    PrintLine("You are in perfect health.");
  else
  {
    PrintText("You have ");
         if (wounds == 1) PrintText("a light wound");
    else if (wounds == 2) PrintText("a serious wound");
    else if (wounds == 3) PrintText("several wounds");
    else                  PrintText("serious wounds");
    PrintText(", which will be cured after ");
    count = CURE_WAIT * (wounds - 1) + EnableCureRoutine;
    PrintInteger(count);
    if (count == 1) PrintLine(" move.");
    else            PrintLine(" moves.");
  }

  PrintText("You can ");
       if (death_dist == 0) PrintLine("expect death soon.");
  else if (death_dist == 1) PrintLine("be killed by one more light wound.");
  else if (death_dist == 2) PrintLine("be killed by a serious wound.");
  else if (death_dist == 3) PrintLine("survive one serious wound.");
  else                      PrintLine("survive several wounds.");

  if (NumDeaths != 0)
  {
    PrintText("You have been killed ");
    if (NumDeaths == 1) PrintLine("once.");
    else                PrintLine("twice.");
  }
}



void DoOdysseus(void)
{
  if (Obj[OBJ_YOU].loc != ROOM_CYCLOPS_ROOM || Obj[OBJ_CYCLOPS].loc == 0)
    PrintLine("Wasn't he a sailor?");
  else if (CyclopsState == 3)
    PrintLine("No use talking to him. He's fast asleep.");
  else
  {
    CyclopsState = 4;
    Obj[OBJ_CYCLOPS].loc = 0;
    PrintLine("The cyclops, hearing the name of his father's deadly nemesis, flees the room by knocking down the wall on the east of the room.");
    TimePassed = 1;
    ExitFound = 1;
  }
}



void DoSwim(void)
{
  if (Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE)
    PrintLine("Swimming isn't usually allowed in the dungeon.");
  else
    PrintLine("Go jump in a lake!");
}



void DoTempleTreasure(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_NORTH_TEMPLE)
    GoToRoutine(ROOM_TREASURE_ROOM);
  else if (Obj[OBJ_YOU].loc == ROOM_TREASURE_ROOM)
    GoToRoutine(ROOM_NORTH_TEMPLE);
  else
    PrintLine("Nothing happens.");
}



void DoIntro(void)
{
  PrintLine("Welcome to Zork I: The Great Underground Empire!\n(c) 1980 by INFOCOM, Inc.\nC port and parser (c) 2021 by Donnie Russell II\n\n\n");
}
//*****************************************************************************



//*****************************************************************************
void ActorResponse(int obj, int odysseus)
{
  switch (obj)
  {
    case OBJ_CYCLOPS:
      if (odysseus)
        DoOdysseus();
      else
        PrintLine("The cyclops prefers eating to making conversation.");
    break;

    case OBJ_GHOSTS:
      PrintLine("The spirits jeer loudly and ignore you.");
    break;

    case OBJ_BAT:
      PrintLine("    Fweep!\n    Fweep!\n    Fweep!\n    Fweep!\n    Fweep!\n    Fweep!");
    break;

    case OBJ_THIEF:
      PrintLine("The thief is a strong, silent type.");
    break;

    case OBJ_TROLL:
      PrintLine("The troll isn't much of a conversationalist.");
    break;
  }
}



int VerifyActor(int obj)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLine("Seriously?!"); return 1;}
  else if (obj == FOBJ_AMB)
    {PrintLine("You need to be more specific about who you want to talk to."); return 1;}
  else if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
    {PrintLine("Seriously?!"); return 1;}
  else if ((Obj[obj].prop & PROP_ACTOR) == 0)
    {PrintLine("Seriously?!"); return 1;}
  else if (Obj[obj].loc != Obj[OBJ_YOU].loc)
    {PrintLine("That person isn't visible here!"); return 1;}

  return 0;
}



// actor, *** until end of input

void DoCommandActor(int obj)
{
  int odysseus = 0;

  while (CurWord < NumStrWords)
  {
    if (MatchCurWordIndex(WORD_ODYSSEUS) || MatchCurWordIndex(WORD_ULYSSES))
      odysseus = 1;
    else
      CurWord++;
  }

  if (VerifyActor(obj) == 0)
    ActorResponse(obj, odysseus);
}



// talkto/ask/tell actor (about) (***)

void DoTalkTo(void)
{
  int obj, odysseus = 0;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  if (VerifyActor(obj)) return;

  while (CurWord < NumStrWords)
  {
    if (MatchCurWordIndex(WORD_THEN))
      {CurWord--; break;} // end of this turn's command; back up so "then" can be matched later
    else if (MatchCurWordIndex(WORD_ODYSSEUS) || MatchCurWordIndex(WORD_ULYSSES))
      odysseus = 1;
    else
      CurWord++;
  }

  ActorResponse(obj, odysseus);
}



// greet/hello (,) actor

void DoGreet(void)
{
  int obj, odysseus = 0;

  MatchCurWordIndex(WORD_AND); // and = ,
  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  if (VerifyActor(obj)) return;

  if (obj == OBJ_THIEF && ThiefDescType == 1) // unconscious
    PrintLine("The thief, being temporarily incapacitated, is unable to acknowledge your greeting with his usual graciousness.");
  else if (obj == OBJ_TROLL && TrollDescType == 1) // unconscious
    PrintLine("Unfortunately, the troll can't hear you.");
  else
    ActorResponse(obj, odysseus);
}



// say *** (to actor)

void DoSay(void)
{
  int obj = 0, odysseus = 0, temp;

  while (CurWord < NumStrWords)
  {
    if (MatchCurWordIndex(WORD_TO))
      {CurWord--; break;} // back up so "to" can be matched below
    else if (MatchCurWordIndex(WORD_ODYSSEUS) || MatchCurWordIndex(WORD_ULYSSES))
      odysseus = 1;
    else
      CurWord++;
  }

  if (MatchCurWordIndex(WORD_TO))
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  }

  if (obj == 0)
  {
    // look for exactly one actor in player's room who is described (thief can be invisible)
    for (temp=2; temp<NUM_OBJECTS; temp++)
      if ((Obj[temp].prop & PROP_ACTOR) &&
          (Obj[temp].prop & PROP_INVISIBLE) == 0 &&
          Obj[temp].loc == Obj[OBJ_YOU].loc)
        {if (obj == 0) obj = temp; else {obj = 0; break;}}
    if (obj == 0) // more than one or no actors
      {PrintLine("You need to specify who to talk to."); return;}
  }

  if (VerifyActor(obj) == 0)
    ActorResponse(obj, odysseus);
}
//*****************************************************************************



//*****************************************************************************

// handle things like water and boats

int ActionDirectionRoutine(int newroom)
{
  if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
  {
    //move from water to land or water

    if ((Room[newroom].prop & R_BODYOFWATER) == 0)
      PrintLine("The magic boat comes to a rest on the shore.\n\n");

    Obj[OBJ_INFLATED_BOAT].loc = newroom;

    DownstreamCounter = -1; // in case of moving to water; start at -1 to account for this turn
  }
  else
  {
    //move from land
    if (YouAreInBoat)
    {
      PrintLine("You'll have to get out of the boat first.");
      return 1;
    }
  }
  return 0;
}
//*****************************************************************************



//*****************************************************************************

// returns 0 if action not intercepted

int InterceptActionWhenDead(int action)
{
  if (YouAreDead == 0)
    return 0;

  if (action == A_GO || (action >= A_NORTH && action <= A_OUT))
    return 0;

  switch (action)
  {
    case A_QUIT: case A_RESTART: case A_RESTORE: case A_SAVE:
    case A_BRIEF: case A_VERBOSE: case A_SUPERBRIEF: case A_VERSION:
    case A_PRAY:
      return 0;

    // A_BURN
    case A_OPEN: case A_CLOSE: case A_EAT: case A_DRINK: case A_INFLATE: case A_DEFLATE:
    case A_TURN: case A_TIE: case A_UNTIE: case A_TOUCH:
      PrintLine("Even such an action is beyond your capabilities.");
    break;

    case A_SCORE:
      PrintLine("You're dead! How can you think of your score?");
    break;

    case A_DIAGNOSE:
      PrintLine("You are dead.");
    break;

    case A_WAIT:
      PrintLine("Might as well. You've got an eternity.");
    break;

    case A_ACTIVATE:
      PrintLine("You need no light to guide you.");
    break;

    case A_TAKE:
      PrintLine("Your hand passes through its object.");
    break;

    case A_BREAK:
      PrintLine("All such attacks are vain in your condition.");
    break;

    // A_THROW
    case A_DROP: case A_INVENTORY:
      PrintLine("You have no possessions.");
    break;

    case A_LOOK:
      PrintPlayerRoomDesc(1);
      PrintText("The room looks strange and unearthly");
      if (GetNumObjectsInLocation(Obj[OBJ_YOU].loc) == 0)
        PrintLine(".");
      else
        PrintLine(" and objects appear indistinct.");
      if ((Room[Obj[OBJ_YOU].loc].prop & R_LIT) == 0)
        PrintLine("Although there is no light, the room seems dimly illuminated.");
    break;

    default:
      PrintLine("You can't even do that.");
    break;
  }

  return 1;
}



void PrintEcho(int capital_flag)
{
  int i;
  unsigned int w;

  for (i=0,w=0; i<NumStrWords; i++)
  {
    w = StrWord[i];
    if (w != 0 && w != WORD_AND && w != WORD_THEN)
      break;
  }

  if (w == 0)
    PrintText("...");
  else
    PrintWord(w, capital_flag);
}



// returns 0 if action not intercepted

int InterceptActionInLoudRoom(int action)
{
  if (Obj[OBJ_YOU].loc != ROOM_LOUD_ROOM)
    return 0;

  if (LoudRoomQuiet || (GatesOpen == 0 && LowTide))
    return 0; // room not loud

  if ((action >= A_NORTH && action <= A_OUT) || action == A_GO ||
      action == A_SAVE || action == A_RESTORE || action == A_QUIT ||
      action == A_ECHO)
    return 0; // let these commands through

  PrintEcho(1);
  PrintChar(' ');
  PrintEcho(0);
  PrintLine("...");

  return 1;
}



int InterceptAction(int action)
{
  if (InterceptActionWhenDead(action))   return 1;
  if (InterceptActionInLoudRoom(action)) return 1;

  return 0;
}
//*****************************************************************************



//*****************************************************************************

// returns 0 if take should go ahead

int InterceptTakeObj(int obj)
{
  switch (obj)
  {
    case OBJ_BAT:           PrintLine("You can't reach him; he's on the ceiling."); return 1;
    case OBJ_CYCLOPS:       PrintLine("The cyclops doesn't take kindly to being grabbed."); TimePassed = 1; return 1;
    case OBJ_THIEF:         PrintLine("Once you got him, what would you do with him?"); return 1;
    case OBJ_TROLL:         PrintLine("The troll spits in your face, grunting \"Better luck next time\" in a rather barbarous accent."); TimePassed = 1; return 1;
    case OBJ_MACHINE:       PrintLine("It is far too large to carry."); return 1;
    case OBJ_TROPHY_CASE:   PrintLine("The trophy case is securely fastened to the wall."); return 1;
    case OBJ_MAILBOX:       PrintLine("It is securely anchored."); return 1;
    case OBJ_KITCHEN_TABLE: PrintLine("You can't take the table."); return 1;
    case OBJ_ATTIC_TABLE:   PrintLine("You can't take the table."); return 1;
    case OBJ_HOT_BELL:      PrintLine("The bell is very hot and cannot be taken."); return 1;

    case OBJ_WATER:
      if ((Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE) == 0 &&
           !(IsObjVisible(OBJ_BOTTLE) &&
             (Obj[OBJ_BOTTLE].prop & PROP_OPEN) &&
             Obj[OBJ_WATER].loc == INSIDE + OBJ_BOTTLE))
        PrintLine("There's no water here!");
      else
        PrintLine("The water slips through your fingers.");
      return 1;

    case OBJ_TOOL_CHEST:
      PrintLine("The chests are so rusty and corroded that they crumble when you touch them.");
      Obj[OBJ_TOOL_CHEST].loc = 0;
      return 1;

    case OBJ_ROPE:
      if (RopeTiedToRail)
        {PrintLine("The rope is tied to the railing."); return 1;}
    break;

    case OBJ_RUSTY_KNIFE:
      if (Obj[OBJ_SWORD].loc == INSIDE + OBJ_YOU)
        PrintLine("As you touch the rusty knife, your sword gives a single pulse of blinding blue light.");
    break;

    case OBJ_CHALICE:
      if (Obj[OBJ_CHALICE].loc == ROOM_TREASURE_ROOM &&
          Obj[OBJ_THIEF].loc == ROOM_TREASURE_ROOM &&
          (Obj[OBJ_THIEF].prop & PROP_INVISIBLE) == 0 &&
          VillainAttacking[VILLAIN_THIEF] &&
          ThiefDescType != 1) // not unconscious
        {PrintLine("You'd be stabbed in the back first."); return 1;}
    break;

    case OBJ_LARGE_BAG:
      if (ThiefDescType == 1) // unconscious
        PrintLine("Sadly for you, the robber collapsed on top of the bag. Trying to take it would wake him.");
      else
        PrintLine("The bag will be taken over his dead body.");
      return 1;
  }

  return 0;
}



// if player is inside vehicle, return vehicle obj; otherwise return 0

int GetPlayersVehicle(void)
{
  if (YouAreInBoat)
    return OBJ_INFLATED_BOAT;
  else
    return 0;
}



void MoveTreasuresToLandOfLivingDead(int loc)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == loc &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0 &&
        (Obj[obj].prop & PROP_SACRED) == 0 &&
        Obj[obj].thiefvalue > 0)
  {
    Obj[obj].loc = ROOM_LAND_OF_LIVING_DEAD;
    Obj[obj].prop |= PROP_MOVEDDESC;
  }
}



// returns 1 if intercepted

int InterceptTakeFixedObj(int obj)
{
  switch (obj)
  {
    case FOBJ_BOARD:        PrintLine("The boards are securely fastened.");                          return 1;
    case FOBJ_SONGBIRD:     PrintLine("The songbird is not here but is probably nearby.");           return 1;
    case FOBJ_BODIES:       PrintLine("A force keeps you from taking the bodies.");                  return 1;
    case FOBJ_RUG:          PrintLine("The rug is extremely heavy and cannot be carried.");          return 1;
    case FOBJ_NAILS:        PrintLine("The nails, deeply imbedded in the door, cannot be removed."); return 1;
    case FOBJ_GRANITE_WALL: PrintLine("It's solid granite.");                                        return 1;
    case FOBJ_CHAIN:        PrintLine("The chain is secure.");                                       return 1;

    case FOBJ_BOLT:
    case FOBJ_BUBBLE:
      PrintLine("It is an integral part of the control panel.");
      return 1;

    case FOBJ_MIRROR2:
    case FOBJ_MIRROR1:
      PrintLine("The mirror is many times your size. Give up.");
      return 1;

    case FOBJ_BONES:
      PrintLine("A ghost appears in the room and is appalled at your desecration of the remains of a fellow adventurer. He casts a curse on your valuables and banishes them to the Land of the Living Dead. The ghost leaves, muttering obscenities.");
      MoveTreasuresToLandOfLivingDead(Obj[OBJ_YOU].loc);
      MoveTreasuresToLandOfLivingDead(INSIDE + OBJ_YOU);
      return 1;
  }

  return 0;
}



int InterceptTakeOutOf(int container)
{
  switch (container)
  {
    case OBJ_LARGE_BAG:
    {
      PrintLine("It would be a good trick.");
      return 1;
    }
  }

  return 0;
}



// test flag:  1 if no changes should be made (yet)
// multi flag: 1 if obj name should be printed

// returns:
//   1:  intercepted, and obj MUST leave inventory, unless container is full
//  -1:  intercepted and calling function should immediately return

int InterceptDropPutObj(int obj, int container, int test, int multi)
{
  switch (container)
  {
    case OBJ_LOWERED_BASKET:
    {
      if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
      PrintLine("The basket is at the other end of the shaft.");
      return -1;
    }

    case OBJ_CHALICE:
    {
      if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
      PrintLine("You can't. It's not a very good chalice, is it?");
      return -1;
    }

    case OBJ_LARGE_BAG:
    {
      if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
      PrintLine("It would be a good trick.");
      return -1;
    }

    case FOBJ_GRATE:
    {
      if (GetObjectSize(obj) > 20)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        PrintLine("It won't fit through the grating.");
        return -1;
      }
      else if (Obj[OBJ_YOU].loc != ROOM_GRATING_CLEARING)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        PrintLine("It won't go through.");
        return -1;
      }

      if (test == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        PrintLine("It goes through the grating into the darkness below.");

        Obj[obj].loc = ROOM_GRATING_ROOM;
        MoveObjOrderToLast(obj);
        TimePassed = 1;
      }

      return 1;
    }

    case FOBJ_SLIDE:
    {
      if (test == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        if (Obj[OBJ_YOU].loc == ROOM_SLIDE_ROOM)
          PrintLine("It falls into the slide and is gone.");
        else
          PrintLine("It falls into the slide.");

        Obj[obj].loc = ROOM_CELLAR;
        MoveObjOrderToLast(obj);
        TimePassed = 1;
      }

      return 1;
    }

    case FOBJ_RIVER:
    case OBJ_WATER:
    {
      if ((Room[Obj[OBJ_YOU].loc].prop & R_WATERHERE) == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        PrintLine("There isn't any water here!");
        return -1;
      }

      if (obj == OBJ_INFLATED_BOAT)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        PrintLine("You should get in the boat then launch it.");
        return -1;
      }

      if (test == 0)
      {
        if (multi) {PrintObjectDesc(obj, 0); PrintText(": ");}
        if (Obj[obj].prop & PROP_INFLAMMABLE)
          PrintLine("It floats for a moment, then sinks.");
        else
          PrintLine("It splashes into the water and is gone forever.");

        Obj[obj].loc = 0;
        TimePassed = 1;
      }

      return 1;
    }
  }

  if (container >= NUM_OBJECTS)
  {
    PrintLine("You can't put anything into that!");
    return -1;
  }

  return 0; // not intercepted
}

//*****************************************************************************



//*****************************************************************************
int IsActorInRoom(int room)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == room &&
        (Obj[obj].prop & PROP_ACTOR) &&
        (Obj[obj].prop & PROP_INVISIBLE) == 0)
    return 1;

  return 0;
}



// thiefvalue for sword indicates glow level

void SwordRoutine(void)
{
  int glow, new_glow, i, room;

  if (Obj[OBJ_SWORD].loc != INSIDE + OBJ_YOU) return;

  glow = Obj[OBJ_SWORD].thiefvalue;
  new_glow = 0;

  if (IsActorInRoom(Obj[OBJ_YOU].loc))
    new_glow = 2;
  else
    for (i=0; i<10; i++)
  {
    room = GetRoomPassage(Obj[OBJ_YOU].loc, i);
    if (room > 0 && room < NUM_ROOMS && IsActorInRoom(room))
    {
      new_glow = 1;
      break;
    }
  }

  if (new_glow != glow)
  {
         if (new_glow == 0) PrintLine("Your sword is no longer glowing.");
    else if (new_glow == 1) PrintLine("Your sword is glowing with a faint blue glow.");
    else                    PrintLine("Your sword has begun to glow very brightly.");
    Obj[OBJ_SWORD].thiefvalue = new_glow;
  }
}



void LampDrainRoutine(void)
{
  if (Obj[OBJ_LAMP].loc == 0) return; // destroyed by machine or lost

  if ((Obj[OBJ_LAMP].prop & PROP_LIT) == 0) return;

  if (LampTurnsLeft > 0) LampTurnsLeft--;

  if (IsObjVisible(OBJ_LAMP))
    switch (LampTurnsLeft)
  {
    case 100: PrintLine("The lamp appears a bit dimmer.");     break;
    case  70: PrintLine("The lamp is definitely dimmer now."); break;
    case  15: PrintLine("The lamp is nearly out.");            break;
  }

  if (LampTurnsLeft == 0)
  {
    int prev_darkness;

    prev_darkness = IsPlayerInDarkness();
    Obj[OBJ_LAMP].prop &= ~PROP_LIT;
    if (IsPlayerInDarkness() != prev_darkness)
    {
      PrintNewLine();
      PrintPlayerRoomDesc(1);
    }
  }
}



// also handles:
//   candles in ceremony
//   candles put out by draft
//   items destroyed when candle is put inside them

void CandlesShrinkRoutine(void)
{
  int prev_darkness;

  if (Obj[OBJ_CANDLES].loc == 0) return; // destroyed by machine or lost

  if ((Obj[OBJ_CANDLES].prop & PROP_MOVEDDESC) == 0) return; // still sitting on altar

  if ((Obj[OBJ_CANDLES].prop & PROP_LIT) == 0) return; // not lit


  if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU &&
      Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES &&
      BellRungCountdown > 0 &&
      CandlesLitCountdown == 0)
  {
    PrintLine("The flames flicker wildly and appear to dance. The earth beneath your feet trembles, and your legs nearly buckle beneath you. The spirits cower at your unearthly power.");
    BellRungCountdown = 0;
    CandlesLitCountdown = 3;
  }


  if (CandleTurnsLeft > 0) CandleTurnsLeft--;

  if (IsObjVisible(OBJ_CANDLES))
    switch (CandleTurnsLeft)
  {
    case 20: PrintLine("The candles grow shorter.");                           break;
    case 10: PrintLine("The candles are becoming quite short.");               break;
    case  5: PrintLine("The candles won't last long now.");                    break;
    case  0: PrintLine("You'd better have more light than from the candles."); break;
  }


  prev_darkness = IsPlayerInDarkness();


  if (CandleTurnsLeft == 0)
    Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
  else if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_INFLATED_BOAT)
  {
    if (IsObjVisible(OBJ_INFLATED_BOAT))
      PrintLine("The boat ignites and it, any contents it may have had, and the candles are destroyed.");
    Obj[OBJ_INFLATED_BOAT].loc = 0;
    Obj[OBJ_CANDLES].loc = 0;
  }
  else if (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_NEST)
  {
    if (IsObjVisible(OBJ_NEST))
      PrintLine("The nest ignites and it, any contents it may have had, and the candles are destroyed.");
    Obj[OBJ_NEST].loc = 0;
    Obj[OBJ_CANDLES].loc = 0;
  }
  else if (Obj[OBJ_YOU].loc == ROOM_TINY_CAVE && PercentChance(50, 80))
  {
    if (IsObjVisible(OBJ_CANDLES)) PrintLine("A gust of wind blows out your candles!");
    Obj[OBJ_CANDLES].prop &= ~PROP_LIT;
  }


  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintNewLine();
    PrintPlayerRoomDesc(1);
  }
}



void ReservoirFillRoutine(void)
{
  if (ReservoirFillCountdown == 0) return;
      ReservoirFillCountdown--;
  if (ReservoirFillCountdown > 0) return;

  Room[ROOM_RESERVOIR  ].prop |= R_BODYOFWATER;
  Room[ROOM_DEEP_CANYON].prop &= ~R_DESCRIBED;
  Room[ROOM_LOUD_ROOM  ].prop &= ~R_DESCRIBED;

  LowTide = 0;

  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_RESERVOIR:
      if (YouAreInBoat)
        PrintLine("The boat lifts gently out of the mud and is now floating on the reservoir.");
      else
      {
        PrintLine("You are lifted up by the rising river! You try to swim, but the currents are too strong. You come closer, closer to the awesome structure of Flood Control Dam #3. The dam beckons to you. The roar of the water nearly deafens you, but you remain conscious as you tumble over the dam toward your certain doom among the rocks at its base.");
        YoureDead(); // ##### RIP #####
      }
    break;

    case ROOM_DEEP_CANYON:
      PrintLine("A sound, like that of flowing water, starts to come from below.");
    break;

    case ROOM_LOUD_ROOM:
      if (LoudRoomQuiet == 0)
      {
        int random_room[3] = {ROOM_DAMP_CAVE, ROOM_ROUND_ROOM, ROOM_DEEP_CANYON};

        PrintLine("All of a sudden, an alarmingly loud roaring sound fills the room. Filled with fear, you scramble away.\n\n");

        YouAreInBoat = 0; // in case you're in it
        GoToRoutine(random_room[GetRandom(3)]);
      }
    break;

    case ROOM_RESERVOIR_NORTH:
    case ROOM_RESERVOIR_SOUTH:
      PrintLine("You notice that the water level has risen to the point that it is impossible to cross.");
    break;
  }
}



void ReservoirDrainRoutine(void)
{
  if (ReservoirDrainCountdown == 0) return;
      ReservoirDrainCountdown--;
  if (ReservoirDrainCountdown > 0) return;

  Room[ROOM_RESERVOIR  ].prop &= ~R_BODYOFWATER;
  Room[ROOM_DEEP_CANYON].prop &= ~R_DESCRIBED;
  Room[ROOM_LOUD_ROOM  ].prop &= ~R_DESCRIBED;

  LowTide = 1;

  switch (Obj[OBJ_YOU].loc)
  {
    case ROOM_RESERVOIR:
      if (YouAreInBoat)
        PrintLine("The water level has dropped to the point at which the boat can no longer stay afloat. It sinks into the mud.");
    break;

    case ROOM_DEEP_CANYON:
      PrintLine("The roar of rushing water is quieter now.");
    break;

    case ROOM_RESERVOIR_NORTH:
    case ROOM_RESERVOIR_SOUTH:
      PrintLine("The water level is now quite low here and you could easily cross over to the other side.");
    break;
  }
}



void SinkingObjectsRoutine(void)
{
  int obj, i;
  int check_room[7] = {ROOM_RESERVOIR, ROOM_IN_STREAM, ROOM_RIVER_1, ROOM_RIVER_2, ROOM_RIVER_3,
                       ROOM_RIVER_4, ROOM_RIVER_5};

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (obj != OBJ_INFLATED_BOAT && obj != OBJ_BUOY && obj != OBJ_THIEF)
      for (i=0; i<7; i++)
        if (Obj[obj].loc == check_room[i])
  {
    if ((Room[check_room[i]].prop & R_BODYOFWATER) && (Obj[obj].prop & PROP_INVISIBLE) == 0)
    {
      // if room is filled with water and object hasn't sunk, sink object
      Obj[obj].prop |= PROP_INVISIBLE;
    }
    else if ((Room[check_room[i]].prop & R_BODYOFWATER) == 0 && (Obj[obj].prop & PROP_INVISIBLE))
    {
      // if room is not filled with water and object has sunk, unsink object
      Obj[obj].prop &= ~PROP_INVISIBLE;
    }
  }
}



void LoudRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_LOUD_ROOM && LoudRoomQuiet == 0 && GatesOpen && LowTide == 0)
  {
    int random_room[3] = {ROOM_DAMP_CAVE, ROOM_ROUND_ROOM, ROOM_DEEP_CANYON};

    PrintLine("It is unbearably loud here, with an ear-splitting roar seeming to come from all around you. There is a pounding in your head which won't stop. With a tremendous effort, you scramble out of the room.\n\n");

    YouAreInBoat = 0; // in case you're in it

    GoToRoutine(random_room[GetRandom(3)]);
  }
}



void MaintenanceLeakRoutine(void)
{
  if (MaintenanceWaterLevel <= 0 || MaintenanceWaterLevel > 16) return;

  if (Obj[OBJ_YOU].loc == ROOM_MAINTENANCE_ROOM)
  {
    PrintText("The water level here is now ");

    switch (MaintenanceWaterLevel / 2)
    {
      case 0: PrintLine("up to your ankles.");  break;
      case 1: PrintLine("up to your shin.");    break;
      case 2: PrintLine("up to your knees.");   break;
      case 3: PrintLine("up to your hips.");    break;
      case 4: PrintLine("up to your waist.");   break;
      case 5: PrintLine("up to your chest.");   break;
      case 6: PrintLine("up to your neck.");    break;
      case 7: PrintLine("over your head.");     break;
      case 8: PrintLine("high in your lungs."); break;
    }
  }

  MaintenanceWaterLevel++;
  if (MaintenanceWaterLevel > 16 && Obj[OBJ_YOU].loc == ROOM_MAINTENANCE_ROOM)
  {
    PrintLine("I'm afraid you have done drowned yourself.");
    if (YouAreInBoat)
      switch (Obj[OBJ_YOU].loc)
    {
      case ROOM_MAINTENANCE_ROOM:
      case ROOM_DAM_ROOM:
      case ROOM_DAM_LOBBY:
        PrintLine("The rising water carries the boat over the dam, down the river, and over the falls. Tsk, tsk.");
      break;
    }
    YoureDead(); // ##### RIP #####
  }
}



void BoatPuncturedRoutine(void)
{
  int i, flag;
  int pointy_obj[6] = {OBJ_SCEPTRE, OBJ_KNIFE, OBJ_SWORD, OBJ_RUSTY_KNIFE, OBJ_AXE, OBJ_STILETTO};

  flag = 0;
  for (i=0; i<6; i++)
    if (Obj[pointy_obj[i]].loc == INSIDE + OBJ_INFLATED_BOAT)
  {
    flag = 1;
    Obj[pointy_obj[i]].loc = Obj[OBJ_INFLATED_BOAT].loc;
  }
  if (flag == 0) return;

  PrintLine("It seems that something pointy didn't agree with the boat, as evidenced by the loud hissing noise issuing therefrom. With a pathetic sputter, the boat deflates, leaving you without.");

  Obj[OBJ_PUNCTURED_BOAT].loc = Obj[OBJ_INFLATED_BOAT].loc;
  Obj[OBJ_INFLATED_BOAT].loc = 0;

  if (YouAreInBoat) YouAreInBoat = 0;

  if (Room[Obj[OBJ_YOU].loc].prop & R_BODYOFWATER)
  {
    if (Obj[OBJ_YOU].loc == ROOM_RESERVOIR || Obj[OBJ_YOU].loc == ROOM_IN_STREAM)
      PrintLine("Another pathetic sputter, this time from you, heralds your drowning.");
    else
      PrintLine("In other words, fighting the fierce currents of the Frigid River. You manage to hold your own for a bit, but then you are carried over a waterfall and into some nasty rocks. Ouch!");
    YoureDead(); // ##### RIP #####
  }
}



void BuoyRoutine(void)
{
  if (BuoyFlag == 0 && Obj[OBJ_BUOY].loc == INSIDE + OBJ_YOU)
  {
    BuoyFlag = 1;
    PrintLine("You notice something funny about the feel of the buoy.");
  }
}



void DownstreamRoutine(void)
{
  int i;
  int  float_from[5] = {ROOM_RIVER_1, ROOM_RIVER_2, ROOM_RIVER_3, ROOM_RIVER_4, ROOM_RIVER_5};
  int    float_to[5] = {ROOM_RIVER_2, ROOM_RIVER_3, ROOM_RIVER_4, ROOM_RIVER_5, 0};
  int float_speed[5] = {4, 4, 3, 2, 1};

  for (i=0; i<5; i++)
    if (Obj[OBJ_YOU].loc == float_from[i]) break;
  if (i == 5) return;

  DownstreamCounter++;
  if (DownstreamCounter < float_speed[i]) return;

  if (float_to[i] == 0)
  {
    PrintLine("Unfortunately, the magic boat doesn't provide protection from the rocks and boulders one meets at the bottom of waterfalls. Including this one.");
    YoureDead(); // ##### RIP #####
    return;
  }

  PrintLine("The flow of the river carries you downstream.\n\n");

  DownstreamCounter = 0;
  BoatGoToRoutine(float_to[i]);
}



void BatRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_BAT_ROOM && IsObjVisible(OBJ_GARLIC) == 0)
  {
    int random_room[8] = {ROOM_MINE_1, ROOM_MINE_2, ROOM_MINE_3, ROOM_MINE_4, ROOM_LADDER_TOP,
                          ROOM_LADDER_BOTTOM, ROOM_SQUEEKY_ROOM, ROOM_MINE_ENTRANCE};

    PrintLine("    Fweep!\n    Fweep!\n    Fweep!\nThe bat grabs you by the scruff of your neck and lifts you away....\n\n");

    GoToRoutine(random_room[GetRandom(8)]);
  }
}



void LeavesTakenRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_CLEARING &&
      Obj[OBJ_LEAVES].loc != ROOM_GRATING_CLEARING &&
      GratingRevealed == 0)
  {
    GratingRevealed = 1;
    PrintLine("With the leaves moved, a grating is revealed.");
  }

  // also reveal grating just by being in grating room
  if (Obj[OBJ_YOU].loc == ROOM_GRATING_ROOM)
    GratingRevealed = 1;
}



// must call before match routine

void GasRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_GAS_ROOM)
  {
    int match   = (Obj[OBJ_MATCH  ].loc == INSIDE + OBJ_YOU && (Obj[OBJ_MATCH  ].prop & PROP_LIT));
    int candles = (Obj[OBJ_CANDLES].loc == INSIDE + OBJ_YOU && (Obj[OBJ_CANDLES].prop & PROP_LIT));
    int torch   = (Obj[OBJ_TORCH  ].loc == INSIDE + OBJ_YOU && (Obj[OBJ_TORCH  ].prop & PROP_LIT));
    int type = 0; // 1: lighted  2: carried

    if (match && MatchTurnsLeft == 2)
      type = 1;
    else if (match || candles || torch)
      type = 2;

    if (type)
    {
      if (type == 1)
        PrintLine("How sad for an aspiring adventurer to light a match in a room which reeks of gas. Fortunately, there is justice in the world.");
      else
        PrintLine("Oh dear. It appears that the smell coming from this room was coal gas. I would have thought twice about carrying flaming objects in here.");

      PrintLine("\n      ** BOOOOOOOOOOOM **");

      YoureDead(); // ##### RIP #####
    }
  }
}



void MatchRoutine(void)
{
  if (MatchTurnsLeft == 0) return;

  MatchTurnsLeft--;
  if (MatchTurnsLeft == 0)
  {
    int prev_darkness;

    if (IsObjVisible(OBJ_MATCH))
      PrintLine("The match has gone out.");

    prev_darkness = IsPlayerInDarkness();
    Obj[OBJ_MATCH].prop &= ~PROP_LIT;
    if (IsPlayerInDarkness() != prev_darkness)
    {
      PrintNewLine();
      PrintPlayerRoomDesc(1);
    }
  }
}



void CeremonyBroken(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES)
    PrintLine("The tension of this ceremony is broken, and the wraiths, amused but shaken at your clumsy attempt, resume their hideous jeering.");
}



void BellRungRoutine(void)
{
  if (BellRungCountdown == 0) return;
      BellRungCountdown--;
  if (BellRungCountdown == 0)
    CeremonyBroken();
}



void CandlesLitRoutine(void)
{
  if (CandlesLitCountdown == 0) return;
      CandlesLitCountdown--;
  if (CandlesLitCountdown == 0)
    CeremonyBroken();
}



void BellHotRoutine(void)
{
  if (BellHotCountdown == 0) return;
      BellHotCountdown--;
  if (BellHotCountdown == 0)
  {
    if (Obj[OBJ_YOU].loc == ROOM_ENTRANCE_TO_HADES)
      PrintLine("The bell appears to have cooled down.");

    Obj[OBJ_BELL].loc = ROOM_ENTRANCE_TO_HADES;
    Obj[OBJ_HOT_BELL].loc = 0;
  }
}



void HoldingGunkRoutine(void)
{
  if (Obj[OBJ_GUNK].loc == INSIDE + OBJ_YOU)
  {
    Obj[OBJ_GUNK].loc = 0;
    PrintLine("The slag was rather insubstantial, and crumbles into dust at your touch.");
  }
}



void InRoomOnRainbowRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_ON_RAINBOW)
    ExitFound = 1;
}



void DomeRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc == ROOM_DOME_ROOM && YouAreDead)
  {
    PrintLine("As you enter the dome you feel a strong pull as if from a wind drawing you over the railing and down.\n\n");

    GoToRoutine(ROOM_TORCH_ROOM);
  }
}



void UpATreeRoutine(void)
{
  int obj, other_fell = 0, count = 0;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == ROOM_UP_A_TREE)
  {
    if (obj == OBJ_NEST)
    {
      if (Obj[obj].prop & PROP_MOVEDDESC)
      {
        count++;
        Obj[obj].loc = ROOM_PATH;
        if (Obj[OBJ_EGG].loc == INSIDE + OBJ_NEST)
        {
          other_fell = 1;
          Obj[OBJ_EGG].loc = 0;
          Obj[OBJ_BROKEN_EGG].loc = ROOM_PATH;
        }
      }
    }
    else if (obj == OBJ_EGG)
    {
      other_fell = 2;
      count++;
      Obj[OBJ_EGG].loc = 0;
      Obj[OBJ_BROKEN_EGG].loc = ROOM_PATH;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPENABLE;
      Obj[OBJ_BROKEN_EGG].prop |= PROP_OPEN;
    }
    else
    {
      count++;
      Obj[obj].loc = ROOM_PATH;
    }
  }

  if (count == 1 && other_fell == 0)
    PrintLine("It falls to the ground.");
  else if (count > 1)
    PrintLine("They fall to the ground.");

  if (other_fell == 1)
    PrintLine("The nest falls to the ground, and the egg spills out of it, seriously damaged.");
  else if (other_fell == 2)
    PrintLine("The egg falls to the ground and springs open, seriously damaged.");
}



void SongbirdRoutine(void)
{
  if (AreYouInForest() && PercentChance(15, -1))
    PrintLine("You hear in the distance the chirping of a song bird.");
}



void WaterSpilledRoutine(void)
{
  if (Obj[OBJ_WATER].loc == Obj[OBJ_YOU].loc)
  {
    Obj[OBJ_WATER].loc = 0;
    PrintLine("The water spills to the floor and evaporates.");
  }
}



void CyclopsRoomRoutine(void)
{
  if (Obj[OBJ_YOU].loc != ROOM_CYCLOPS_ROOM)
    {CyclopsCounter = 0; return;}

  if (CyclopsState >= 3 ||                  // asleep or fled
      VillainAttacking[VILLAIN_CYCLOPS] ||  // attacking
      Obj[OBJ_CYCLOPS].loc == 0)            // dead
    return;

  CyclopsCounter++;

  if (CyclopsState >= 1) // hungry or thirsty
  {
    switch (CyclopsCounter - 1)
    {
      case 0: PrintLine("The cyclops seems somewhat agitated."); break;
      case 1: PrintLine("The cyclops appears to be getting more agitated."); break;
      case 2: PrintLine("The cyclops is moving about the room, looking for something."); break;
      case 3: PrintLine("The cyclops was looking for salt and pepper. No doubt they are condiments for his upcoming snack."); break;
      case 4: PrintLine("The cyclops is moving toward you in an unfriendly manner."); break;
      case 5: PrintLine("You have two choices: 1. Leave  2. Become dinner."); break;
      case 6: PrintLine("The cyclops, tired of all of your games and trickery, grabs you firmly. As he licks his chops, he says \"Mmm. Just like Mom used to make 'em.\" It's nice to be appreciated."); break;
    }

    if (CyclopsCounter == 7)
      YoureDead(); // ##### RIP #####
  }
  else if (CyclopsCounter == 5)
  {
    CyclopsCounter = 0;
    CyclopsState = 1; // hungry
  }
}

//-----------------------------------------------------------------------------

void ScoreObj(int obj, unsigned char takeval, unsigned char caseval)
{
  if (Obj[obj].loc == INSIDE + OBJ_YOU && (Obj[obj].prop & PROP_SCOREDTAKE) == 0)
  {
    Obj[obj].prop |= PROP_SCOREDTAKE;
    Score += takeval;
  }
  else if (Obj[obj].loc == INSIDE + OBJ_TROPHY_CASE && (Obj[obj].prop & PROP_SCOREDCASE) == 0)
  {
    Obj[obj].prop |= PROP_SCOREDCASE;
    Score += caseval;
  }
}



void ScoreRoom(int room, unsigned char visitval)
{
  if (Obj[OBJ_YOU].loc == room && (Room[room].prop & R_SCOREDVISIT) == 0)
  {
    Room[room].prop |= R_SCOREDVISIT;
    Score += visitval;
  }
}



void ScoreUpdateRoutine(void)
{
  int old_score = Score;

  ScoreObj(OBJ_EGG          ,  5,  5);
  ScoreObj(OBJ_CANARY       ,  6,  4);
  ScoreObj(OBJ_PAINTING     ,  4,  6);
  ScoreObj(OBJ_BAR          , 10,  5);
  ScoreObj(OBJ_TORCH        , 14,  6);
  ScoreObj(OBJ_COFFIN       , 10, 15);
  ScoreObj(OBJ_SCEPTRE      ,  4,  6);
  ScoreObj(OBJ_TRUNK        , 15,  5);
  ScoreObj(OBJ_TRIDENT      ,  4, 11);
  ScoreObj(OBJ_JADE         ,  5,  5);
  ScoreObj(OBJ_BRACELET     ,  5,  5);
  ScoreObj(OBJ_DIAMOND      , 10, 10);
  ScoreObj(OBJ_BAG_OF_COINS , 10,  5);
  ScoreObj(OBJ_SKULL        , 10, 10);
  ScoreObj(OBJ_SCARAB       ,  5,  5);
  ScoreObj(OBJ_EMERALD      ,  5, 10);
  ScoreObj(OBJ_CHALICE      , 10,  5);
  ScoreObj(OBJ_POT_OF_GOLD  , 10, 10);
  ScoreObj(OBJ_BAUBLE       ,  1,  1);

  ScoreRoom(ROOM_KITCHEN       , 10);
  ScoreRoom(ROOM_CELLAR        , 25);
  ScoreRoom(ROOM_EW_PASSAGE    ,  5);
  ScoreRoom(ROOM_LOWER_SHAFT   , 13);
  ScoreRoom(ROOM_TREASURE_ROOM , 25);

  if (Score - old_score > 0)
  {
    // PrintText("[Your score just went up by ");
    // PrintInteger(Score - old_score);
    // if (Score - old_score == 1) PrintLine(" point.]");
    // else                        PrintLine(" points.]");
  }

  if (Score == SCORE_MAX && WonGame == 0)
  {
    WonGame = 1;
    Obj[OBJ_MAP].prop &= ~PROP_INVISIBLE;
    Room[ROOM_WEST_OF_HOUSE].prop &= ~R_DESCRIBED;
    PrintLine("An almost inaudible voice whispers in your ear, \"Look to your treasures for the final secret.\"");
  }
}

//-----------------------------------------------------------------------------

//run event routines after each action that set time-passed flag
void RunEventRoutines(void)
{
  SwordRoutine();
  LampDrainRoutine();
  CandlesShrinkRoutine();
  ReservoirFillRoutine();
  ReservoirDrainRoutine();
  SinkingObjectsRoutine(); // must be called after reservoir fill/drain routines
  LoudRoomRoutine();
  MaintenanceLeakRoutine();
  BoatPuncturedRoutine();
  BuoyRoutine(); // should be called before downstream routine because of message order
  DownstreamRoutine();
  BatRoomRoutine();
  LeavesTakenRoutine();
  GasRoomRoutine(); // must be called before match routine
  MatchRoutine();
  BellRungRoutine();
  CandlesLitRoutine();
  BellHotRoutine();
  HoldingGunkRoutine();
  InRoomOnRainbowRoutine();
  DomeRoomRoutine();
  UpATreeRoutine();
  SongbirdRoutine();
  WaterSpilledRoutine();
  CyclopsRoomRoutine();
  ScoreUpdateRoutine();

  VillainsRoutine();
}
//*****************************************************************************



//*****************************************************************************
int GetScore(void)
{
  return Score;
}



int GetMaxScore(void)
{
  return SCORE_MAX;
}



void PrintRankName(void)
{
       if (Score == 350) PrintLine("Master Adventurer.");
  else if (Score >  330) PrintLine("Wizard.");
  else if (Score >  300) PrintLine("Master.");
  else if (Score >  200) PrintLine("Adventurer.");
  else if (Score >  100) PrintLine("Junior Adventurer.");
  else if (Score >   50) PrintLine("Novice Adventurer.");
  else if (Score >   25) PrintLine("Amateur Adventurer.");
  else                   PrintLine("Beginner.");
}

//*****************************************************************************



//*****************************************************************************

void ZeroMem(void *p, int n)
{
  memset(p, 0, n);
}



void InitGameState(void)
{
  int i;

  ZeroMem(&GameFlags, sizeof(GameFlags));
  ZeroMem(&GameVars , sizeof(GameVars ));

  LampTurnsLeft   = 200;
  CandleTurnsLeft = 40;
  MatchesLeft     = 6;
  LoadAllowed     = 100;

  VillainStrength[VILLAIN_TROLL  ] = 2;
  VillainStrength[VILLAIN_THIEF  ] = 5;
  VillainStrength[VILLAIN_CYCLOPS] = 10000;

  InitRoomProperties();

  InitObjectLocations();
  InitObjectProperties();

  for (i=0; i<NUM_OBJECTS; i++)
  {
    Obj[i].order = i;
    Obj[i].thiefvalue = 0;
  }

  Obj[OBJ_SKULL          ].thiefvalue = 10;
  Obj[OBJ_SCEPTRE        ].thiefvalue =  6;
  Obj[OBJ_CHALICE        ].thiefvalue =  5;
  Obj[OBJ_TRIDENT        ].thiefvalue = 11;
  Obj[OBJ_COFFIN         ].thiefvalue = 15;
  Obj[OBJ_DIAMOND        ].thiefvalue = 10;
  Obj[OBJ_JADE           ].thiefvalue =  5;
  Obj[OBJ_BAG_OF_COINS   ].thiefvalue =  5;
  Obj[OBJ_EMERALD        ].thiefvalue = 10;
  Obj[OBJ_PAINTING       ].thiefvalue =  6;
  Obj[OBJ_BAR            ].thiefvalue =  5;
  Obj[OBJ_POT_OF_GOLD    ].thiefvalue = 10;
  Obj[OBJ_BRACELET       ].thiefvalue =  5;
  Obj[OBJ_SCARAB         ].thiefvalue =  5;
  Obj[OBJ_TORCH          ].thiefvalue =  6;
  Obj[OBJ_TRUNK          ].thiefvalue =  5;
  Obj[OBJ_EGG            ].thiefvalue =  5;
  Obj[OBJ_BROKEN_EGG     ].thiefvalue =  2;
  Obj[OBJ_BAUBLE         ].thiefvalue =  1;
  Obj[OBJ_CANARY         ].thiefvalue =  4;
  Obj[OBJ_BROKEN_CANARY  ].thiefvalue =  1;

  ItObj = OBJ_MAILBOX;
}
//*****************************************************************************
