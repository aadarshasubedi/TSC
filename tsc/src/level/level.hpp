#ifndef TSC_LEVEL_HPP_HPP
#define TSC_LEVEL_HPP_HPP
namespace TSC {

    /**
     * This class represents a level that is being played.
     */
    class cLevel {
    public:
        static cLevel* Load_From_File(boost::filesystem::path levelfile);

#ifdef _DEBUG
        static cLevel* Construct_Debugging_Level();
#endif

        cLevel(); /// \private Private constructor
        cLevel(boost::filesystem::path levelfile);
        ~cLevel();

        bool operator==(const cLevel& other) const;
        bool operator!=(const cLevel& other) const;

        void Update();
        void Draw(sf::RenderWindow& stage) const;

        inline std::string Name() const {return path_to_utf8(m_level_filename.stem());}
        void Sort_Z_Elements();

        void Add_Player(cLevel_Player* p_player);
        void Add_Actor(cActor* p_actor, const unsigned long& uid = 0);
        void Check_Collisions_For_Actor(cActor& actor);

        inline cLevel_Player* Get_Player(){return mp_level_player;}
        inline const cLevel_Player* Get_Player() const {return mp_level_player;}

        inline const sf::View& Get_View() const {return m_camera_view;}

        /* Return the current Music filename relative to the music/
         * directory.
        */
        boost::filesystem::path Get_Music_Filename() const;
        // Set the Music filename. `filename' must be relative to
        // the music/ directory.
        void Set_Music(boost::filesystem::path filename);
        /* Set the filename
         * rename_old : if set also rename the level file in the user folder
        */
        void Set_Filename(boost::filesystem::path filename, bool rename_old = true);
        // Set the level author
        void Set_Author(const std::string& name);
        // Set the level version
        void Set_Version(const std::string& level_version);
        // Set the level description
        void Set_Description(const std::string& level_description);
        // Set the level difficulty ( 0 = undefined, 1 = dead easy and 100 = ultimate challenge )
        void Set_Difficulty(const uint8_t level_difficulty);
        // Set the level land type
        void Set_Land_Type(const LevelLandType level_land_type);

    // private: // TODO: This should be uncommented when we fix the OOP
        void Init();
        void Add_Collision_If_Required(cCollision* p_collision);
        /// Grab the next free UID, adjusting the level’s internal mantissa.
        inline unsigned long Get_Next_UID(){return ++m_last_max_uid;}
        /**
         * Reset the level’s internal UID mantissa. You need this when
         * you assigned an UID directly rather than used Get_Next_UID().
         * This method only allows to raise the mantissa, it cannot be
         * lowered to prevent UID clashes.
         */
        inline void Adjust_UID_Mantissa(const unsigned long& new_max){if (new_max > m_last_max_uid) m_last_max_uid = new_max;}

        static bool Is_Level_Object_Element(const CEGUI::String& element)
        {
            if (element == "information" || element == "settings" || element == "background" || element == "music" ||
                    element == "global_effect" || element == "player" || element == "sound" || element == "particle_emitter" ||
                    element == "path" || element == "sprite" || element == "powerup" || element == "item" || element == "enemy" ||
                    element == "levelexit" || element == "level_entry" || element == "enemystopper" || element == "box" ||
                    element == "moving_platform" || element == "falling_platform" || element == "ball" || element == "lava" || element == "crate") {
                return 1;
            }

            return 0;
        };

        // level filename
        boost::filesystem::path m_level_filename;
        // if a new level should be loaded this is the next level filename
        boost::filesystem::path  m_next_level_filename;
        int m_engine_version;

        std::unordered_map<unsigned long, TSC::Bintree<TSC::cCollision*> > m_collisions; //< Detected collisions from the current frame.
        unsigned long m_last_max_uid;

        std::vector<cActor*> m_actors;
        /// Level backgrounds.
        std::vector<sf::Sprite*> m_backgrounds;

        /// The one and only Alex.
        cLevel_Player* mp_level_player;

        // last save time
        time_t m_last_saved;
        // author
        std::string m_author;
        // version
        std::string m_version;
        // music filename
        boost::filesystem::path m_musicfile;
        // valid music to play
        bool m_valid_music;
        // description
        std::string m_description;
        // difficulty ( 0 = undefined, 1 = dead easy and 100 = ultimate challenge )
        uint8_t m_difficulty;

        // land type
        LevelLandType m_land_type;

        // player
        float m_player_start_pos_x;
        float m_player_start_pos_y;
        ObjectDirection m_player_start_direction;
        // camera
        sf::FloatRect m_camera_limits;
        float m_fixed_camera_hor_vel;
        // Unload after exiting (for a sublevel used from the same level more than once)
        bool m_unload_after_exit;

        // Do not re-Init() on sublevel loading.
        bool m_mruby_has_been_initialized;
        // MRuby script code associated with this level
        std::string m_script;

        sf::View m_camera_view;
    };

}
#endif
