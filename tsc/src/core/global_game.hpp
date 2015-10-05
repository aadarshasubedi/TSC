/***************************************************************************
 * global_game.h
 *
 * Copyright © 2003 - 2011 Florian Richter
 * Copyright © 2013 - 2014 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TSC_GLOBAL_GAME_HPP
#define TSC_GLOBAL_GAME_HPP

namespace TSC {

    /* Turns the version numbers into a numeric value:
     * (1,2,3) -> (10203)
     * Assumes that there will never be more than 100 minor or patch versions
     */
#define TSC_VERSION_NUM(X, Y, Z) ((X)*10000 + (Y)*100 + (Z))

// Caption
#define CAPTION "The Secret Chronicles of Dr. M."

    static const unsigned int tsc_version = TSC_VERSION_NUM(TSC_VERSION_MAJOR, TSC_VERSION_MINOR, TSC_VERSION_PATCH);

    /* *** *** *** *** *** *** *** Object Direction *** *** *** *** *** *** *** *** *** *** */

    enum ObjectDirection {
        // undefined
        DIR_UNDEFINED   = -1,

        // default
        DIR_LEFT    = 0,
        DIR_RIGHT   = 1,
        DIR_UP      = 2,
        DIR_DOWN    = 3,

        DIR_TOP     = 2,
        DIR_BOTTOM  = 3,

        // multi
        DIR_TOP_LEFT = 4,
        DIR_TOP_RIGHT = 5,
        DIR_BOTTOM_LEFT = 6,
        DIR_BOTTOM_RIGHT = 7,
        DIR_LEFT_TOP = 21,
        DIR_LEFT_BOTTOM = 22,
        DIR_RIGHT_TOP = 23,
        DIR_RIGHT_BOTTOM = 24,

        DIR_UP_LEFT = 4,
        DIR_UP_RIGHT = 5,
        DIR_DOWN_LEFT = 6,
        DIR_DOWN_RIGHT = 7,
        DIR_LEFT_UP = 21,
        DIR_LEFT_DOWN = 22,
        DIR_RIGHT_UP = 23,
        DIR_RIGHT_DOWN = 24,

        // extra
        DIR_HORIZONTAL  = 10,   // left or right
        DIR_VERTICAL    = 11,   // up or down
        DIR_ALL         = 20,   // all directions

        // special
        DIR_FIRST       = 100,  // Overworld first waypoint
        DIR_LAST        = 101   // Overworld last waypoint
    };

    /* *** *** *** *** *** *** *** Default Color *** *** *** *** *** *** *** *** *** *** */

    enum DefaultColor {
        COL_DEFAULT = -1,
        COL_WHITE   = 0,
        COL_BLACK   = 1,
        COL_RED     = 2,
        COL_ORANGE  = 3,
        COL_YELLOW  = 4,
        COL_GREEN   = 5,
        COL_BLUE    = 6,
        COL_BROWN   = 7,
        COL_GREY    = 8,
        COL_VIOLET  = 9
    };

    /* *** *** *** *** *** *** *** Game Mode *** *** *** *** *** *** *** *** *** *** */

    enum GameMode {
        MODE_NOTHING        = 0,
        MODE_LEVEL          = 1,
        MODE_OVERWORLD      = 2,
        MODE_MENU           = 3,
        MODE_LEVEL_SETTINGS = 4
    };

    enum GameModeType {
        MODE_TYPE_DEFAULT       = 0,
        MODE_TYPE_LEVEL_CUSTOM  = 1
    };

    /* *** *** *** *** *** *** *** Game Action *** *** *** *** *** *** *** *** *** *** */

    enum GameAction {
        GA_NONE             = 0,
        GA_ENTER_MENU       = 1,
        GA_ENTER_WORLD      = 2,
        GA_ENTER_LEVEL      = 3,
        GA_DOWNGRADE_PLAYER = 4,
        GA_ACTIVATE_LEVEL_EXIT  = 5,
        GA_ENTER_LEVEL_SETTINGS = 6,
        GA_RESET                = 7
    };

    /* *** *** *** *** *** Level draw type *** *** *** *** *** *** *** *** *** *** *** *** */

    enum LevelDrawType {
        LVL_DRAW        = 0,    // only draw
        LVL_DRAW_NO_BG  = 1,    // only draws and without background gradient and image
        LVL_DRAW_BG     = 2     // only draws the background gradient and image
    };

    /* *** *** *** *** *** Level land type *** *** *** *** *** *** *** *** *** *** *** *** */

    enum LevelLandType {
        LLT_UNDEFINED = 0,
        LLT_GREEN = 1,
        LLT_JUNGLE = 2,
        LLT_ICE = 3,
        LLT_SNOW = 4,
        LLT_WATER = 5,
        LLT_CANDY = 6,
        LLT_DESERT = 7,
        LLT_SAND = 8,
        LLT_CASTLE = 9,
        LLT_UNDERGROUND = 10,
        LLT_CRYSTAL = 11,
        LLT_GHOST = 12,
        LLT_MUSHROOM = 13,
        LLT_SKY = 14,
        LLT_PLASTIC = 15,
        LLT_LAST = 16
    };

    /* *** *** *** *** *** *** *** Movement states *** *** *** *** *** *** *** *** *** *** */

    enum Moving_state {
        STA_STAY    = 0,
        STA_WALK    = 1,
        // This one is abused special movements
        STA_RUN     = 2,
        STA_FALL    = 3,
        STA_FLY     = 4,
        STA_JUMP    = 5,
        STA_CLIMB   = 6,
        // another object controls this object ( object doesn't update itself )
        STA_OBJ_LINKED = 7
    };

    /* *** *** *** *** *** *** *** Paths *** *** *** *** *** *** *** *** *** *** */

