// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "def.h"
#include "_enum.h"



//-----------------------------------------------------------------------------

struct NOUNPHRASETOFIXEDOBJ_STRUCT
{
  const char *phrase;
  const unsigned short room;
  const unsigned short fobj;

  int w1, w2;
};



// single words that refer to multiple objects:
//   window  cliff  door  table  button

// two-word noun phrases must come before one-word, for matching to work properly

struct NOUNPHRASETOFIXEDOBJ_STRUCT NounPhraseToFixedObj[] =
{
  { "steep slide"          , ROOM_CELLAR              , FOBJ_SLIDE           },
  { "steep slide"          , ROOM_SLIDE_ROOM          , FOBJ_SLIDE           },
  { "metal slide"          , ROOM_CELLAR              , FOBJ_SLIDE           },
  { "metal slide"          , ROOM_SLIDE_ROOM          , FOBJ_SLIDE           },
  { "steep ramp"           , ROOM_CELLAR              , FOBJ_SLIDE           },
  { "steep ramp"           , ROOM_SLIDE_ROOM          , FOBJ_SLIDE           },
  { "metal ramp"           , ROOM_CELLAR              , FOBJ_SLIDE           },
  { "metal ramp"           , ROOM_SLIDE_ROOM          , FOBJ_SLIDE           },
  { "slide"                , ROOM_CELLAR              , FOBJ_SLIDE           },
  { "slide"                , ROOM_SLIDE_ROOM          , FOBJ_SLIDE           },
  { "ramp"                 , ROOM_CELLAR              , FOBJ_SLIDE           },
  { "ramp"                 , ROOM_SLIDE_ROOM          , FOBJ_SLIDE           },
  { "boards"               , ROOM_WEST_OF_HOUSE       , FOBJ_BOARD           },
  { "boards"               , ROOM_NORTH_OF_HOUSE      , FOBJ_BOARD           },
  { "boards"               , ROOM_SOUTH_OF_HOUSE      , FOBJ_BOARD           },
  { "board"                , ROOM_WEST_OF_HOUSE       , FOBJ_BOARD           },
  { "board"                , ROOM_NORTH_OF_HOUSE      , FOBJ_BOARD           },
  { "board"                , ROOM_SOUTH_OF_HOUSE      , FOBJ_BOARD           },
  { "song bird"            , ROOM_FOREST_1            , FOBJ_SONGBIRD        },
  { "song bird"            , ROOM_FOREST_2            , FOBJ_SONGBIRD        },
  { "song bird"            , ROOM_FOREST_3            , FOBJ_SONGBIRD        },
  { "song bird"            , ROOM_PATH                , FOBJ_SONGBIRD        },
  { "song bird"            , ROOM_UP_A_TREE           , FOBJ_SONGBIRD        },
  { "song bird"            , ROOM_CLEARING            , FOBJ_SONGBIRD        },
  { "songbird"             , ROOM_FOREST_1            , FOBJ_SONGBIRD        },
  { "songbird"             , ROOM_FOREST_2            , FOBJ_SONGBIRD        },
  { "songbird"             , ROOM_FOREST_3            , FOBJ_SONGBIRD        },
  { "songbird"             , ROOM_PATH                , FOBJ_SONGBIRD        },
  { "songbird"             , ROOM_UP_A_TREE           , FOBJ_SONGBIRD        },
  { "songbird"             , ROOM_CLEARING            , FOBJ_SONGBIRD        },
  { "bird"                 , ROOM_FOREST_1            , FOBJ_SONGBIRD        },
  { "bird"                 , ROOM_FOREST_2            , FOBJ_SONGBIRD        },
  { "bird"                 , ROOM_FOREST_3            , FOBJ_SONGBIRD        },
  { "bird"                 , ROOM_PATH                , FOBJ_SONGBIRD        },
  { "bird"                 , ROOM_UP_A_TREE           , FOBJ_SONGBIRD        },
  { "bird"                 , ROOM_CLEARING            , FOBJ_SONGBIRD        },
  { "white house"          , ROOM_WEST_OF_HOUSE       , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_NORTH_OF_HOUSE      , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_SOUTH_OF_HOUSE      , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_EAST_OF_HOUSE       , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_FOREST_1            , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_FOREST_2            , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_MOUNTAINS           , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_FOREST_3            , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_PATH                , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_UP_A_TREE           , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_GRATING_CLEARING    , FOBJ_WHITE_HOUSE     },
  { "white house"          , ROOM_CLEARING            , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_WEST_OF_HOUSE       , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_NORTH_OF_HOUSE      , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_SOUTH_OF_HOUSE      , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_EAST_OF_HOUSE       , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_FOREST_1            , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_FOREST_2            , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_MOUNTAINS           , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_FOREST_3            , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_PATH                , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_UP_A_TREE           , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_GRATING_CLEARING    , FOBJ_WHITE_HOUSE     },
  { "house"                , ROOM_CLEARING            , FOBJ_WHITE_HOUSE     },
  { "forest"               , ROOM_WEST_OF_HOUSE       , FOBJ_FOREST          },
  { "forest"               , ROOM_NORTH_OF_HOUSE      , FOBJ_FOREST          },
  { "forest"               , ROOM_SOUTH_OF_HOUSE      , FOBJ_FOREST          },
  { "forest"               , ROOM_EAST_OF_HOUSE       , FOBJ_FOREST          },
  { "forest"               , ROOM_FOREST_1            , FOBJ_FOREST          },
  { "forest"               , ROOM_FOREST_2            , FOBJ_FOREST          },
  { "forest"               , ROOM_FOREST_3            , FOBJ_FOREST          },
  { "forest"               , ROOM_PATH                , FOBJ_FOREST          },
  { "forest"               , ROOM_UP_A_TREE           , FOBJ_FOREST          },
  { "forest"               , ROOM_CLEARING            , FOBJ_FOREST          },
  { "trees"                , ROOM_WEST_OF_HOUSE       , FOBJ_FOREST          },
  { "trees"                , ROOM_NORTH_OF_HOUSE      , FOBJ_FOREST          },
  { "trees"                , ROOM_SOUTH_OF_HOUSE      , FOBJ_FOREST          },
  { "trees"                , ROOM_EAST_OF_HOUSE       , FOBJ_FOREST          },
  { "trees"                , ROOM_FOREST_1            , FOBJ_FOREST          },
  { "trees"                , ROOM_FOREST_2            , FOBJ_FOREST          },
  { "trees"                , ROOM_FOREST_3            , FOBJ_FOREST          },
  { "trees"                , ROOM_PATH                , FOBJ_FOREST          },
  { "trees"                , ROOM_UP_A_TREE           , FOBJ_FOREST          },
  { "trees"                , ROOM_CLEARING            , FOBJ_FOREST          },
  { "large tree"           , ROOM_FOREST_1            , FOBJ_TREE            },
  { "large tree"           , ROOM_FOREST_2            , FOBJ_TREE            },
  { "large tree"           , ROOM_MOUNTAINS           , FOBJ_TREE            },
  { "large tree"           , ROOM_FOREST_3            , FOBJ_TREE            },
  { "large tree"           , ROOM_PATH                , FOBJ_TREE            },
  { "large tree"           , ROOM_UP_A_TREE           , FOBJ_TREE            },
  { "large tree"           , ROOM_CLEARING            , FOBJ_TREE            },
  { "tree"                 , ROOM_FOREST_1            , FOBJ_TREE            },
  { "tree"                 , ROOM_FOREST_2            , FOBJ_TREE            },
  { "tree"                 , ROOM_MOUNTAINS           , FOBJ_TREE            },
  { "tree"                 , ROOM_FOREST_3            , FOBJ_TREE            },
  { "tree"                 , ROOM_PATH                , FOBJ_TREE            },
  { "tree"                 , ROOM_UP_A_TREE           , FOBJ_TREE            },
  { "tree"                 , ROOM_CLEARING            , FOBJ_TREE            },
  { "small window"         , ROOM_EAST_OF_HOUSE       , FOBJ_KITCHEN_WINDOW  },
  { "small window"         , ROOM_KITCHEN             , FOBJ_KITCHEN_WINDOW  },
  { "kitchen window"       , ROOM_EAST_OF_HOUSE       , FOBJ_KITCHEN_WINDOW  },
  { "kitchen window"       , ROOM_KITCHEN             , FOBJ_KITCHEN_WINDOW  },
  { "window"               , ROOM_EAST_OF_HOUSE       , FOBJ_KITCHEN_WINDOW  },
  { "window"               , ROOM_KITCHEN             , FOBJ_KITCHEN_WINDOW  },
  { "dark chimney"         , ROOM_KITCHEN             , FOBJ_CHIMNEY         },
  { "dark chimney"         , ROOM_STUDIO              , FOBJ_CHIMNEY         },
  { "narrow chimney"       , ROOM_KITCHEN             , FOBJ_CHIMNEY         },
  { "narrow chimney"       , ROOM_STUDIO              , FOBJ_CHIMNEY         },
  { "chimney"              , ROOM_KITCHEN             , FOBJ_CHIMNEY         },
  { "chimney"              , ROOM_STUDIO              , FOBJ_CHIMNEY         },
  { "boarded window"       , ROOM_NORTH_OF_HOUSE      , FOBJ_BOARDED_WINDOW  },
  { "boarded window"       , ROOM_SOUTH_OF_HOUSE      , FOBJ_BOARDED_WINDOW  },
  { "window"               , ROOM_NORTH_OF_HOUSE      , FOBJ_BOARDED_WINDOW  },
  { "window"               , ROOM_SOUTH_OF_HOUSE      , FOBJ_BOARDED_WINDOW  },
  { "narrow crack"         , ROOM_DAMP_CAVE           , FOBJ_CRACK           },
  { "narrow crack"         , ROOM_CHASM_ROOM          , FOBJ_CRACK           },
  { "crack"                , ROOM_DAMP_CAVE           , FOBJ_CRACK           },
  { "crack"                , ROOM_CHASM_ROOM          , FOBJ_CRACK           },
  { "grate"                , ROOM_GRATING_CLEARING    , FOBJ_GRATE           },
  { "grate"                , ROOM_GRATING_ROOM        , FOBJ_GRATE           },
  { "grating"              , ROOM_GRATING_CLEARING    , FOBJ_GRATE           },
  { "grating"              , ROOM_GRATING_ROOM        , FOBJ_GRATE           },
  { "rocky ledge"          , ROOM_CANYON_BOTTOM       , FOBJ_CLIMBABLE_CLIFF },
  { "rocky ledge"          , ROOM_CLIFF_MIDDLE        , FOBJ_CLIMBABLE_CLIFF },
  { "rocky ledge"          , ROOM_CANYON_VIEW         , FOBJ_CLIMBABLE_CLIFF },
  { "rocky cliff"          , ROOM_CANYON_BOTTOM       , FOBJ_CLIMBABLE_CLIFF },
  { "rocky cliff"          , ROOM_CLIFF_MIDDLE        , FOBJ_CLIMBABLE_CLIFF },
  { "rocky cliff"          , ROOM_CANYON_VIEW         , FOBJ_CLIMBABLE_CLIFF },
  { "ledge"                , ROOM_CANYON_BOTTOM       , FOBJ_CLIMBABLE_CLIFF },
  { "ledge"                , ROOM_CLIFF_MIDDLE        , FOBJ_CLIMBABLE_CLIFF },
  { "ledge"                , ROOM_CANYON_VIEW         , FOBJ_CLIMBABLE_CLIFF },
  { "cliff"                , ROOM_CANYON_BOTTOM       , FOBJ_CLIMBABLE_CLIFF },
  { "cliff"                , ROOM_CLIFF_MIDDLE        , FOBJ_CLIMBABLE_CLIFF },
  { "cliff"                , ROOM_CANYON_VIEW         , FOBJ_CLIMBABLE_CLIFF },
  { "white cliff"          , ROOM_WHITE_CLIFFS_NORTH  , FOBJ_WHITE_CLIFF     },
  { "white cliff"          , ROOM_WHITE_CLIFFS_SOUTH  , FOBJ_WHITE_CLIFF     },
  { "white cliffs"         , ROOM_WHITE_CLIFFS_NORTH  , FOBJ_WHITE_CLIFF     },
  { "white cliffs"         , ROOM_WHITE_CLIFFS_SOUTH  , FOBJ_WHITE_CLIFF     },
  { "cliff"                , ROOM_WHITE_CLIFFS_NORTH  , FOBJ_WHITE_CLIFF     },
  { "cliff"                , ROOM_WHITE_CLIFFS_SOUTH  , FOBJ_WHITE_CLIFF     },
  { "cliffs"               , ROOM_WHITE_CLIFFS_NORTH  , FOBJ_WHITE_CLIFF     },
  { "cliffs"               , ROOM_WHITE_CLIFFS_SOUTH  , FOBJ_WHITE_CLIFF     },
  { "mangled bodies"       , ROOM_ENTRANCE_TO_HADES   , FOBJ_BODIES          },
  { "mangled bodies"       , ROOM_LAND_OF_LIVING_DEAD , FOBJ_BODIES          },
  { "bodies"               , ROOM_ENTRANCE_TO_HADES   , FOBJ_BODIES          },
  { "bodies"               , ROOM_LAND_OF_LIVING_DEAD , FOBJ_BODIES          },
  { "rainbow"              , ROOM_ARAGAIN_FALLS       , FOBJ_RAINBOW         },
  { "rainbow"              , ROOM_ON_RAINBOW          , FOBJ_RAINBOW         },
  { "rainbow"              , ROOM_END_OF_RAINBOW      , FOBJ_RAINBOW         },
  { "rainbow"              , ROOM_CANYON_VIEW         , FOBJ_RAINBOW         },
  { "frigid river"         , ROOM_DAM_BASE            , FOBJ_RIVER           },
  { "frigid river"         , ROOM_RIVER_1             , FOBJ_RIVER           },
  { "frigid river"         , ROOM_RIVER_2             , FOBJ_RIVER           },
  { "frigid river"         , ROOM_RIVER_3             , FOBJ_RIVER           },
  { "frigid river"         , ROOM_WHITE_CLIFFS_NORTH  , FOBJ_RIVER           },
  { "frigid river"         , ROOM_WHITE_CLIFFS_SOUTH  , FOBJ_RIVER           },
  { "frigid river"         , ROOM_RIVER_4             , FOBJ_RIVER           },
  { "frigid river"         , ROOM_RIVER_5             , FOBJ_RIVER           },
  { "frigid river"         , ROOM_SHORE               , FOBJ_RIVER           },
  { "frigid river"         , ROOM_SANDY_BEACH         , FOBJ_RIVER           },
  { "frigid river"         , ROOM_ARAGAIN_FALLS       , FOBJ_RIVER           },
  { "frigid river"         , ROOM_END_OF_RAINBOW      , FOBJ_RIVER           },
  { "frigid river"         , ROOM_CANYON_BOTTOM       , FOBJ_RIVER           },
  { "frigid river"         , ROOM_CLIFF_MIDDLE        , FOBJ_RIVER           },
  { "frigid river"         , ROOM_CANYON_VIEW         , FOBJ_RIVER           },
  { "river"                , ROOM_DAM_BASE            , FOBJ_RIVER           },
  { "river"                , ROOM_RIVER_1             , FOBJ_RIVER           },
  { "river"                , ROOM_RIVER_2             , FOBJ_RIVER           },
  { "river"                , ROOM_RIVER_3             , FOBJ_RIVER           },
  { "river"                , ROOM_WHITE_CLIFFS_NORTH  , FOBJ_RIVER           },
  { "river"                , ROOM_WHITE_CLIFFS_SOUTH  , FOBJ_RIVER           },
  { "river"                , ROOM_RIVER_4             , FOBJ_RIVER           },
  { "river"                , ROOM_RIVER_5             , FOBJ_RIVER           },
  { "river"                , ROOM_SHORE               , FOBJ_RIVER           },
  { "river"                , ROOM_SANDY_BEACH         , FOBJ_RIVER           },
  { "river"                , ROOM_ARAGAIN_FALLS       , FOBJ_RIVER           },
  { "river"                , ROOM_END_OF_RAINBOW      , FOBJ_RIVER           },
  { "river"                , ROOM_CANYON_BOTTOM       , FOBJ_RIVER           },
  { "river"                , ROOM_CLIFF_MIDDLE        , FOBJ_RIVER           },
  { "river"                , ROOM_CANYON_VIEW         , FOBJ_RIVER           },
  { "rickety ladder"       , ROOM_LADDER_TOP          , FOBJ_LADDER          },
  { "rickety ladder"       , ROOM_LADDER_BOTTOM       , FOBJ_LADDER          },
  { "narrow ladder"        , ROOM_LADDER_TOP          , FOBJ_LADDER          },
  { "narrow ladder"        , ROOM_LADDER_BOTTOM       , FOBJ_LADDER          },
  { "wooden ladder"        , ROOM_LADDER_TOP          , FOBJ_LADDER          },
  { "wooden ladder"        , ROOM_LADDER_BOTTOM       , FOBJ_LADDER          },
  { "ladder"               , ROOM_LADDER_TOP          , FOBJ_LADDER          },
  { "ladder"               , ROOM_LADDER_BOTTOM       , FOBJ_LADDER          },

