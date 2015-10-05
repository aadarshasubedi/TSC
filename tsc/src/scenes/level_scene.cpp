#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/bintree.hpp"
#include "../core/property_helper.hpp"
#include "../core/errors.hpp"
#include "../core/xml_attributes.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../core/collision.hpp"
#include "../level/level.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_set.hpp"
#include "../objects/actor.hpp"
#include "../objects/sprite_actor.hpp"
#include "scene.hpp"
#include "../core/scene_manager.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/filesystem/package_manager.hpp"
#include "../video/img_manager.hpp"
#include "../user/preferences.hpp"
#include "../core/tsc_app.hpp"
#include "../objects/animated_actor.hpp"
#include "../level/level_player.hpp"
#include "level_scene.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

cLevelScene::cLevelScene(const std::string& startlevel)
{
#ifdef _DEBUG
    if (startlevel == "testlevel")
        Add_Level(cLevel::Construct_Debugging_Level());
    else
        Add_Level(startlevel);
#else
    Add_Level(startlevel);
#endif

    // TODO: Move this into HUD
    m_debug_playerpos_font.loadFromFile(path_to_utf8(gp_app->Get_ResourceManager().Get_Gui_Font("default.ttf")));
    m_debug_playerpos_text.setFont(m_debug_playerpos_font);
    m_debug_playerpos_text.setCharacterSize(12);
    m_debug_playerpos_text.setColor(sf::Color::Yellow);
    m_debug_playerpos_text.setString("(0|0)");
}

cLevelScene::~cLevelScene()
{
    std::vector<cLevel*>::iterator iter;
    for(iter=m_active_levels.begin(); iter != m_active_levels.end(); iter++)
        delete *iter;

    gp_app->Get_ImageManager().Clear();
}

void cLevelScene::Handle_Event(sf::Event& evt)
{
    if (evt.type == sf::Event::KeyPressed)
        Handle_Keydown_Event(evt);
    else if (evt.type == sf::Event::KeyReleased)
        Handle_Keyup_Event(evt);
}

void cLevelScene::Handle_Keydown_Event(sf::Event& evt)
{
    cLevel_Player* p_player = gp_current_level->Get_Player();
    cPreferences& preferences = gp_app->Get_Preferences();

    // (this is the old cLevel::KeyDown() method)
    // TODO: It is not completely converted yet!
    if (evt.key.code == sf::Keyboard::F8) {
        // open level editor
    }
    else if (evt.key.code == preferences.m_key_shoot) {
        /*
        Scripting::cKeyDown_Event evt("shoot");
        evt.Fire(m_mruby, pKeyboard);
        p_player->Action_Shoot();
        */
    }
    else if (evt.key.code == preferences.m_key_jump) {
        // OLD Scripting::cKeyDown_Event evt("jump");
        // OLD evt.Fire(m_mruby, pKeyboard);
        p_player->Action_Jump();
    }
    else if (evt.key.code == preferences.m_key_action) {
        /*
        Scripting::cKeyDown_Event evt("action");
        evt.Fire(m_mruby, pKeyboard);
        */
        p_player->Action_Interact(INP_ACTION);
    }
    else if (evt.key.code == preferences.m_key_up) {
        /*
        Scripting::cKeyDown_Event evt("up");
        evt.Fire(m_mruby, pKeyboard);
        */
        p_player->Action_Interact(INP_UP);
    }
    else if (evt.key.code == preferences.m_key_down) {
        /*
        Scripting::cKeyDown_Event evt("down");
        evt.Fire(m_mruby, pKeyboard);
        */
        p_player->Action_Interact(INP_DOWN);
    }
    else if (evt.key.code == preferences.m_key_left) {
        /*
        Scripting::cKeyDown_Event evt("left");
        evt.Fire(m_mruby, pKeyboard);
        */
        p_player->Action_Interact(INP_LEFT);
    }
    else if (evt.key.code == preferences.m_key_right) {
        /*
        Scripting::cKeyDown_Event evt("right");
        evt.Fire(m_mruby, pKeyboard);
        */
        p_player->Action_Interact(INP_RIGHT);
    }
    else if (evt.key.code == preferences.m_key_item) {
        /*
        Scripting::cKeyDown_Event evt("item");
        evt.Fire(m_mruby, pKeyboard);
        */
        p_player->Action_Interact(INP_ITEM);
    }
    // The following key combinations should belong
    // in the level player class directly as
    // they query the keyboard state as a whole.
    else if (evt.key.code == sf::Keyboard::G) {
        // God mode
    }
    else if (evt.key.code == sf::Keyboard::K) {
        // Kid cheat
    }
        /////////////
    else if (evt.key.code == sf::Keyboard::Escape) {
        p_player->Action_Interact(INP_EXIT);
    }
    else {
        return;
    }

    // Editor forward
}