// Core
#define GAME_OVERWORLD_DIR "worlds"
#define GAME_LEVEL_DIR "levels"
#define GAME_CAMPAIGN_DIR "campaigns"
#define GAME_PIXMAPS_DIR "pixmaps"
#define GAME_SOUNDS_DIR "sounds"
#define GAME_MUSIC_DIR "music"
#define GAME_EDITOR_DIR "editor"
#define GAME_ICON_DIR "icon"
#define GAME_SCHEMA_DIR "schema"
#define GAME_TRANSLATION_DIR "translations"
#define GAME_SCRIPTING_DIR "scripting"
// GUI
#define GUI_SCHEME_DIR "gui/schemes"
#define GUI_IMAGESET_DIR "gui/imagesets"
#define GUI_FONT_DIR "gui/font"
#define GUI_LAYOUT_DIR "gui/layout"
#define GUI_LOOKNFEEL_DIR "gui/looknfeel"
// User
#define USER_SAVEGAME_DIR "savegames"
#define USER_SCREENSHOT_DIR "screenshots"
#define USER_LEVEL_DIR "levels"
#define USER_WORLD_DIR "worlds"
#define USER_CAMPAIGN_DIR "campaigns"
#define USER_IMGCACHE_DIR "images"

    /* *** *** *** *** *** *** *** forward declarations *** *** *** *** *** *** *** *** *** *** */

