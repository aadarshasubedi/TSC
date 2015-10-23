#ifndef TSC_SCENES_MENU_SCENE_HPP
#define TSC_SCENES_MENU_SCENE_HPP
namespace TSC {

    /**
     * The Scene for the game’s main menu.
     */
    class cMenuScene: public cScene
    {
    public:
        cMenuScene();
        virtual ~cMenuScene();

        virtual void Handle_Event(sf::Event& evt);
        virtual void Update(sf::RenderWindow& stage);
        virtual void Draw(sf::RenderTarget& stage);
        virtual std::string Name() const;
    protected:
        const int MENUITEM_COUNT = 5;
        const float MAX_SCALE = 0.25f;
        const float MIN_SCALE = 0.1953f;
        const char* TITLE_MUSIC = "game/menu.ogg";

        void on_handle_key_released(sf::Event& evt);
        void on_handle_return_released(sf::Event& evt);

        std::vector<sf::Sprite> m_menu_items;
        std::vector<sf::Texture> m_menu_textures;
        sf::Music m_titlemusic;
        int m_selected_item_id;
        float m_scale_factor;
        bool m_is_growing;
    };

}
#endif
