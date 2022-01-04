// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>



//#define USE_CHAR_CURSOR
//#define NO_STATUS_LINE



#ifdef __AVR_ATmega2560__

#define USE_CHAR_CURSOR

#include <avr/pgmspace.h>

#else

#define PROGMEM

#define pgm_read_byte(x)  (*(x))
#define pgm_read_word(x)  (*(x))

#endif



#define NUM_ROOMS            111  // including null room 0
#define NUM_OBJECTS          80
#define NUM_BLOCK_MESSAGES   23
#define SCORE_MAX            350
#define CURE_WAIT            30
#define LOAD_MAX             100
#define MAX_INVENTORY_ITEMS  7
#define INV_LIMIT_CHANCE     8
#define STRENGTH_MIN         2
#define STRENGTH_MAX         7
#define INSIDE               2048
#define NUMERIC              10000  // words such as "0" and "999" start at this value

enum
{
  VILLAIN_TROLL,
  VILLAIN_THIEF,
  VILLAIN_CYCLOPS,

  NUM_VILLAINS
};



struct GAMEFLAGS
{
  unsigned char b0  : 1, b1  : 1, b2  : 1, b3  : 1, b4  : 1, b5  : 1, b6  : 1, b7  : 1;
  unsigned char b8  : 1, b9  : 1, b10 : 1, b11 : 1, b12 : 1, b13 : 1, b14 : 1, b15 : 1;
  unsigned char b16 : 1, b17 : 1, b18 : 1, b19 : 1, b20 : 1, b21 : 1, b22 : 1, b23 : 1;
  unsigned char b24 : 1, b25 : 1;
};

#define RugMoved            GameFlags.b0
#define TrapOpen            GameFlags.b1
#define ExitFound           GameFlags.b2   // set when player finds exit from dungeon other than trapdoor
#define KitchenWindowOpen   GameFlags.b3
#define GratingRevealed     GameFlags.b4
#define GratingUnlocked     GameFlags.b5
#define GratingOpen         GameFlags.b6
#define GatesOpen           GameFlags.b7
#define LowTide             GameFlags.b8
#define GatesButton         GameFlags.b9
#define LoudRoomQuiet       GameFlags.b10
#define RainbowSolid        GameFlags.b11
#define WonGame             GameFlags.b12
#define MirrorBroken        GameFlags.b13  // set NotLucky too
#define RopeTiedToRail      GameFlags.b14
#define SpiritsBanished     GameFlags.b15
#define TrollAllowsPassage  GameFlags.b16
#define YouAreSanta         GameFlags.b17
#define YouAreInBoat        GameFlags.b18
#define NotLucky            GameFlags.b19
#define YouAreDead          GameFlags.b20
#define SongbirdSang        GameFlags.b21
#define ThiefHere           GameFlags.b22
#define ThiefEngrossed      GameFlags.b23
#define YouAreStaggered     GameFlags.b24
#define BuoyFlag            GameFlags.b25



struct GAMEVARS
{
  unsigned char LampTurnsLeft;
  unsigned char MatchTurnsLeft;
  unsigned char CandleTurnsLeft;
  unsigned char MatchesLeft;
  unsigned char ReservoirFillCountdown;
  unsigned char ReservoirDrainCountdown;
  unsigned char BellRungCountdown;        // these three are for ceremony
  unsigned char CandlesLitCountdown;
  unsigned char BellHotCountdown;
  unsigned char CaveHoleDepth;
  unsigned char NumDeaths;
  unsigned char CyclopsCounter;
  unsigned char CyclopsState;             // 0: default  1: hungry  2: thirsty  3: asleep  4: fled
  unsigned char LoadAllowed;
  unsigned char TrollDescType;
  unsigned char ThiefDescType;            // 0: default  1: unconscious
  unsigned char EnableCureRoutine;        // countdown
  unsigned char VillainAttacking[NUM_VILLAINS];
  unsigned char VillainStaggered[NUM_VILLAINS];
  unsigned char VillainWakingChance[NUM_VILLAINS];
  char MaintenanceWaterLevel;
  char DownstreamCounter;
  unsigned short NumMoves;
  unsigned short Score;
  short PlayerStrength;
  short VillainStrength[NUM_VILLAINS];
};

