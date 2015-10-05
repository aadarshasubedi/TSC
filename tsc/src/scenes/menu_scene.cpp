#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/bintree.hpp"
#include "../core/errors.hpp"
#include "../core/property_helper.hpp"
#include "../core/xml_attributes.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../objects/actor.hpp"
#include "../scenes/scene.hpp"
#include "../core/scene_manager.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/filesystem/package_manager.hpp"
#include "../user/preferences.hpp"
#include "../core/tsc_app.hpp"
#include "../objects/actor.hpp"
#include "../level/level.hpp"
#include "scene.hpp"
#include "level_scene.hpp"
#include "menu_scene.hpp"

using namespace TSC;

cMenuScene::cMenuScene()
{
    m_menu_textures.resize(MENUITEM_COUNT);
    m_menu_items.resize(MENUITEM_COUNT);

    m_menu_textures[0].loadFromFile(gp_app->Get_ResourceManager().Get_Game_Pixmap("menu/start.png").native());
    m_menu_textures[1].loadFromFile(gp_app->Get_ResourceManager().Get_Game_Pixmap("menu/options.png").native());
    m_menu_textures[2].loadFromFile(gp_app->Get_ResourceManager().Get_Game_Pixmap("menu/load.png").native());
    m_menu_textures[3].loadFromFile(gp_app->Get_ResourceManager().Get_Game_Pixmap("menu/save.png").native());
    m_menu_textures[4].loadFromFile(gp_app->Get_ResourceManager().Get_Game_Pixmap("menu/quit.png").native());

    int x = gp_app->Get_Preferences().m_video_screen_w / 2;
    for(int i=0; i < 5; i++) {
        m_menu_textures[i].setSmooth(true);

        m_menu_items[i].setTexture(m_menu_textures[i]);
        m_menu_items[i].setOrigin(m_menu_items[i].getLocalBounds().width / 2.0f, m_menu_items[i].getLocalBounds().height / 2.0f);
        m_menu_items[i].setPosition(sf::Vector2f(x, 100 * (i + 1)));
        m_menu_items[i].setScale(MIN_SCALE, MIN_SCALE);
    }

    m_titlemusic.openFromFile(gp_app->Get_ResourceManager().Get_Game_Music(TITLE_MUSIC).native());
    m_titlemusic.setLoop(true);
    m_titlemusic.play();

    m_selected_item_id = 0;
    m_scale_factor = MIN_SCALE;
    m_is_growing = true;
}

cMenuScene::~cMenuScene()
{
    m_titlemusic.stop();
}

void cMenuScene::Handle_Event(sf::Event& evt)
{
    cScene::Handle_Event(evt);

    switch(evt.type) {
    case sf::Event::KeyReleased:
        on_handle_key_released(evt);
        break;
    default:
        break;
    }
}

void cMenuScene::Update(sf::RenderWindow& stage)
{
    cScene::Update(stage);

    if (m_is_growing) {
        m_scale_factor += 0.1f * gp_app->Get_SceneManager().Get_Speedfactor();

        if (m_scale_factor >= MAX_SCALE) {
            m_is_growing = false;
        }
    }
    else {
        m_scale_factor -= 0.1f * gp_app->Get_SceneManager().Get_Speedfactor();

        if (m_scale_factor <= MIN_SCALE) {
            m_is_growing = true;
        }
    }

    m_menu_items[m_selected_item_id].setScale(m_scale_factor, m_scale_factor);
}

void cMenuScene::Draw(sf::RenderWindow& stage)
{
    cScene::Draw(stage);

    std::vector<sf::Sprite>::const_iterator iter;
    for(iter=m_menu_items.begin(); iter != m_menu_items.end(); iter++)
        stage.draw(*iter);
}

std::string cMenuScene::Name() const
{
    return "MenuScene";
}

void cMenuScene::on_handle_key_released(sf::Event& evt)
{
    switch (evt.key.code) {
    case sf::Keyboard::Down:
        m_selected_item_id++;

        if (m_selected_item_id >= MENUITEM_COUNT)
            m_selected_item_id = MENUITEM_COUNT - 1;

        break;
    case sf::Keyboard::Up:
        m_selected_item_id--;

        if (m_selected_item_id < 0)
            m_selected_item_id = 0;
        break;
    case sf::Keyboard::Return:
        on_handle_return_released(evt);
    default:
        break;
    }

    std::vector<sf::Sprite>::iterator iter;
    for(iter=m_menu_items.begin(); iter != m_menu_items.end(); iter++)
        iter->setScale(MIN_SCALE, MIN_SCALE);
}

void cMenuScene::on_handle_return_released(sf::Event& evt)
{
    switch (m_selected_item_id) {
    case 0:
        gp_app->Get_SceneManager().Push_Scene(new cLevelScene("testlevel")); // DEBUG
        break;
    case 1:
        break;
    case 2:
        gp_app->Get_SceneManager().Push_Scene(new cLevelScene("lvl_2")); // DEBUG
        break;
    case 3:
        break;
    case 4:
        gp_app->Get_SceneManager().End_Play();
        break;
    default:
        std::cerr << "Warning: Invalid menu index " << m_selected_item_id << "." << std::endl;
        break;
    }
}