  { "trap door"            , ROOM_LIVING_ROOM         , FOBJ_TRAP_DOOR       },
  { "trap"                 , ROOM_LIVING_ROOM         , FOBJ_TRAP_DOOR       },
  { "trap-door"            , ROOM_LIVING_ROOM         , FOBJ_TRAP_DOOR       },
  { "trapdoor"             , ROOM_LIVING_ROOM         , FOBJ_TRAP_DOOR       },
  { "wooden door"          , ROOM_LIVING_ROOM         , FOBJ_WOODEN_DOOR     },
  { "gothic lettering"     , ROOM_LIVING_ROOM         , FOBJ_WOODEN_DOOR     },
  { "lettering"            , ROOM_LIVING_ROOM         , FOBJ_WOODEN_DOOR     },
  { "door"                 , ROOM_LIVING_ROOM         , FOBJ_AMB             },  // living room: trap or wooden door?

  { "trap door"            , ROOM_CELLAR              , FOBJ_TRAP_DOOR       },
  { "trap"                 , ROOM_CELLAR              , FOBJ_TRAP_DOOR       },
  { "trap-door"            , ROOM_CELLAR              , FOBJ_TRAP_DOOR       },
  { "trapdoor"             , ROOM_CELLAR              , FOBJ_TRAP_DOOR       },
  { "door"                 , ROOM_CELLAR              , FOBJ_TRAP_DOOR       },
  { "stairs"               , ROOM_KITCHEN             , FOBJ_STAIRS          },
  { "stairs"               , ROOM_ATTIC               , FOBJ_STAIRS          },
  { "stairs"               , ROOM_LIVING_ROOM         , FOBJ_STAIRS          },
  { "stairs"               , ROOM_CELLAR              , FOBJ_STAIRS          },
  { "stairs"               , ROOM_CYCLOPS_ROOM        , FOBJ_STAIRS          },
  { "stairs"               , ROOM_TREASURE_ROOM       , FOBJ_STAIRS          },
  { "stairs"               , ROOM_RESERVOIR_NORTH     , FOBJ_STAIRS          },
  { "stairs"               , ROOM_SMALL_CAVE          , FOBJ_STAIRS          },
  { "stairs"               , ROOM_TINY_CAVE           , FOBJ_STAIRS          },
  { "stairs"               , ROOM_ATLANTIS_ROOM       , FOBJ_STAIRS          },
  { "stairs"               , ROOM_EW_PASSAGE          , FOBJ_STAIRS          },
  { "stairs"               , ROOM_DEEP_CANYON         , FOBJ_STAIRS          },
  { "stairs"               , ROOM_LOUD_ROOM           , FOBJ_STAIRS          },
  { "stairs"               , ROOM_CHASM_ROOM          , FOBJ_STAIRS          },
  { "stairs"               , ROOM_EGYPT_ROOM          , FOBJ_STAIRS          },
  { "stairs"               , ROOM_TORCH_ROOM          , FOBJ_STAIRS          },
  { "stairs"               , ROOM_NORTH_TEMPLE        , FOBJ_STAIRS          },
  { "stairs"               , ROOM_SMELLY_ROOM         , FOBJ_STAIRS          },
  { "stairs"               , ROOM_GAS_ROOM            , FOBJ_STAIRS          },
  { "stairs"               , ROOM_LADDER_TOP          , FOBJ_STAIRS          },
  { "staircase"            , ROOM_KITCHEN             , FOBJ_STAIRS          },
  { "staircase"            , ROOM_ATTIC               , FOBJ_STAIRS          },
  { "staircase"            , ROOM_LIVING_ROOM         , FOBJ_STAIRS          },
  { "staircase"            , ROOM_CELLAR              , FOBJ_STAIRS          },
  { "staircase"            , ROOM_CYCLOPS_ROOM        , FOBJ_STAIRS          },
  { "staircase"            , ROOM_TREASURE_ROOM       , FOBJ_STAIRS          },
  { "staircase"            , ROOM_RESERVOIR_NORTH     , FOBJ_STAIRS          },
  { "staircase"            , ROOM_SMALL_CAVE          , FOBJ_STAIRS          },
  { "staircase"            , ROOM_TINY_CAVE           , FOBJ_STAIRS          },
  { "staircase"            , ROOM_ATLANTIS_ROOM       , FOBJ_STAIRS          },
  { "staircase"            , ROOM_EW_PASSAGE          , FOBJ_STAIRS          },
  { "staircase"            , ROOM_DEEP_CANYON         , FOBJ_STAIRS          },
  { "staircase"            , ROOM_LOUD_ROOM           , FOBJ_STAIRS          },
  { "staircase"            , ROOM_CHASM_ROOM          , FOBJ_STAIRS          },
  { "staircase"            , ROOM_EGYPT_ROOM          , FOBJ_STAIRS          },
  { "staircase"            , ROOM_TORCH_ROOM          , FOBJ_STAIRS          },
  { "staircase"            , ROOM_NORTH_TEMPLE        , FOBJ_STAIRS          },
  { "staircase"            , ROOM_SMELLY_ROOM         , FOBJ_STAIRS          },
  { "staircase"            , ROOM_GAS_ROOM            , FOBJ_STAIRS          },
  { "staircase"            , ROOM_LADDER_TOP          , FOBJ_STAIRS          },
  { "stairway"             , ROOM_KITCHEN             , FOBJ_STAIRS          },
  { "stairway"             , ROOM_ATTIC               , FOBJ_STAIRS          },
  { "stairway"             , ROOM_LIVING_ROOM         , FOBJ_STAIRS          },
  { "stairway"             , ROOM_CELLAR              , FOBJ_STAIRS          },
  { "stairway"             , ROOM_CYCLOPS_ROOM        , FOBJ_STAIRS          },
  { "stairway"             , ROOM_TREASURE_ROOM       , FOBJ_STAIRS          },
  { "stairway"             , ROOM_RESERVOIR_NORTH     , FOBJ_STAIRS          },
  { "stairway"             , ROOM_SMALL_CAVE          , FOBJ_STAIRS          },
  { "stairway"             , ROOM_TINY_CAVE           , FOBJ_STAIRS          },
  { "stairway"             , ROOM_ATLANTIS_ROOM       , FOBJ_STAIRS          },
  { "stairway"             , ROOM_EW_PASSAGE          , FOBJ_STAIRS          },
  { "stairway"             , ROOM_DEEP_CANYON         , FOBJ_STAIRS          },
  { "stairway"             , ROOM_LOUD_ROOM           , FOBJ_STAIRS          },
  { "stairway"             , ROOM_CHASM_ROOM          , FOBJ_STAIRS          },
  { "stairway"             , ROOM_EGYPT_ROOM          , FOBJ_STAIRS          },
  { "stairway"             , ROOM_TORCH_ROOM          , FOBJ_STAIRS          },
  { "stairway"             , ROOM_NORTH_TEMPLE        , FOBJ_STAIRS          },
  { "stairway"             , ROOM_SMELLY_ROOM         , FOBJ_STAIRS          },
  { "stairway"             , ROOM_GAS_ROOM            , FOBJ_STAIRS          },
  { "stairway"             , ROOM_LADDER_TOP          , FOBJ_STAIRS          },
  { "mountain range"       , ROOM_MOUNTAINS           , FOBJ_MOUNTAIN_RANGE  },
  { "range"                , ROOM_MOUNTAINS           , FOBJ_MOUNTAIN_RANGE  },
  { "mountain"             , ROOM_MOUNTAINS           , FOBJ_MOUNTAIN_RANGE  },
  { "mountains"            , ROOM_MOUNTAINS           , FOBJ_MOUNTAIN_RANGE  },
  { "large bolt"           , ROOM_DAM_ROOM            , FOBJ_BOLT            },
  { "metal bolt"           , ROOM_DAM_ROOM            , FOBJ_BOLT            },
  { "large nut"            , ROOM_DAM_ROOM            , FOBJ_BOLT            },
  { "metal nut"            , ROOM_DAM_ROOM            , FOBJ_BOLT            },
  { "bolt"                 , ROOM_DAM_ROOM            , FOBJ_BOLT            },
  { "nut"                  , ROOM_DAM_ROOM            , FOBJ_BOLT            },
  { "small bubble"         , ROOM_DAM_ROOM            , FOBJ_BUBBLE          },
  { "green bubble"         , ROOM_DAM_ROOM            , FOBJ_BUBBLE          },
  { "bubble"               , ROOM_DAM_ROOM            , FOBJ_BUBBLE          },
  { "altar"                , ROOM_SOUTH_TEMPLE        , FOBJ_ALTAR           },