#define LampTurnsLeft               GameVars.LampTurnsLeft
#define MatchTurnsLeft              GameVars.MatchTurnsLeft
#define CandleTurnsLeft             GameVars.CandleTurnsLeft
#define MatchesLeft                 GameVars.MatchesLeft
#define ReservoirFillCountdown      GameVars.ReservoirFillCountdown
#define ReservoirDrainCountdown     GameVars.ReservoirDrainCountdown
#define BellRungCountdown           GameVars.BellRungCountdown
#define CandlesLitCountdown         GameVars.CandlesLitCountdown
#define BellHotCountdown            GameVars.BellHotCountdown
#define CaveHoleDepth               GameVars.CaveHoleDepth
#define NumDeaths                   GameVars.NumDeaths
#define CyclopsCounter              GameVars.CyclopsCounter
#define CyclopsState                GameVars.CyclopsState
#define LoadAllowed                 GameVars.LoadAllowed
#define TrollDescType               GameVars.TrollDescType
#define ThiefDescType               GameVars.ThiefDescType
#define EnableCureRoutine           GameVars.EnableCureRoutine
#define VillainAttacking            GameVars.VillainAttacking
#define VillainStaggered            GameVars.VillainStaggered
#define VillainWakingChance         GameVars.VillainWakingChance
#define MaintenanceWaterLevel       GameVars.MaintenanceWaterLevel
#define DownstreamCounter           GameVars.DownstreamCounter
#define NumMoves                    GameVars.NumMoves
#define Score                       GameVars.Score
#define PlayerStrength              GameVars.PlayerStrength
#define VillainStrength             GameVars.VillainStrength



//room properties bit flags

#define R_DESCRIBED    (1<<0)  // set when room description already printed
#define R_BODYOFWATER  (1<<1)  
#define R_LIT          (1<<2)  // set when there is natural light or a light fixture
#define R_WATERHERE    (1<<3)  
#define R_SACRED       (1<<4)  // set when thief not allowed in room
#define R_MAZE         (1<<5)  
#define R_ALWAYSDESC   (1<<6)  
#define R_SCOREDVISIT  (1<<7)  



//object properties bit flags

#define PROP_OPENABLE     (1<< 0)   
#define PROP_OPEN         (1<< 1)   
#define PROP_LIT          (1<< 2)   
#define PROP_NODESC       (1<< 3)   
#define PROP_NOTTAKEABLE  (1<< 4)   
#define PROP_MOVEDDESC    (1<< 5)  // set when object is first taken
#define PROP_INSIDEDESC   (1<< 6)  // set for objects that are initially described as inside another object
#define PROP_SACRED       (1<< 7)  // set for objects that aren't allowed to be taken by thief
#define PROP_EVERYWHERE   (1<< 8)   
#define PROP_WEAPON       (1<< 9)   
#define PROP_ACTOR        (1<<10)  
#define PROP_INVISIBLE    (1<<11)  
#define PROP_INFLAMMABLE  (1<<12)  
#define PROP_SURFACE      (1<<13)  
#define PROP_SCOREDTAKE   (1<<14)  
#define PROP_SCOREDCASE   (1<<15)  



enum
{
  WORD_NULL,

  WORD_A,
  WORD_ACROSS,
  WORD_AGAIN,
  WORD_ALL,
  WORD_AN,
  WORD_AND,
  WORD_AT,
  WORD_AUTOPLAY,
  WORD_BUT,
  WORD_EVERYTHING,
  WORD_EXCEPT,
  WORD_EXIT,
  WORD_FOR,
  WORD_FROM,
  WORD_G,
  WORD_IN,
  WORD_INSIDE,
  WORD_INTO,
  WORD_IT,
  WORD_N,
  WORD_NO,
  WORD_ODYSSEUS,
  WORD_OF,
  WORD_OFF,
  WORD_ON,
  WORD_ONTO,
  WORD_OOPS,
  WORD_OUT,
  WORD_OUTSIDE,
  WORD_OVER,
  WORD_RESTART,
  WORD_RESTORE,
  WORD_THE,
  WORD_THEM,
  WORD_THEN,
  WORD_THROUGH,
  WORD_TO,
  WORD_TOWARD,
  WORD_ULYSSES,
  WORD_USING,
  WORD_WITH,
  WORD_Y,
  WORD_YES
};



//move directions

enum
{
  DIR_NULL,

  DIR_N,
  DIR_S,
  DIR_E,
  DIR_W,
  DIR_NE,
  DIR_NW,
  DIR_SE,
  DIR_SW,
  DIR_U,
  DIR_D,
  DIR_IN,
  DIR_OUT
};



//actions

enum
{
  A_NOTHING,

  A_NORTH, //direction actions must be grouped together in this order
  A_SOUTH,
  A_EAST,
  A_WEST,
  A_NORTHEAST,
  A_NORTHWEST,
  A_SOUTHEAST,
  A_SOUTHWEST,
  A_UP,
  A_DOWN,
  A_IN,
  A_OUT,

