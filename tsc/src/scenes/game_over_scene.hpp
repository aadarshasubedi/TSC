#ifndef TSC_SCENES_GAMEOVER_SCENE_HPP
#define TSC_SCENES_GAMEOVER_SCENE_HPP
namespace TSC {

    /**
     * This scene is the last one played.
     */
    class cGameOverScene: public cScene
    {
    public:
        cGameOverScene();
        virtual ~cGameOverScene();

        virtual void Handle_Event(sf::Event& evt);
        virtual void Update(sf::RenderWindow& stage);
        virtual void Draw(sf::RenderTarget& stage);
        virtual std::string Name() const;
    private:
        sf::Sprite m_gameover_sprite;
        sf::Texture m_gameover_texture;
        sf::Clock* mp_gameover_clock;

        void on_handle_key_released(sf::Event& evt);
    };

}
#endif