// Allows use of pointers in header files without including individual headers
// which decreases dependencies between files

    /* *** speedfactor framerate *** */
    static const int speedfactor_fps = 32;

    /* *** level engine version *** */
    static const int level_engine_version = 47;
    /* *** world engine version *** */
    static const int world_engine_version = 5;

    /* *** Sprite Types *** */

    enum SpriteType {
        TYPE_UNDEFINED = 0,
        // global
        TYPE_SPRITE = 1,
        TYPE_ENEMY = 2,
        TYPE_PLAYER = 3,
        TYPE_ACTIVE_SPRITE = 4,
        // game
        TYPE_MOUSECURSOR = 100,
        // overworld
        TYPE_OW_WAYPOINT = 55,
        TYPE_OW_LINE_START = 57,
        TYPE_OW_LINE_END = 58,
        // level
        TYPE_LEVEL_EXIT = 18,
        TYPE_LEVEL_ENTRY = 54,
        TYPE_ENEMY_STOPPER = 20,
        TYPE_BONUS_BOX = 26,
        TYPE_SPIN_BOX = 27,
        TYPE_TEXT_BOX = 59,
        TYPE_MOVING_PLATFORM = 38,
        TYPE_CRATE = 71,
        // enemy
        TYPE_FURBALL = 10,
        TYPE_FURBALL_BOSS = 62,
        TYPE_ARMY = 19,
        TYPE_TURTLE_BOSS = 56,
        TYPE_FLYON = 29,
        TYPE_ROKKO = 30,
        TYPE_KRUSH = 36,
        TYPE_THROMP = 41,
        TYPE_EATO = 42,
        TYPE_GEE = 43,
        TYPE_SPIKA = 31,
        TYPE_STATIC_ENEMY = 50,
        TYPE_SPIKEBALL = 64,
        TYPE_PIP = 67,
        TYPE_BEETLE_BARRAGE = 68,
        TYPE_BEETLE = 69,
        TYPE_SHELL = 70,
        TYPE_LARRY = 72,
        // items
        TYPE_POWERUP = 23,
        TYPE_MUSHROOM_DEFAULT = 25,
        TYPE_MUSHROOM_LIVE_1 = 35,
        TYPE_MUSHROOM_POISON = 49,
        TYPE_MUSHROOM_BLUE = 51,
        TYPE_MUSHROOM_GHOST = 52,
        TYPE_FIREPLANT = 24,
        TYPE_JUMPING_GOLDPIECE = 22,
        TYPE_FALLING_GOLDPIECE = 48,
        TYPE_GOLDPIECE = 8,
        TYPE_MOON = 37,
        TYPE_STAR = 39,
        // special
        TYPE_BALL = 28,
        TYPE_SOUND = 60,
        TYPE_ANIMATION = 61,
        TYPE_PARTICLE_EMITTER = 65,
        TYPE_PATH = 63,
        // HUD
        TYPE_HUD_POINTS = 12,
        TYPE_HUD_TIME = 13,
        TYPE_HUD_DEBUG = 14,
        TYPE_HUD_LIFE = 15,
        TYPE_HUD_GOLD = 16,
        TYPE_HUD_BACKGROUND = 17,
        TYPE_HUD_ITEMBOX = 32,
        TYPE_HUD_INFOMESSAGE = 66
    };

    /* *** Ground Types *** */

    enum GroundType {
        GROUND_NORMAL = 0,
        GROUND_EARTH = 1,
        GROUND_ICE = 2,
        GROUND_SAND = 3,
        GROUND_STONE = 4,
        GROUND_PLASTIC = 5
    };

    // MassiveType has been replaced by CollisionType

    /**
     * Determines how the actor behaves in the collision detection
     * mechanism, and how the object behaves when the player tries
     * to enter it (e.g. a player can jump through an halfmassive
     * object).
     *
     * Z positioning is managed by use of the ZLayer constants in actor.hpp.
     **/
    enum CollisionType {
        COLTYPE_MASSIVE = 1,
        COLTYPE_PASSIVE,
        COLTYPE_ENEMY,
        COLTYPE_ACTIVE,
        COLTYPE_ANIM,
        COLTYPE_PLAYER,
        COLTYPE_LAVA,
        COLTYPE_HALFMASSIVE,
        COLTYPE_FRONTPASSIVE,
        COLTYPE_CLIMBABLE
    };

    /* *** Array Types *** */
