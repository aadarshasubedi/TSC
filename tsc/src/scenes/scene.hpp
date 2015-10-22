#ifndef TSC_SCENE_HPP
#define TSC_SCENE_HPP

namespace TSC {

    /**
     * Base class for all scenes in the game. A scene defines what
     * is currently happening, it has control of the entire scene
     * and the entire event handling stack (except for the global
     * event handlers defined in the cSpriteManager directly). In
     * each scene, something happens as defined by the Update()
     * and Draw() methods.
     *
     * This is an abstract class intended to be subclassed. By default,
     * it does nothing.
     */
    class cScene
    {
    public:
        cScene();
        virtual ~cScene();
        virtual void Handle_Event(sf::Event& evt);
        virtual void Update(sf::RenderWindow& stage);
        virtual void Draw(sf::RenderWindow& stage);

        bool Has_Finished() const;
        void Finish();

        virtual std::string Name() const;
    private:
        bool m_finished;
    };
}

#endif