  { "yellow button"        , ROOM_MAINTENANCE_ROOM    , FOBJ_YELLOW_BUTTON   },
  { "yellow"               , ROOM_MAINTENANCE_ROOM    , FOBJ_YELLOW_BUTTON   },
  { "brown button"         , ROOM_MAINTENANCE_ROOM    , FOBJ_BROWN_BUTTON    },
  { "brown"                , ROOM_MAINTENANCE_ROOM    , FOBJ_BROWN_BUTTON    },
  { "red button"           , ROOM_MAINTENANCE_ROOM    , FOBJ_RED_BUTTON      },
  { "red"                  , ROOM_MAINTENANCE_ROOM    , FOBJ_RED_BUTTON      },
  { "blue button"          , ROOM_MAINTENANCE_ROOM    , FOBJ_BLUE_BUTTON     },
  { "blue"                 , ROOM_MAINTENANCE_ROOM    , FOBJ_BLUE_BUTTON     },
  { "button"               , ROOM_MAINTENANCE_ROOM    , FOBJ_AMB             },  // maintenance room: which button?

  { "large carpet"         , ROOM_LIVING_ROOM         , FOBJ_RUG             },
  { "oriental carpet"      , ROOM_LIVING_ROOM         , FOBJ_RUG             },
  { "large rug"            , ROOM_LIVING_ROOM         , FOBJ_RUG             },
  { "oriental rug"         , ROOM_LIVING_ROOM         , FOBJ_RUG             },
  { "carpet"               , ROOM_LIVING_ROOM         , FOBJ_RUG             },
  { "rug"                  , ROOM_LIVING_ROOM         , FOBJ_RUG             },
  { "dam"                  , ROOM_DAM_ROOM            , FOBJ_DAM             },
  { "gate"                 , ROOM_DAM_ROOM            , FOBJ_DAM             },
  { "gates"                , ROOM_DAM_ROOM            , FOBJ_DAM             },
  { "front door"           , ROOM_WEST_OF_HOUSE       , FOBJ_FRONT_DOOR      },
  { "boarded door"         , ROOM_WEST_OF_HOUSE       , FOBJ_FRONT_DOOR      },
  { "door"                 , ROOM_WEST_OF_HOUSE       , FOBJ_FRONT_DOOR      },
  { "huge door"            , ROOM_STONE_BARROW        , FOBJ_BARROW_DOOR     },
  { "stone door"           , ROOM_STONE_BARROW        , FOBJ_BARROW_DOOR     },
  { "door"                 , ROOM_STONE_BARROW        , FOBJ_BARROW_DOOR     },
  { "massive barrow"       , ROOM_STONE_BARROW        , FOBJ_BARROW          },
  { "stone barrow"         , ROOM_STONE_BARROW        , FOBJ_BARROW          },
  { "barrow"               , ROOM_STONE_BARROW        , FOBJ_BARROW          },
  { "skeleton"             , ROOM_MAZE_5              , FOBJ_BONES           },
  { "leaky pipe"           , ROOM_MAINTENANCE_ROOM    , FOBJ_LEAK            },
  { "leak"                 , ROOM_MAINTENANCE_ROOM    , FOBJ_LEAK            },
  { "pipe"                 , ROOM_MAINTENANCE_ROOM    , FOBJ_LEAK            },
  { "enormous mirror"      , ROOM_MIRROR_ROOM_2       , FOBJ_MIRROR2         },
  { "mirror"               , ROOM_MIRROR_ROOM_2       , FOBJ_MIRROR2         },
  { "reflection"           , ROOM_MIRROR_ROOM_2       , FOBJ_MIRROR2         },
  { "enormous mirror"      , ROOM_MIRROR_ROOM_1       , FOBJ_MIRROR1         },
  { "mirror"               , ROOM_MIRROR_ROOM_1       , FOBJ_MIRROR1         },
  { "reflection"           , ROOM_MIRROR_ROOM_1       , FOBJ_MIRROR1         },
  { "ancient inscription"  , ROOM_NORTH_TEMPLE        , FOBJ_PRAYER          },
  { "ancient prayer"       , ROOM_NORTH_TEMPLE        , FOBJ_PRAYER          },
  { "inscription"          , ROOM_NORTH_TEMPLE        , FOBJ_PRAYER          },
  { "prayer"               , ROOM_NORTH_TEMPLE        , FOBJ_PRAYER          },
  { "wooden railing"       , ROOM_DOME_ROOM           , FOBJ_RAILING         },
  { "wooden rail"          , ROOM_DOME_ROOM           , FOBJ_RAILING         },
  { "railing"              , ROOM_DOME_ROOM           , FOBJ_RAILING         },
  { "rail"                 , ROOM_DOME_ROOM           , FOBJ_RAILING         },
  { "sand"                 , ROOM_SANDY_CAVE          , FOBJ_SAND            },
  { "switch"               , ROOM_MACHINE_ROOM        , FOBJ_MACHINE_SWITCH  },
  { "white pedestal"       , ROOM_TORCH_ROOM          , FOBJ_PEDESTAL        },
  { "marble pedestal"      , ROOM_TORCH_ROOM          , FOBJ_PEDESTAL        },
  { "pedestal"             , ROOM_TORCH_ROOM          , FOBJ_PEDESTAL        },
  { "control panel"        , ROOM_DAM_ROOM            , FOBJ_CONTROL_PANEL   },
  { "panel"                , ROOM_DAM_ROOM            , FOBJ_CONTROL_PANEL   },
  { "nails"                , ROOM_LIVING_ROOM         , FOBJ_NAILS           },
  { "nail"                 , ROOM_LIVING_ROOM         , FOBJ_NAILS           },
  { "granite wall"         , ROOM_TREASURE_ROOM       , FOBJ_GRANITE_WALL    },
  { "granite wall"         , ROOM_NORTH_TEMPLE        , FOBJ_GRANITE_WALL    },
  { "granite wall"         , ROOM_SLIDE_ROOM          , FOBJ_GRANITE_WALL    },
  { "iron chain"           , ROOM_SHAFT_ROOM          , FOBJ_CHAIN           },
  { "chain"                , ROOM_SHAFT_ROOM          , FOBJ_CHAIN           },
  { "iron chain"           , ROOM_LOWER_SHAFT         , FOBJ_CHAIN           },
  { "chain"                , ROOM_LOWER_SHAFT         , FOBJ_CHAIN           },
  { "gate"                 , ROOM_ENTRANCE_TO_HADES   , FOBJ_GATE            },
  { "gateway"              , ROOM_ENTRANCE_TO_HADES   , FOBJ_GATE            },
  { "door"                 , ROOM_STUDIO              , FOBJ_STUDIO_DOOR     },
  { "paint"                , ROOM_STUDIO              , FOBJ_STUDIO_DOOR     },
  { "chasm"                , ROOM_EAST_OF_CHASM       , FOBJ_CHASM           },
  { "chasm"                , ROOM_RESERVOIR_SOUTH     , FOBJ_CHASM           },
  { "chasm"                , ROOM_CHASM_ROOM          , FOBJ_CHASM           },
  { "lake"                 , ROOM_RESERVOIR_SOUTH     , FOBJ_LAKE            },
  { "lake"                 , ROOM_RESERVOIR_NORTH     , FOBJ_LAKE            },
  { "stream"               , ROOM_RESERVOIR           , FOBJ_STREAM          },
  { "stream"               , ROOM_STREAM_VIEW         , FOBJ_STREAM          },
  { "stream"               , ROOM_IN_STREAM           , FOBJ_STREAM          },
  { "gas"                  , ROOM_SMELLY_ROOM         , FOBJ_GAS             },
  { "gas"                  , ROOM_GAS_ROOM            , FOBJ_GAS             },
  { "odor"                 , ROOM_SMELLY_ROOM         , FOBJ_GAS             },
  { "odor"                 , ROOM_GAS_ROOM            , FOBJ_GAS             },