// Array types define how an object behaves on collisions.

    /* replaced by cActor::CollisionType
    enum ArrayType {
        ARRAY_UNDEFINED = 0,
        // normal blocking object (level default)
        ARRAY_MASSIVE = 1,
        // normal passive object
        ARRAY_PASSIVE = 2,
        // enemy
        ARRAY_ENEMY = 3,
        // special object
        ARRAY_ACTIVE = 4,
        // hud
        ARRAY_HUD = 5,
        // animation
        ARRAY_ANIM = 6,
        // player
        ARRAY_PLAYER = 7,
        // lava
        ARRAY_LAVA = 8
        }; */

    /* *** collision validation types *** */

    enum Col_Valid_Type {
        // not valid
        COL_VTYPE_NOT_VALID = 0,
        // internal
        COL_VTYPE_INTERNAL = 1,
        // blocking
        COL_VTYPE_BLOCKING = 2,
        // no validation possible for a sub-function
        COL_VTYPE_NOT_POSSIBLE = 3
    };

    /* *** Input identifier *** */

    enum input_identifier {
        INP_UNKNOWN = 0,
        INP_UP = 1,
        INP_DOWN = 2,
        INP_LEFT = 3,
        INP_RIGHT = 4,
        INP_JUMP = 5,
        INP_SHOOT = 6,
        INP_ACTION = 7,
        // Request Item
        INP_ITEM = 8,
        // General Exit/Leave/Cancel
        INP_EXIT = 9
    };

    /* *** Menu Types *** */

    enum MenuID {
        MENU_NOTHING = 0,
        MENU_MAIN = 1,
        MENU_START = 5,
        MENU_OPTIONS = 2,
        MENU_LOAD = 3,
        MENU_SAVE = 4,
        MENU_CREDITS = 6
    };

    /* *** Ball Effect types *** */

    enum ball_effect {
        FIREBALL_DEFAULT = 1,
        FIREBALL_EXPLOSION = 2,
        ICEBALL_DEFAULT = 3,
        ICEBALL_EXPLOSION = 4
    };

    /* *** Performance timer types *** */

    enum performance_timer_type {
        // update
        PERF_UPDATE_PROCESS_INPUT = 0,
        PERF_UPDATE_LEVEL = 1,
        PERF_UPDATE_LEVEL_EDITOR = 2,
        PERF_UPDATE_HUD = 3,
        PERF_UPDATE_PLAYER = 4,
        PERF_UPDATE_PLAYER_COLLISIONS = 23,
        PERF_UPDATE_LATE_LEVEL = 22,
        PERF_UPDATE_LEVEL_COLLISIONS = 5,
        PERF_UPDATE_CAMERA = 6,
        // update overworld
        PERF_UPDATE_OVERWORLD = 17,
        // update menu
        PERF_UPDATE_MENU = 18,
        // update level settings
        PERF_UPDATE_LEVEL_SETTINGS = 19,
        // draw level
        PERF_DRAW_LEVEL_LAYER1 = 7,
        PERF_DRAW_LEVEL_PLAYER = 8,
        PERF_DRAW_LEVEL_LAYER2 = 9,
        PERF_DRAW_LEVEL_HUD = 10,
        PERF_DRAW_LEVEL_EDITOR = 11,
        // draw overworld
        PERF_DRAW_OVERWORLD = 16,
        // draw menu
        PERF_DRAW_MENU = 14,
        // draw level settings
        PERF_DRAW_LEVEL_SETTINGS = 15,
        // draw
        PERF_DRAW_MOUSE = 12,
        // rendering
        PERF_RENDER_GAME = 13,
        PERF_RENDER_GUI = 20,
        PERF_RENDER_BUFFER = 21
    };

    /* *** Classes *** */

    // Do not remove the CLASS FORWARD comments below. They’re used by
    // the rake task that generates this class list.

    /** CLASS FORWARD DECLRATIONS START **/

    class cActor;
    class cAnimation;
    class cAnimation_Fireball;
    class cAnimation_Goldpiece;
    class cAnimation_List;
    class cAnimation_Manager;
    class cApp;
    class cArmy;
    class cAudio;
    class cAudio_Sound;
    class cBackground;
    class cBackground_Manager;
    class cBall;
    class cBaseBox;
    class cBeetle;
    class cBeetleBarrage;
    class cBonusBox;
    class cCamera;
    class cCampaign;
    class cCampaignLoader;
    class cCampaign_Manager;
    class cCircle_Request;
    class cClear_Request;
    class cCollidingSprite;
    class cCollision;
    class cCopyObject;
    class cCrate;
    class cDebugDisplay;
    class cDialogBox;
    class cDialogBox_Question;
    class cDialogBox_Text;
    class cDowngrade_Event;
    class cEato;
    class cEditor;
    class cEditorItemsLoader;
    class cEditor_CEGUI_Texture;
    class cEditor_Item_Object;
    class cEditor_Level;
    class cEditor_Menu_Object;
    class cEditor_Object_Settings_Item;
    class cEditor_World;
    class cEnemy;
    class cEnemyStopper;
    class cEvent;
    class cFGoldpiece;
    class cFile_parser;
    class cFirePlant;
    class cFlyon;
    class cFont_Manager;
    class cFramerate;
    class cFurball;
    class cGL_Surface;
    class cGee;
    class cGoldDisplay;
    class cGoldpiece;
    class cGradient_Request;
    class cHudSprite;
    class cHud_Manager;
    class cImageSet;
    class cImage_Manager;
    class cImage_Settings_Data;
    class cImage_Settings_Parser;
    class cInfoMessage;
    class cItemBox;
    class cJGoldpiece;
    class cJoystick;
    class cKeyDown_Event;
    class cKeyboard;
    class cKrush;
    class cLarry;
    class cLava;
    class cLayer;
    class cLayer_Line_Point;
    class cLayer_Line_Point_Start;
    class cLevel;
    class cLevelLoader;
    class cLevelScene;
    class cLevel_Entry;
    class cLevel_Exit;
    class cLevel_Load_Event;
    class cLevel_Manager;
    class cLevel_Player;
    class cLevel_Save_Event;
    class cLevel_Settings;
    class cLine_Request;
    class cLine_collision;
    class cLiveDisplay;
    class cMRuby_Interpreter;
    class cMenuBackground;
    class cMenuCore;
    class cMenuHandler;
    class cMenuScene;
    class cMenu_Base;
    class cMenu_Credits;
    class cMenu_Item;
    class cMenu_Main;
    class cMenu_Options;
    class cMenu_Savegames;
    class cMenu_Start;
    class cMoon;
    class cMouseCursor;
    class cMovingSprite;
    class cMoving_Platform;
    class cMushroom;
    class cObjectCollision;
    class cObjectCollisionType;
    class cObjectCollision_List;
    class cOverworld;
    class cOverworldDescriptionLoader;
    class cOverworldLayerLoader;
    class cOverworldLoader;
    class cOverworld_Manager;
    class cOverworld_Player;
    class cOverworld_description;
    class cPackage_Loader;
    class cPackage_Manager;
    class cParticle;
    class cParticle_Emitter;
    class cPath;
    class cPath_Segment;
    class cPath_State;
    class cPerformance_Timer;
    class cPip;
    class cPlayer;
    class cPlayerPoints;
    class cPowerUp;
    class cPreferences;
    class cPreferencesLoader;
    class cRandom_Sound;
    class cRect_Request;
    class cRenderQueue;
    class cRender_Request;
    class cRender_Request_Advanced;
    class cResource_Manager;
    class cRokko;
    class cSave;
    class cSave_Level;
    class cSave_Level_Object;
    class cSave_Level_Object_Property;
    class cSave_Overworld;
    class cSave_Overworld_Waypoint;
    class cSave_Player_Return_Entry;
    class cSavegame;
    class cSavegameLoader;
    class cScene;
    class cSceneManager;
    class cSelectedObject;
    class cShell;
    class cShoot_Event;
    class cSimpleImageSet;
    class cSound;
    class cSound_Manager;
    class cSpika;
    class cSpikeball;
    class cSpinBox;
    class cSprite;
    class cSpriteActor;
    class cAnimatedActor;
    class cSprite_List;
    class cSprite_Manager;
    class cStaticEnemy;
    class cStatusText;
    class cSurface_Request;
    class cText_Box;
    class cThromp;
    class cTimeDisplay;
    class cTimer;
    class cTouch_Event;
    class cTurtleBoss;
    class cVideo;
    class cWaypoint;
    class cWorld_Sprite_Manager;
    class XmlAttributes;

    /** CLASS FORWARD DECLRATIONS END **/

    /* *** Things the rake task fails to detect *** */
    struct ConfiguredTexture;

    namespace Scripting {
        class cScriptable_Object;
    }

    /* *** Shortcuts *** */

    /* Semantically, this should be a member variable of cLevel_Scene,
     * however, as it is needed at several places it’d be rather difficult
     * and cumbersome to always obtain the entire chain from the global
     * TSC app pointer down to the current level. This variable allows
     * to shortcut that. It is managed by cLevel_Scene as if it was
     * a member variable, though.
     */
    extern cLevel* gp_current_level;

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif
