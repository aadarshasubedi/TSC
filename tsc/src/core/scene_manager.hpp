#ifndef TSC_SCENE_MANAGER_HPP
#define TSC_SCENE_MANAGER_HPP

namespace TSC {

    /**
     * Main class for managing the happenings in the game. Each state
     * the game may have is represented by a scene that encapsulates
     * all the information required for it.
     *
     * The scenes are managed by a stack, where the scene on top of
     * the stack is the active (current) scene. By placing a new scene
     * on top of the stack, the game switches to it; by removing the
     * top scene from the stack, the game returns to the previous
     * scene.
     *
     * You can retrieve two interesting values from the global instance
     * of this class via Get_FPS() and Get_Speedfactor(). The former
     * returns how many frames have elapsed during the last mainloop
     * iteration. The latter is constructed by the time the last
     * iteration of the mainloop needed divided by the FPS, which
     * results in a variable factor that:
     *
     * 1. Grows if the FPS are lower, i.e. it is larger if the game
     *    has a slower overall experience, and
     * 2. Shrinks if the FPS are higher, i.e. it is smaller if the
     *    game has a faster overall experience.
     *
     * The reasoning behind that factor is that objects moving on
     * the screen do not move super-fast with a high FPS and do not move
     * super-slow with a low FPS. To move over the same amount of pixels
     * in the same time you need to do smaller movement steps with a
     * higher FPS and larger movement steps with a smaller FPS. To ease
     * this calculation, you can just multiply your desired distance
     * with the speedfactor which you can retrieve from this class,
     * and the object will behave roughly the same relative to the
     * user’s screen and time regardless of the FPS.
     *
     * The calculation breaks if a frame needs longer than a second
     * to be processed. But then you can’t play the game anyway.
     */
    class cSceneManager
    {
    public:
        cSceneManager();

        void Push_Scene(cScene* p_scene);
        cScene* Pop_Scene();

        void Play(sf::RenderWindow& stage);
        inline cScene& Current_Scene(){return *m_scenes_stack.top();}
        inline void End_Play(){m_end_play = true;}

        inline float Get_Speedfactor() const {return m_speedfactor;}
        inline unsigned int Get_FPS() const {return m_frames_counted;}

        // Returns the time that was needed to handle the entire last
        // frame.
        inline float Get_Elapsed_Time() const {return m_elapsed_time;}
    private:
        bool Handle_Global_Event(sf::Event& evt);
        unsigned int SFMLKey2CEGUIKey(const sf::Keyboard::Key& key);

        std::stack<cScene*> m_scenes_stack;
        sf::Clock m_game_clock;
        float m_speedfactor;
        float m_elapsed_time;
        float m_last_ticks;
        unsigned int m_frames_counted;
        bool m_end_play;

        sf::Text m_framerate_text;
        sf::Font m_framerate_textfont;
    };

}

#endif