  A_ACTIVATE,
  A_ATTACK,
  A_BREAK,
  A_BRIEF,
  A_BRUSH,
  A_CLIMB,
  A_CLIMBDOWN,
  A_CLIMBTHROUGH,
  A_CLIMBUP,
  A_CLOSE,
  A_COUNT,
  A_CROSS,
  A_DEACTIVATE,
  A_DEFLATE,
  A_DIAGNOSE,
  A_DIG,
  A_DISEMBARK,
  A_DISMOUNT,
  A_DRINK,
  A_DROP,
  A_EAT,
  A_ECHO,
  A_EMPTY,
  A_ENTER,
  A_EXAMINE,
  A_EXIT,
  A_EXORCISE,
  A_FILL,
  A_FIX,
  A_GIVE,
  A_GO,
  A_GREET,
  A_INFLATE,
  A_INVENTORY,
  A_JUMP,
  A_KNOCK,
  A_LAND,
  A_LAUNCH,
  A_LISTENTO,
  A_LOCK,
  A_LOOK,
  A_LOOKBEHIND,
  A_LOOKIN,
  A_LOOKON,
  A_LOOKTHROUGH,
  A_LOOKUNDER,
  A_LOWER,
  A_MOUNT,
  A_MOVE,
  A_ODYSSEUS,
  A_OIL,
  A_OPEN,
  A_PLAY,
  A_POUR,
  A_PRAY,
  A_PRY,
  A_PULL,
  A_PUSH,
  A_PUT,
  A_QUIT,
  A_RAISE,
  A_READ,
  A_REMOVE,
  A_RESTART,
  A_RESTORE,
  A_RING,
  A_SAVE,
  A_SAY,
  A_SCORE,
  A_SLEEP,
  A_SLEEPON,
  A_SLIDEDOWN,
  A_SLIDEUP,
  A_SMELL,
  A_SQUEEZE,
  A_SUPERBRIEF,
  A_SWIM,
  A_TAKE,
  A_TALKTO,
  A_TEMPLETREASURE,
  A_THROW,
  A_TIE,
  A_TOUCH,
  A_TURN,
  A_UNLOCK,
  A_UNTIE,
  A_VERBOSE,
  A_VERSION,
  A_WAIT,
  A_WAVE,
  A_WEAR,
  A_WHEREIS,
  A_WIND
};



//fixed (unmoving) objects

enum
{
  FOBJ_SCENERY_VIS = 2048, //some anonymous scenery object, visible
  FOBJ_SCENERY_NOTVIS,     //                               not visible

  FOBJ_NOTVIS,             //fixed object not visible

  FOBJ_AMB,                //amibiguous (ask for clarification)

  //game-specific data follows

  FOBJ_SLIDE,
  FOBJ_BOARD,
  FOBJ_SONGBIRD,
  FOBJ_WHITE_HOUSE,
  FOBJ_FOREST,
  FOBJ_TREE,
  FOBJ_KITCHEN_WINDOW,
  FOBJ_CHIMNEY,
  FOBJ_BOARDED_WINDOW,
  FOBJ_CRACK,
  FOBJ_GRATE,
  FOBJ_CLIMBABLE_CLIFF,
  FOBJ_WHITE_CLIFF,
  FOBJ_BODIES,
  FOBJ_RAINBOW,
  FOBJ_RIVER,
  FOBJ_LADDER,
  FOBJ_TRAP_DOOR,
  FOBJ_STAIRS,
  FOBJ_MOUNTAIN_RANGE,
  FOBJ_BOLT,
  FOBJ_BUBBLE,
  FOBJ_ALTAR,
  FOBJ_YELLOW_BUTTON,
  FOBJ_BROWN_BUTTON,
  FOBJ_RED_BUTTON,
  FOBJ_BLUE_BUTTON,
  FOBJ_RUG,
  FOBJ_DAM,
  FOBJ_FRONT_DOOR,
  FOBJ_BARROW_DOOR,
  FOBJ_BARROW,
  FOBJ_BONES,
  FOBJ_LEAK,
  FOBJ_MIRROR2,
  FOBJ_MIRROR1,
  FOBJ_PRAYER,
  FOBJ_RAILING,
  FOBJ_SAND,
  FOBJ_MACHINE_SWITCH,
  FOBJ_WOODEN_DOOR,
  FOBJ_PEDESTAL,
  FOBJ_CONTROL_PANEL,
  FOBJ_NAILS,
  FOBJ_GRANITE_WALL,
  FOBJ_CHAIN,
  FOBJ_GATE,
  FOBJ_STUDIO_DOOR,
  FOBJ_CHASM,
  FOBJ_LAKE,
  FOBJ_STREAM,
  FOBJ_GAS
};