  {0, 0, 0}
};



struct NOUNPHRASETOOBJ_STRUCT
{
  const char *phrase;
  const unsigned short obj;

  int w1, w2, w3;
};



//phrase must contain a maximum of three words

//for matching to work, phrases must be listed in order of decreasing word size

//different objects can have the same phrase, but there must
//be larger phrases for them, so player can be more specific when needed

//words must be separated by exactly one space

//NOTE: if there is a phrase like "disk drive" and then phrases "disk" and drive",
//which represent two different objects, parser will ask player to be more specific,
//so it is best to avoid "disk drive" and use something like "storage drive"


// objects that have same phrase as fixed objects:
//   water

// objects that have same phrase but won't prompt player to be specific
// because they won't exist in same room at same time:
//   basket                                                (raised and lowered)
//   brass bell  bell                                      (cool and hot)
//   lantern  lamp                                         (unbroken and broken)
//   boat                                                  (uninflated, inflated, punctured)
//   large egg  jewel-encrusted egg  egg                   (unbroken and broken)
//   golden canary  gold canary  clockwork canary  canary  (unbroken and broken)
//   table                                                 (kitchen and attic)

// objects that have same phrase and will prompt player to be specific
// if they are in same room at same time:
//   lantern  lamp                                         (unbroken/broken and burned-out)
//   bag                                                   (sandwich bag, bag of coins, large bag)
//   knife                                                 (nasty and rusty)
//   pile                                                  (leaves, uninflated boat, coal)
//   book                                                  (black and guide)

