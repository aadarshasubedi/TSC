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
#include "game_over_scene.hpp"

using namespace TSC;

cGameOverScene::cGameOverScene()
    : mp_gameover_clock(NULL)
{
    m_gameover_texture.loadFromFile(gp_app->Get_ResourceManager().Get_Game_Pixmap("game/game_over.png").native());

    m_gameover_sprite.setTexture(m_gameover_texture);
    m_gameover_sprite.setOrigin(m_gameover_sprite.getLocalBounds().width / 2.0,
                                m_gameover_sprite.getLocalBounds().height / 2.0);
}

cGameOverScene::~cGameOverScene()
{
    if (mp_gameover_clock)
        delete mp_gameover_clock;
}

std::string cGameOverScene::Name() const
{
    return "GameOverScene";
}

void cGameOverScene::Update(sf::RenderWindow& stage)
{
    sf::Vector2u size = stage.getSize();
    m_gameover_sprite.setPosition(size.x / 2.0, size.y / 2.0);

    if (mp_gameover_clock) {
        // Display it for 10 seconds (matches with the game over melody).
        if (mp_gameover_clock->getElapsedTime().asSeconds() > 10.0) {
            Finish();
        }
        // else do nothing, just show gameover screen
    }
    else {
        // Start the timer on first scene use (not just scene on stack)
        mp_gameover_clock = new sf::Clock();
    }
}

void cGameOverScene::Draw(sf::RenderWindow& stage)
{
    cScene::Draw(stage);

    stage.draw(m_gameover_sprite);
}

void cGameOverScene::Handle_Event(sf::Event& evt)
{
    cScene::Handle_Event(evt);

    switch (evt.type) {
        case sf::Event::KeyReleased:
            on_handle_key_released(evt);
            break;
    default:
        break;
    }
}

void cGameOverScene::on_handle_key_released(sf::Event& evt)
{
    if (evt.key.code == sf::Keyboard::Escape) {
        Finish();
    }
}