struct ROOM_STRUCT
{
  unsigned char prop;
};



struct OBJ_STRUCT
{
  unsigned short loc;
  unsigned short order;
  unsigned short prop;
  unsigned char thiefvalue;
};



//utility.cpp
void ActivateAutoPlay(void);
char *GetString(void);
void PrintNewLine(void);
void PrintChar(uint8_t c);
void PrintText(const char *p);
void PrintLine(const char *p);
void PrintTextIndex(int16_t i);
void PrintLineIndex(int16_t i);
void PrintRoomDescIndex(int16_t i, int16_t segment);
#ifndef NO_STATUS_LINE
void PrintRoomDescIndexToStatusLine(int16_t i);
void PrintScoreAndMovesToStatusLine(void);
void PrintStatusLine(void);
#endif
void PrintBlockMsgIndex(int16_t i, int16_t segment);
void PrintObjectDescIndex(int16_t i, int16_t segment);
int PrintIntToBuffer(int num, char *buf);
void PrintInteger(int num);
int GetDoMiscWithTo_Action(int i);
int GetDoMiscWithTo_Obj(int i);
int GetDoMisc_Action(int i);
int GetDoMisc_Obj(int i);
int GetGoFrom_Room(int i);
int GetGoFrom_Action(int i);
void InitObjectLocations(void);
void InitRoomProperties(void);
void InitObjectProperties(void);
int GetObjectSize(int obj);
int GetObjectCapacity(int obj);
uint8_t GetRoomPassage(int16_t room, int16_t dir);
void GetNounPhraseToFixedObj_Words(int i, int *w1, int *w2);
int GetNounPhraseToFixedObj_Room(int i);
int GetNounPhraseToFixedObj_FObj(int i);
void GetNounPhraseToObj_Words(int i, int *w1, int *w2, int *w3);
int GetNounPhraseToObj_Obj(int i);
void GetVerbToAction_Words(int i, int *w1, int *w2);
int GetVerbToAction_Action(int i);
uint8_t ReadWordList(uint8_t mode);
int ReadWriteSaveState(FILE *f, int mode);
void SetRandomSeed(uint32_t seed);
uint32_t GetRandom(uint32_t range);

//parser.cpp
void PrintBlockMsg(int newroom);
void PrintObjectDesc(int obj, int desc_flag);
void PrintContents(int obj, const char *heading, int print_empty);
void GetWords(void);
unsigned char MatchCurWordIndex(int i);
void PrintWord(unsigned int w, int capital_flag);
int IsObjVisible(int obj);
int IsPlayerInDarkness(void);
int GetNumObjectsInLocation(int loc);
void MoveObjOrderToLast(int obj);
void PrintPlayerRoomDesc(int force_description);
void PrintPresentObjects(int location, const char *heading, int list_flag);
int GetAllObjFromInput(int room);
int TakeRoutine(int obj, const char *msg);
int GetWith(void);
void GameLoop(void);

//game.cpp
void CallDoMiscWithTo(int i, int with_to);
void CallDoMisc(int i);
int CallGoFrom(int i);
int CallDoMiscGiveTo(int to, int obj);
int CallDoMiscThrowTo(int to, int obj);
int OverrideRoomDesc(int room);
int OverrideObjectDesc(int obj, int desc_flag);
int PercentChance(int x, int x_not_lucky);
void YoureDead(void);
void DoJump(void);
void DoSleep(void);
void DoDisembark(void);
void DoLaunch(void);
void DoLand(void);
void DoEcho(void);
void DoPray(void);
void DoVersion(void);
void DoDiagnose(void);
void DoOdysseus(void);
void DoSwim(void);
void DoTempleTreasure(void);
void DoIntro(void);
void DoCommandActor(int obj);
void DoTalkTo(void);
void DoGreet(void);
void DoSay(void);
int ActionDirectionRoutine(int newroom);
int InterceptAction(int action);
int InterceptTakeObj(int obj);
int GetPlayersVehicle(void);
int InterceptTakeFixedObj(int obj);
int InterceptTakeOutOf(int container);
int InterceptDropPutObj(int obj, int container, int test, int multi);
void ThrowObjRoutine(int obj, int to);
void RunEventRoutines(void);
int CountLoot(void);
int GetScore(void);
int GetMaxScore(void);
void PrintRankName(void);
void InitGameState(void);

//villains.cpp
int PlayerFightStrength(int adjust);
void ThiefRecoverStiletto(void);
void VillainDead(int i);
void VillainConscious(int i);
void VillainsRoutine(void);
void PlayerBlow(int obj, int player_weapon);
void ThiefProtectsTreasure(void);