struct NOUNPHRASETOOBJ_STRUCT NounPhraseToObj[] =
{
  { "me"                          , OBJ_YOU                },
  { "myself"                      , OBJ_YOU                },
  { "i"                           , OBJ_YOU                },
  { "self"                        , OBJ_YOU                },
  { "bare hands"                  , OBJ_YOU                },
  { "my hands"                    , OBJ_YOU                },
  { "my hand"                     , OBJ_YOU                },
  { "hands"                       , OBJ_YOU                },
  { "hand"                        , OBJ_YOU                },
  { "my teeth"                    , OBJ_YOU                },
  { "teeth"                       , OBJ_YOU                },
  { "cyclops"                     , OBJ_CYCLOPS            },
  { "evil spirits"                , OBJ_GHOSTS             },
  { "spirits"                     , OBJ_GHOSTS             },
  { "spirit"                      , OBJ_GHOSTS             },
  { "ghosts"                      , OBJ_GHOSTS             },
  { "ghost"                       , OBJ_GHOSTS             },
  { "vampire bat"                 , OBJ_BAT                },
  { "bat"                         , OBJ_BAT                },
  { "individual"                  , OBJ_THIEF              },
  { "man"                         , OBJ_THIEF              },
  { "person"                      , OBJ_THIEF              },
  { "thief"                       , OBJ_THIEF              },
  { "robber"                      , OBJ_THIEF              },
  { "nasty troll"                 , OBJ_TROLL              },
  { "troll"                       , OBJ_TROLL              },
  { "basket"                      , OBJ_LOWERED_BASKET     },
  { "basket"                      , OBJ_RAISED_BASKET      },
  { "trophy case"                 , OBJ_TROPHY_CASE        },
  { "case"                        , OBJ_TROPHY_CASE        },
  { "machine"                     , OBJ_MACHINE            },
  { "lid"                         , OBJ_MACHINE            },
  { "small mailbox"               , OBJ_MAILBOX            },
  { "mailbox"                     , OBJ_MAILBOX            },
  { "box"                         , OBJ_MAILBOX            },
  { "quantity of water"           , OBJ_WATER              },
  { "water"                       , OBJ_WATER              },
  { "crystal skull"               , OBJ_SKULL              },
  { "skull"                       , OBJ_SKULL              },
  { "broken timber"               , OBJ_TIMBERS            },
  { "timber"                      , OBJ_TIMBERS            },
  { "hot pepper sandwich"         , OBJ_LUNCH              },
  { "pepper sandwich"             , OBJ_LUNCH              },
  { "sandwich"                    , OBJ_LUNCH              },
  { "lunch"                       , OBJ_LUNCH              },
  { "brass bell"                  , OBJ_BELL               },
  { "bell"                        , OBJ_BELL               },
  { "red hot bell"                , OBJ_HOT_BELL           },
  { "hot brass bell"              , OBJ_HOT_BELL           },
  { "hot bell"                    , OBJ_HOT_BELL           },
  { "brass bell"                  , OBJ_HOT_BELL           },
  { "bell"                        , OBJ_HOT_BELL           },
  { "large book"                  , OBJ_BOOK               },
  { "black book"                  , OBJ_BOOK               },
  { "book"                        , OBJ_BOOK               },
  { "page"                        , OBJ_BOOK               },
  { "bloody axe"                  , OBJ_AXE                },
  { "bloody ax"                   , OBJ_AXE                },
  { "axe"                         , OBJ_AXE                },
  { "ax"                          , OBJ_AXE                },
  { "broken lantern"              , OBJ_BROKEN_LAMP        },
  { "broken lamp"                 , OBJ_BROKEN_LAMP        },
  { "lantern"                     , OBJ_BROKEN_LAMP        },
  { "lamp"                        , OBJ_BROKEN_LAMP        },
  { "ornamented sceptre"          , OBJ_SCEPTRE            },
  { "ornamented scepter"          , OBJ_SCEPTRE            },
  { "sceptre"                     , OBJ_SCEPTRE            },
  { "scepter"                     , OBJ_SCEPTRE            },
  { "elongated sack"              , OBJ_SANDWICH_BAG       },
  { "brown sack"                  , OBJ_SANDWICH_BAG       },
  { "elongated bag"               , OBJ_SANDWICH_BAG       },
  { "brown bag"                   , OBJ_SANDWICH_BAG       },
  { "sack"                        , OBJ_SANDWICH_BAG       },
  { "bag"                         , OBJ_SANDWICH_BAG       },
  { "engraved chalice"            , OBJ_CHALICE            },
  { "silver chalice"              , OBJ_CHALICE            },
  { "chalice"                     , OBJ_CHALICE            },
  { "clove of garlic"             , OBJ_GARLIC             },
  { "clove"                       , OBJ_GARLIC             },
  { "garlic"                      , OBJ_GARLIC             },
  { "crystal trident"             , OBJ_TRIDENT            },
  { "trident"                     , OBJ_TRIDENT            },
  { "glass bottle"                , OBJ_BOTTLE             },
  { "bottle"                      , OBJ_BOTTLE             },
  { "solid-gold coffin"           , OBJ_COFFIN             },
  { "gold coffin"                 , OBJ_COFFIN             },
  { "coffin"                      , OBJ_COFFIN             },
  { "hand-held pump"              , OBJ_PUMP               },
  { "air pump"                    , OBJ_PUMP               },
  { "pump"                        , OBJ_PUMP               },
  { "huge diamond"                , OBJ_DIAMOND            },
  { "enormous diamond"            , OBJ_DIAMOND            },
  { "diamond"                     , OBJ_DIAMOND            },
  { "exquisite figurine"          , OBJ_JADE               },
  { "jade figurine"               , OBJ_JADE               },
  { "jade"                        , OBJ_JADE               },
  { "figurine"                    , OBJ_JADE               },
  { "nasty-looking knife"         , OBJ_KNIFE              },
  { "nasty knife"                 , OBJ_KNIFE              },
  { "knife"                       , OBJ_KNIFE              },
  { "burned-out lantern"          , OBJ_BURNED_OUT_LANTERN },
  { "burned-out lamp"             , OBJ_BURNED_OUT_LANTERN },
  { "useless lantern"             , OBJ_BURNED_OUT_LANTERN },
  { "useless lamp"                , OBJ_BURNED_OUT_LANTERN },
  { "lantern"                     , OBJ_BURNED_OUT_LANTERN },
  { "lamp"                        , OBJ_BURNED_OUT_LANTERN },
  { "bag of coins"                , OBJ_BAG_OF_COINS       },
  { "leather bag"                 , OBJ_BAG_OF_COINS       },
  { "bag"                         , OBJ_BAG_OF_COINS       },
  { "coins"                       , OBJ_BAG_OF_COINS       },
  { "brass lantern"               , OBJ_LAMP               },
  { "brass lamp"                  , OBJ_LAMP               },
  { "lantern"                     , OBJ_LAMP               },
  { "lamp"                        , OBJ_LAMP               },
  { "light"                       , OBJ_LAMP               },
  { "large emerald"               , OBJ_EMERALD            },
  { "emerald"                     , OBJ_EMERALD            },
  { "small leaflet"               , OBJ_ADVERTISEMENT      },
  { "leaflet"                     , OBJ_ADVERTISEMENT      },
  { "mail"                        , OBJ_ADVERTISEMENT      },
  { "magic boat"                  , OBJ_INFLATED_BOAT      },
  { "inflated boat"               , OBJ_INFLATED_BOAT      },
  { "boat"                        , OBJ_INFLATED_BOAT      },
  { "match"                       , OBJ_MATCH              },
  { "matches"                     , OBJ_MATCH              },
  { "matchbook"                   , OBJ_MATCH              },
  { "beautiful painting"          , OBJ_PAINTING           },
  { "painting"                    , OBJ_PAINTING           },
  { "pair of candles"             , OBJ_CANDLES            },
  { "candles"                     , OBJ_CANDLES            },
  { "piece of slag"               , OBJ_GUNK               },
  { "vitreous slag"               , OBJ_GUNK               },
  { "slag"                        , OBJ_GUNK               },
  { "pile of leaves"              , OBJ_LEAVES             },
  { "leaves"                      , OBJ_LEAVES             },
  { "pile"                        , OBJ_LEAVES             },
  { "punctured boat"              , OBJ_PUNCTURED_BOAT     },
  { "boat"                        , OBJ_PUNCTURED_BOAT     },
  { "pile of plastic"             , OBJ_INFLATABLE_BOAT    },
  { "inflatable boat"             , OBJ_INFLATABLE_BOAT    },
  { "plastic"                     , OBJ_INFLATABLE_BOAT    },
  { "pile"                        , OBJ_INFLATABLE_BOAT    },
  { "boat"                        , OBJ_INFLATABLE_BOAT    },
  { "valve"                       , OBJ_INFLATABLE_BOAT    },
  { "bar of platinum"             , OBJ_BAR                },
  { "large bar"                   , OBJ_BAR                },
  { "platinum bar"                , OBJ_BAR                },
  { "bar"                         , OBJ_BAR                },
  { "pot of gold"                 , OBJ_POT_OF_GOLD        },
  { "pot"                         , OBJ_POT_OF_GOLD        },
  { "gold"                        , OBJ_POT_OF_GOLD        },
  { "red buoy"                    , OBJ_BUOY               },
  { "buoy"                        , OBJ_BUOY               },
  { "coil of rope"                , OBJ_ROPE               },
  { "coil"                        , OBJ_ROPE               },
  { "rope"                        , OBJ_ROPE               },
  { "rusty knife"                 , OBJ_RUSTY_KNIFE        },
  { "knife"                       , OBJ_RUSTY_KNIFE        },
  { "sapphire-encrusted bracelet" , OBJ_BRACELET           },
  { "sapphire bracelet"           , OBJ_BRACELET           },
  { "bracelet"                    , OBJ_BRACELET           },
  { "tool chests"                 , OBJ_TOOL_CHEST         },
  { "tool chest"                  , OBJ_TOOL_CHEST         },
  { "chests"                      , OBJ_TOOL_CHEST         },
  { "chest"                       , OBJ_TOOL_CHEST         },
  { "screwdriver"                 , OBJ_SCREWDRIVER        },
  { "skeleton key"                , OBJ_KEYS               },
  { "key"                         , OBJ_KEYS               },
  { "shovel"                      , OBJ_SHOVEL             },
  { "pile of coal"                , OBJ_COAL               },
  { "small pile"                  , OBJ_COAL               },
  { "pile"                        , OBJ_COAL               },
  { "coal"                        , OBJ_COAL               },
  { "beautiful scarab"            , OBJ_SCARAB             },
  { "jeweled scarab"              , OBJ_SCARAB             },
  { "scarab"                      , OBJ_SCARAB             },
  { "large bag"                   , OBJ_LARGE_BAG          },
  { "thief's bag"                 , OBJ_LARGE_BAG          },
  { "bag"                         , OBJ_LARGE_BAG          },
  { "vicious stiletto"            , OBJ_STILETTO           },
  { "stiletto"                    , OBJ_STILETTO           },
  { "elvish sword"                , OBJ_SWORD              },
  { "antique sword"               , OBJ_SWORD              },
  { "sword"                       , OBJ_SWORD              },
  { "ancient map"                 , OBJ_MAP                },
  { "ancient parchment"           , OBJ_MAP                },
  { "parchment map"               , OBJ_MAP                },
  { "map"                         , OBJ_MAP                },
  { "parchment"                   , OBJ_MAP                },
  { "tan label"                   , OBJ_BOAT_LABEL         },
  { "label"                       , OBJ_BOAT_LABEL         },
  { "ivory torch"                 , OBJ_TORCH              },
  { "torch"                       , OBJ_TORCH              },
  { "tour guidebook"              , OBJ_GUIDE              },
  { "tour guide"                  , OBJ_GUIDE              },
  { "guide book"                  , OBJ_GUIDE              },
  { "guidebook"                   , OBJ_GUIDE              },
  { "guide"                       , OBJ_GUIDE              },
  { "book"                        , OBJ_GUIDE              },
  { "trunk of jewels"             , OBJ_TRUNK              },
  { "old trunk"                   , OBJ_TRUNK              },
  { "trunk"                       , OBJ_TRUNK              },
  { "jewels"                      , OBJ_TRUNK              },
  { "tube of toothpaste"          , OBJ_TUBE               },
  { "toothpaste tube"             , OBJ_TUBE               },
  { "toothpaste"                  , OBJ_TUBE               },
  { "tube"                        , OBJ_TUBE               },
  { "viscous material"            , OBJ_PUTTY              },
  { "material"                    , OBJ_PUTTY              },
  { "gunk"                        , OBJ_PUTTY              },
  { "glue"                        , OBJ_PUTTY              },
  { "piece of paper"              , OBJ_OWNERS_MANUAL      },
  { "zork manual"                 , OBJ_OWNERS_MANUAL      },
  { "owner's manual"              , OBJ_OWNERS_MANUAL      },
  { "manual"                      , OBJ_OWNERS_MANUAL      },
  { "paper"                       , OBJ_OWNERS_MANUAL      },
  { "wrench"                      , OBJ_WRENCH             },
  { "small nest"                  , OBJ_NEST               },
  { "bird's nest"                 , OBJ_NEST               },
  { "bird nest"                   , OBJ_NEST               },
  { "nest"                        , OBJ_NEST               },
  { "large egg"                   , OBJ_EGG                },
  { "jewel-encrusted egg"         , OBJ_EGG                },
  { "egg"                         , OBJ_EGG                },
  { "broken egg"                  , OBJ_BROKEN_EGG         },
  { "ruined egg"                  , OBJ_BROKEN_EGG         },
  { "large egg"                   , OBJ_BROKEN_EGG         },
  { "jewel-encrusted egg"         , OBJ_BROKEN_EGG         },
  { "egg"                         , OBJ_BROKEN_EGG         },
  { "beautiful bauble"            , OBJ_BAUBLE             },
  { "brass bauble"                , OBJ_BAUBLE             },
  { "bauble"                      , OBJ_BAUBLE             },
  { "golden canary"               , OBJ_CANARY             },
  { "gold canary"                 , OBJ_CANARY             },
  { "clockwork canary"            , OBJ_CANARY             },
  { "canary"                      , OBJ_CANARY             },
  { "broken canary"               , OBJ_BROKEN_CANARY      },
  { "golden canary"               , OBJ_BROKEN_CANARY      },
  { "gold canary"                 , OBJ_BROKEN_CANARY      },
  { "clockwork canary"            , OBJ_BROKEN_CANARY      },
  { "canary"                      , OBJ_BROKEN_CANARY      },
  { "old engravings"              , OBJ_ENGRAVINGS         },
  { "old engraving"               , OBJ_ENGRAVINGS         },
  { "engravings"                  , OBJ_ENGRAVINGS         },
  { "engraving"                   , OBJ_ENGRAVINGS         },
  { "lurking grues"               , OBJ_GRUE               },
  { "lurking grue"                , OBJ_GRUE               },
  { "grues"                       , OBJ_GRUE               },
  { "grue"                        , OBJ_GRUE               },
  { "zorkmids"                    , OBJ_ZORKMID            },
  { "zorkmid"                     , OBJ_ZORKMID            },
  { "kitchen table"               , OBJ_KITCHEN_TABLE      },
  { "table"                       , OBJ_KITCHEN_TABLE      },
  { "table"                       , OBJ_ATTIC_TABLE        },

