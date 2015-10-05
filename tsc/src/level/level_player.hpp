#ifndef TSC_LEVEL_PLAYER_HPP
#define TSC_LEVEL_PLAYER_HPP

namespace TSC {

    /* *** *** *** *** *** *** *** *** Alex states *** *** *** *** *** *** *** *** *** */

    enum Alex_type {
        // normal
        ALEX_DEAD = 0,
        ALEX_SMALL = 1,
        ALEX_BIG = 2,
        ALEX_FIRE = 3,
        ALEX_ICE = 4,
        ALEX_CAPE = 5,
        ALEX_GHOST = 6
    };

    class cLevel_Player: public cAnimatedActor
    {
    public:
        cLevel_Player();
        virtual ~cLevel_Player();

        virtual bool Handle_Collision_Massive(cCollision* p_collision);

        // lets the Player halt
        void Hold(void);

        // moves in the current direction
        void Move_Player(float velocity, float vel_wrongway);
        void Set_Moving_State(Moving_state new_state);

        // Start a jump
        void Action_Jump(bool enemy_jump = 0);
        // General input interact event
        void Action_Interact(input_identifier key_type);
        // Shoot if available
        void Action_Shoot(void);

        // Stop jump
        void Action_Stop_Jump(void);
        // Stop Interacting event
        void Action_Stop_Interact(input_identifier key_type);
        // Stop Shoot action
        void Action_Stop_Shoot(void);

        // Stop ducking
        void Stop_Ducking(void);
        // Set up the m_up_key_time for better jumping detection
        void Start_Jump_Keytime(void);
        // Update the m_up_key_time and start a jump if possible
        void Update_Jump_Keytime(void);
        // Start a jump with the given de-acceleration
        void Start_Jump(float deaccel = 0.08f);
        // Update jump
        void Update_Jump(void);

        void Stop_Flying(bool parachute = 1);
        /* Set the parachute for flying
         * doesn't reload images
        */
        void Parachute(bool enable);



    protected:
        virtual void Update();

        void Load_Images(bool new_startimage = false);
        void Update_Walking(void);
        void Update_Staying(void);
        float Get_Vel_Modifier(void) const;
    private:
        ObjectDirection m_direction;

    public: // FIXME: Legacy code. Remove this for secrecy principle!
        Moving_state m_state;

        // current Alex type
        Alex_type m_alex_type;

        // true if player god mode is active
        bool m_god_mode;

        // time alex walked
        float m_walk_time;
        // running particle counter
        float m_running_particle_counter;
        // time alex is in ghost mode
        float m_ghost_time;
        // ghost mode drawing modifier
        float m_ghost_time_mod;
        // Amount of time left for invincibility (from damage, star, or file load). 0 means not invincible.
        float m_invincible;
        // Amount of time left for star (used for graphic flash effect)
        float m_invincible_star;
        // invincible drawing modifier
        float m_invincible_mod;
        // invincible star counter
        float m_invincible_star_counter;

        // walking animation counter
        float m_walk_count;
        // lives left
        int m_lives;

        // jumping key time
        float m_up_key_time;
        // force next jump
        bool m_force_jump;
        // next jump with sound
        bool m_next_jump_sound;
        // next jump power
        float m_next_jump_power;
        // next jump acceleration
        float m_next_jump_accel;
        // Acceleration if up key is pressed
        float m_jump_accel_up;
        // vely De-acceleration use 0.05 - 0.08
        float m_jump_vel_deaccel;
        // next jump power
        float m_jump_power;

        // throw animation counter
        float m_throwing_counter;
        // if alex is ducked
        uint32_t m_ducked_counter;
        // ducked animation counter
        float m_ducked_animation_counter;
        // if parachute falling
        bool m_parachute;
        // amount of goldpieces collected
        unsigned int m_goldpieces;
        // collected points
        long m_points;
        // enemy killed in a row multiplier
        float m_kill_multiplier;
        // time since the last killed enemy
        float m_last_kill_counter;
        // time player didn't move
        float m_no_velx_counter;
        float m_no_vely_counter;

        // fire/ice-ball shoot counter
        float m_shoot_counter;

        // Are we going through a level exit/level entry right now?
        bool m_is_warping;

        // active/holding object ( f.e. shell )
        cAnimatedActor* mp_active_object;
        // direction when ducking started ( for the anti stuck under a block test )
        ObjectDirection m_duck_direction;

        // default position
        static const float m_default_pos_x;
        static const float m_default_pos_y;
    };
}

#endif