void cLevelScene::Handle_Keyup_Event(sf::Event& evt)
{
    cLevel_Player* p_player = gp_current_level->Get_Player();
    cPreferences& preferences = gp_app->Get_Preferences();

    // (this is the old cLevel::Key_Up method)

    //// only if not in Editor
    //if (editor_level_enabled) {
    //    return 0;
    //}

    if (evt.key.code == preferences.m_key_right)
        p_player->Action_Stop_Interact(INP_RIGHT);
    else if (evt.key.code == preferences.m_key_left)
        p_player->Action_Stop_Interact(INP_LEFT);
    else if (evt.key.code == preferences.m_key_down)
        p_player->Action_Stop_Interact(INP_DOWN);
    else if (evt.key.code == preferences.m_key_jump)
        p_player->Action_Stop_Interact(INP_JUMP);
    else if (evt.key.code == preferences.m_key_shoot)
        p_player->Action_Stop_Interact(INP_SHOOT);
    else if (evt.key.code == preferences.m_key_action)
        p_player->Action_Stop_Interact(INP_ACTION);
    else // not processed
        return;

    // key got processed
}

void cLevelScene::Update(sf::RenderWindow& stage)
{
    cScene::Update(stage);

    gp_current_level->Update();
    stage.setView(gp_current_level->Get_View());

    if (gp_app->Is_Debug_Mode()) {
        cLevel_Player* p_player = Get_Current_Level()->Get_Player();
        sf::Vector2f pos = p_player->getPosition();
        std::stringstream posstr;

        posstr << "("
               << pos.x
               << "|"
               << pos.y
               << ")";

        pos.x += 48; // Position the text right to the player

        m_debug_playerpos_text.setString(posstr.str());
        m_debug_playerpos_text.setPosition(pos);
    }
}

void cLevelScene::Draw(sf::RenderWindow& stage)
{
    cScene::Draw(stage);
    // Draw the level elements themselves
    gp_current_level->Draw(stage);

    if (gp_app->Is_Debug_Mode()) {
        // Text is relative to the level player, so no
        // need to reset to the default SFML view.
        stage.draw(m_debug_playerpos_text);
    }

    // Draw the HUD
    // TODO
}

std::string cLevelScene::Name() const
{
    return "LevelScene";
}

/**
 * Add the given level to the level scene and make it the current
 * level. The level will subsequently be memory-managed by this
 * class, so don’t `delete` it after you added it to the scene.
 */
void cLevelScene::Add_Level(cLevel* p_level)
{
    debug_print("Adding level to current level scene: %s\n", p_level->Name().c_str());

    m_active_levels.push_back(p_level);
    gp_current_level = p_level;
}

/**
 * Add the level with the given name to the level scene and make it
 * the current level.
 *
 * \param[in] levelname
 * Name of the level to load. This is a level name, not a full
 * filename, i.e. something like "lvl_1" without directory
 * and extension. All the necessary pathes are searched automatically
 * to find the actual level file.
 */
void cLevelScene::Add_Level(const std::string& levelname)
{
    // Figure out the absolute path of this level.
    fs::path absolute_level_filename = gp_app->Get_PackageManager().Find_Level(levelname);

    if (absolute_level_filename.empty())
        throw(std::runtime_error("Level file not found!")); // TODO: Proper exception

    cLevel* p_level = cLevel::Load_From_File(absolute_level_filename);
    Add_Level(p_level);
}

/**
 * Set the current level to the loaded level with the given index.
 */
void cLevelScene::Set_Current_Level(size_t i)
{
    gp_current_level = m_active_levels[i];
}