  { 0 , 0 }
};



struct VERBTOACTION_STRUCT
{
  const char *phrase;
  const unsigned short action;

  int w1, w2;
};



//phrase must contain a maximum of two words

//for matching to work, verb with adverb must come before same verb with no adverb

//words must be separated by exactly one space

//verbs don't need to (but should) be alphabetized

struct VERBTOACTION_STRUCT VerbToAction[] =
{
  //two words (verb-adverb)

  { "blow out"      , A_DEACTIVATE     },
  { "blow up"       , A_INFLATE        },
  { "break open"    , A_BREAK          },
  { "burn down"     , A_ACTIVATE       },
  { "cast out"      , A_EXORCISE       },
  { "climb down"    , A_CLIMBDOWN      },
  { "climb in"      , A_ENTER          },
  { "climb into"    , A_ENTER          },
  { "climb on"      , A_MOUNT          },
  { "climb onto"    , A_MOUNT          },
  { "climb out"     , A_EXIT           },
  { "climb through" , A_CLIMBTHROUGH   },
  { "climb up"      , A_CLIMBUP        },
  { "dig in"        , A_DIG            },
  { "drive away"    , A_EXORCISE       },
  { "drive out"     , A_EXORCISE       },
  { "gaze at"       , A_EXAMINE        },
  { "gaze behind"   , A_LOOKBEHIND     },
  { "gaze in"       , A_LOOKIN         },
  { "gaze inside"   , A_LOOKIN         },
  { "gaze into"     , A_LOOKIN         },
  { "gaze on"       , A_LOOKON         },
  { "gaze through"  , A_LOOKTHROUGH    },
  { "gaze under"    , A_LOOKUNDER      },
  { "get in"        , A_ENTER          },
  { "get inside"    , A_ENTER          },
  { "get into"      , A_ENTER          },
  { "get off"       , A_DISMOUNT       },
  { "get on"        , A_MOUNT          },
  { "get out"       , A_EXIT           },
  { "go inside"     , A_ENTER          },
  { "go into"       , A_ENTER          },
  { "go through"    , A_CLIMBTHROUGH   },
  { "knock on"      , A_KNOCK          },
  { "knock with"    , A_KNOCK          },
  { "l at"          , A_EXAMINE        },
  { "l behind"      , A_LOOKBEHIND     },
  { "l in"          , A_LOOKIN         },
  { "l inside"      , A_LOOKIN         },
  { "l into"        , A_LOOKIN         },
  { "l on"          , A_LOOKON         },
  { "l through"     , A_LOOKTHROUGH    },
  { "l under"       , A_LOOKUNDER      },
  { "lay on"        , A_SLEEPON        },
  { "lift up"       , A_RAISE          },
  { "listen to"     , A_LISTENTO       },
  { "listen for"    , A_LISTENTO       },
  { "look at"       , A_EXAMINE        },
  { "look behind"   , A_LOOKBEHIND     },
  { "look in"       , A_LOOKIN         },
  { "look inside"   , A_LOOKIN         },
  { "look into"     , A_LOOKIN         },
  { "look on"       , A_LOOKON         },
  { "look through"  , A_LOOKTHROUGH    },
  { "look under"    , A_LOOKUNDER      },
  { "open up"       , A_OPEN           },
  { "pick up"       , A_TAKE           },
  { "pump up"       , A_INFLATE        },
  { "put down"      , A_DROP           },
  { "put on"        , A_WEAR           },
  { "put out"       , A_DEACTIVATE     },
  { "raise up"      , A_RAISE          },
  { "read from"     , A_READ           },
  { "sit in"        , A_ENTER          },
  { "sleep on"      , A_SLEEPON        },
  { "slide down"    , A_SLIDEDOWN      },
  { "slide up"      , A_SLIDEUP        },
  { "stand on"      , A_MOUNT          },
  { "stand up"      , A_DISEMBARK      },
  { "stare at"      , A_EXAMINE        },
  { "stare behind"  , A_LOOKBEHIND     },
  { "stare in"      , A_LOOKIN         },
  { "stare inside"  , A_LOOKIN         },
  { "stare into"    , A_LOOKIN         },
  { "stare on"      , A_LOOKON         },
  { "stare through" , A_LOOKTHROUGH    },
  { "stare under"   , A_LOOKUNDER      },
  { "swim in"       , A_SWIM           },
  { "swim across"   , A_SWIM           },
  { "take off"      , A_REMOVE         },
  { "take out"      , A_TAKE           },
  { "talk to"       , A_TALKTO         },
  { "tie up"        , A_TIE            },
  { "turn off"      , A_DEACTIVATE     },
  { "turn on"       , A_ACTIVATE       },
  { "walk through"  , A_CLIMBTHROUGH   },
  { "what is"       , A_EXAMINE        },
  { "what are"      , A_EXAMINE        },
  { "where is"      , A_WHEREIS        },
  { "where am"      , A_WHEREIS        },
  { "where are"     , A_WHEREIS        },
  { "who am"        , A_EXAMINE        },
  { "who is"        , A_EXAMINE        },
  { "wind up"       , A_WIND           },

  //one word

  { "acquire"       , A_TAKE           },
  { "activate"      , A_ACTIVATE       },
  { "advance"       , A_GO             },
  { "ascend"        , A_CLIMBUP        },
  { "ask"           , A_TALKTO         },
  { "attach"        , A_TIE            },
  { "attack"        , A_ATTACK         },
  { "banish"        , A_EXORCISE       },
  { "bathe"         , A_SWIM           },
  { "begone"        , A_EXORCISE       },
  { "bite"          , A_EAT            },
  { "board"         , A_ENTER          },
  { "brandish"      , A_WAVE           },
  { "break"         , A_BREAK          },
  { "brief"         , A_BRIEF          },
  { "brush"         , A_BRUSH          },
  { "burn"          , A_ACTIVATE       },
  { "carry"         , A_TAKE           },
  { "catch"         , A_TAKE           },
  { "chuck"         , A_THROW          },
  { "climb"         , A_CLIMB          },
  { "close"         , A_CLOSE          },
  { "consume"       , A_EAT            },
  { "count"         , A_COUNT          },
  { "cross"         , A_CROSS          },
  { "d"             , A_DOWN           },
  { "damage"        , A_BREAK          },
  { "deactivate"    , A_DEACTIVATE     },
  { "deflate"       , A_DEFLATE        },
  { "descend"       , A_CLIMBDOWN      },
  { "describe"      , A_EXAMINE        },
  { "destroy"       , A_BREAK          },
  { "diagnose"      , A_DIAGNOSE       },
  { "dig"           , A_DIG            },
  { "discard"       , A_DROP           },
  { "disembark"     , A_DISEMBARK      },
  { "dislodge"      , A_MOVE           },
  { "dispatch"      , A_ATTACK         },
  { "dive"          , A_JUMP           },
  { "donate"        , A_GIVE           },
  { "down"          , A_DOWN           },
  { "douse"         , A_DEACTIVATE     },
  { "drink"         , A_DRINK          },
  { "drop"          , A_DROP           },
  { "e"             , A_EAST           },
  { "east"          , A_EAST           },
  { "eat"           , A_EAT            },
  { "echo"          , A_ECHO           },
  { "empty"         , A_EMPTY          },
  { "enter"         , A_ENTER          },
  { "examine"       , A_EXAMINE        },
  { "excavate"      , A_DIG            },
  { "exit"          , A_EXIT           },
  { "exorcise"      , A_EXORCISE       },
  { "extinguish"    , A_DEACTIVATE     },
  { "fasten"        , A_TIE            },
  { "feed"          , A_GIVE           },
  { "feel"          , A_TOUCH          },
  { "fight"         , A_ATTACK         },
  { "fill"          , A_FILL           },
  { "find"          , A_WHEREIS        },
  { "fix"           , A_FIX            },
  { "flick"         , A_ACTIVATE       },
  { "flip"          , A_ACTIVATE       },
  { "ford"          , A_CROSS          },
  { "free"          , A_UNTIE          },
  { "gaze"          , A_LOOK           },
  { "get"           , A_TAKE           },
  { "give"          , A_GIVE           },
  { "glue"          , A_FIX            },
  { "go"            , A_GO             },
  { "grab"          , A_TAKE           },
  { "grease"        , A_OIL            },
  { "greet"         , A_GREET          },
  { "head"          , A_GO             },
  { "hello"         , A_GREET          },
  { "hide"          , A_PUT            },
  { "hit"           , A_ATTACK         },
  { "hold"          , A_TAKE           },
  { "hurl"          , A_THROW          },
  { "hurt"          , A_ATTACK         },
  { "i"             , A_INVENTORY      },
  { "ignite"        , A_ACTIVATE       },
  { "imbibe"        , A_DRINK          },
  { "in"            , A_IN             },
  { "incinerate"    , A_ACTIVATE       },
  { "inflate"       , A_INFLATE        },
  { "injure"        , A_ATTACK         },
  { "insert"        , A_PUT            },
  { "inspect"       , A_EXAMINE        },
  { "inventory"     , A_INVENTORY      },
  { "journey"       , A_GO             },
  { "jump"          , A_JUMP           },
  { "kill"          , A_ATTACK         },
  { "knock"         , A_KNOCK          },
  { "l"             , A_LOOK           },
  { "land"          , A_LAND           },
  { "launch"        , A_LAUNCH         },
  { "leap"          , A_JUMP           },
  { "leave"         , A_EXIT           },
  { "lift"          , A_RAISE          },
  { "light"         , A_ACTIVATE       },
  { "locate"        , A_WHEREIS        },
  { "lock"          , A_LOCK           },
  { "look"          , A_LOOK           },
  { "loosen"        , A_MOVE           },
  { "lower"         , A_LOWER          },
  { "lubricate"     , A_OIL            },
  { "mount"         , A_MOUNT          },
  { "move"          , A_MOVE           },
  { "murder"        , A_ATTACK         },
  { "n"             , A_NORTH          },
  { "ne"            , A_NORTHEAST      },
  { "north"         , A_NORTH          },
  { "northeast"     , A_NORTHEAST      },
  { "northwest"     , A_NORTHWEST      },
  { "nw"            , A_NORTHWEST      },
  { "obtain"        , A_TAKE           },
  { "odysseus"      , A_ODYSSEUS       },
  { "offer"         , A_GIVE           },
  { "oil"           , A_OIL            },
  { "open"          , A_OPEN           },
  { "out"           , A_OUT            },
  { "pat"           , A_TOUCH          },
  { "patch"         , A_FIX            },
  { "peal"          , A_RING           },
  { "pet"           , A_TOUCH          },
  { "place"         , A_PUT            },
  { "play"          , A_PLAY           },
  { "plug"          , A_FIX            },
  { "pour"          , A_POUR           },
  { "pray"          , A_PRAY           },
  { "press"         , A_PUSH           },
  { "proceed"       , A_GO             },
  { "pry"           , A_PRY            },
  { "pull"          , A_PULL           },
  { "pump"          , A_INFLATE        },
  { "push"          , A_PUSH           },
  { "put"           , A_PUT            },
  { "q"             , A_QUIT           },
  { "quit"          , A_QUIT           },
  { "raise"         , A_RAISE          },
  { "read"          , A_READ           },
  { "release"       , A_UNTIE          },
  { "remove"        , A_REMOVE         },
  { "repair"        , A_FIX            },
  { "restart"       , A_RESTART        },
  { "restore"       , A_RESTORE        },
  { "ring"          , A_RING           },
  { "rotate"        , A_TURN           },
  { "rub"           , A_TOUCH          },
  { "run"           , A_GO             },
  { "s"             , A_SOUTH          },
  { "save"          , A_SAVE           },
  { "say"           , A_SAY            },
  { "score"         , A_SCORE          },
  { "se"            , A_SOUTHEAST      },
  { "secure"        , A_TIE            },
  { "seek"          , A_WHEREIS        },
  { "set"           , A_PUT            },
  { "shut"          , A_CLOSE          },
  { "skim"          , A_READ           },
  { "slay"          , A_ATTACK         },
  { "sleep"         , A_SLEEP          },
  { "smash"         , A_BREAK          },
  { "smell"         , A_SMELL          },
  { "sniff"         , A_SMELL          },
  { "south"         , A_SOUTH          },
  { "southeast"     , A_SOUTHEAST      },
  { "southwest"     , A_SOUTHWEST      },
  { "spill"         , A_POUR           },
  { "spin"          , A_TURN           },
  { "squeeze"       , A_SQUEEZE        },
  { "stab"          , A_ATTACK         },
  { "stand"         , A_DISEMBARK      },
  { "stare"         , A_LOOK           },
  { "step"          , A_GO             },
  { "stop"          , A_FIX            },
  { "strike"        , A_ATTACK         },
  { "stuff"         , A_PUT            },
  { "superbrief"    , A_SUPERBRIEF     },
  { "sw"            , A_SOUTHWEST      },
  { "swallow"       , A_DRINK          },
  { "swim"          , A_SWIM           },
  { "swing"         , A_WAVE           },
  { "take"          , A_TAKE           },
  { "taste"         , A_EAT            },
  { "tell"          , A_TALKTO         },
  { "temple"        , A_TEMPLETREASURE },
  { "throw"         , A_THROW          },
  { "thrust"        , A_WAVE           },
  { "tie"           , A_TIE            },
  { "toss"          , A_THROW          },
  { "touch"         , A_TOUCH          },
  { "travel"        , A_GO             },
  { "treasure"      , A_TEMPLETREASURE },
  { "turn"          , A_TURN           },
  { "u"             , A_UP             },
  { "ulysses"       , A_ODYSSEUS       },
  { "unattach"      , A_UNTIE          },
  { "unfasten"      , A_UNTIE          },
  { "unhook"        , A_UNTIE          },
  { "unlock"        , A_UNLOCK         },
  { "untie"         , A_UNTIE          },
  { "up"            , A_UP             },
  { "verbose"       , A_VERBOSE        },
  { "version"       , A_VERSION        },
  { "w"             , A_WEST           },
  { "wade"          , A_SWIM           },
  { "wait"          , A_WAIT           },
  { "walk"          , A_GO             },
  { "wave"          , A_WAVE           },
  { "wear"          , A_WEAR           },
  { "west"          , A_WEST           },
  { "what"          , A_EXAMINE        },
  { "what's"        , A_EXAMINE        },
  { "whats"         , A_EXAMINE        },
  { "where's"       , A_WHEREIS        },
  { "wheres"        , A_WHEREIS        },
  { "who"           , A_EXAMINE        },
  { "who's"         , A_EXAMINE        },
  { "whos"          , A_EXAMINE        },
  { "wind"          , A_WIND           },
  { "x"             , A_EXAMINE        },
  { "z"             , A_WAIT           },

