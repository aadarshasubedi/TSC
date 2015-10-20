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
        virtual void Draw(sf::RenderWindow& stage);
        virtual std::string Name() const;
    private:
        sf::Sprite m_gameover_sprite;
        sf::Texture m_gameover_texture;
    };

}
#endif