  {0, 0}
};



char WordList[2048][64];

int NumWords;

const char *InitWords[] =
{
  "!NULL",

  "a",
  "across",
  "again",
  "all",
  "an",
  "and",
  "at",
  "autoplay",
  "but",
  "everything",
  "except",
  "exit",
  "for",
  "from",
  "g",
  "in",
  "inside",
  "into",
  "it",
  "n",
  "no",
  "odysseus",
  "of",
  "off",
  "on",
  "onto",
  "oops",
  "out",
  "outside",
  "over",
  "restart",
  "restore",
  "the",
  "them",
  "then",
  "through",
  "to",
  "toward",
  "ulysses",
  "using",
  "with",
  "y",
  "yes",

  0
};

//-----------------------------------------------------------------------------



//*****************************************************************************

const char *AddWord(const char *p, int *w)
{
  int i, j, c1, c2;

  *w = 0;
  if (p == 0 || *p == 0 || *p == ' ') return 0;

  for (i=0; i<NumWords; i++)
    for (j=0; ; j++)
  {
    c1 = p[j];           if (c1 == ' ') c1 = 0;
    c2 = WordList[i][j]; if (c2 == ' ') c2 = 0;
    if (c1 != c2) break;

    if (p[j] == ' ') {*w = i; return p+j+1;}
    if (p[j] ==   0) {*w = i; return p+j;}
  }

  for (j=0; ; j++)
  {
    WordList[NumWords][j] = (p[j] == ' ') ? 0 : p[j];
    if (p[j] == ' ') {*w = NumWords++; return p+j+1;}
    if (p[j] ==   0) {*w = NumWords++; return p+j;}
  }
}



void CreateWordList(void)
{
  int i, w;
  const char *p;

  NumWords = 0;

  for (i=0; ; i++)
  {
    p = InitWords[i]; if (p == 0) break;
    AddWord(p, &w);
  }

  for (i=0; ; i++)
  {
    p = NounPhraseToFixedObj[i].phrase; if (p == 0) break;
    p = AddWord(p, &w); NounPhraseToFixedObj[i].w1 = w;
    p = AddWord(p, &w); NounPhraseToFixedObj[i].w2 = w;
  }

  for (i=0; ; i++)
  {
    p = NounPhraseToObj[i].phrase; if (p == 0) break;
    p = AddWord(p, &w); NounPhraseToObj[i].w1 = w;
    p = AddWord(p, &w); NounPhraseToObj[i].w2 = w;
    p = AddWord(p, &w); NounPhraseToObj[i].w3 = w;
  }

  for (i=0; ; i++)
  {
    p = VerbToAction[i].phrase; if (p == 0) break;
    p = AddWord(p, &w); VerbToAction[i].w1 = w;
    p = AddWord(p, &w); VerbToAction[i].w2 = w;
  }
}



void PrintWordList(void)
{
  FILE *f;
  int i, j;

  f = fopen("_vocab.cpp", "w");

  fputs("#include \"def.h\"\n\nextern const unsigned char WordList[] PROGMEM =\n{\n", f);
  for (i=0; i<NumWords; i++)
  {
    fputs("  ", f);
    for (j=0; j<strlen(WordList[i])+1; j++)
      fprintf(f, "%i,", WordList[i][j]);
    fputs("\n", f);
  }
  fputs("  0\n};\n\nextern const uint16_t NounPhraseToFixedObj[] PROGMEM =\n{\n", f);
  for (i=0; ; i++)
  {
    struct NOUNPHRASETOFIXEDOBJ_STRUCT *q = &NounPhraseToFixedObj[i];
    fprintf(f, "  %i,%i,%i,%i,\n", q->w1, q->w2, q->room, q->fobj);
    if (q->phrase == 0) break;
  }
  fputs("};\n\nextern const uint16_t NounPhraseToObj[] PROGMEM =\n{\n", f);
  for (i=0; ; i++)
  {
    struct NOUNPHRASETOOBJ_STRUCT *q = &NounPhraseToObj[i];
    fprintf(f, "  %i,%i,%i,%i,\n", q->w1, q->w2, q->w3, q->obj);
    if (q->phrase == 0) break;
  }
  fputs("};\n\nextern const uint16_t VerbToAction[] PROGMEM =\n{\n", f);
  for (i=0; ; i++)
  {
    struct VERBTOACTION_STRUCT *q = &VerbToAction[i];
    fprintf(f, "  %i,%i,%i,\n", q->w1, q->w2, q->action);
    if (q->phrase == 0) break;
  }
  fputs("};\n", f);

  fclose(f);
}

//*****************************************************************************



//#############################################################################

int main(void)
{
  CreateWordList();
  PrintWordList();

  return 0;
}

//#############################################################################
