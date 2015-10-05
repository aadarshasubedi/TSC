#ifndef TSC_ACTOR_HPP
#define TSC_ACTOR_HPP

namespace TSC {

    /**
     * Base class for everything in a level. Not actors are non-level
     * elements such as the HUD.
     *
     * This is an abstract class that is intended to be subclassed.
     * Actors are invisible by default, thus only defined by their
     * collision rectangle. If you want a visible actor, you have
     * to use some drawable entity inside your class, e.g. an sf::Sprite.
     * See the implementation of cStaticActor for an example of this.
     *
     * Each actor has a collision rectangle that gets transformed when
     * you make use of SFML’s sf::Transformable interface. Use
     * Get_Transformed_Collision_Rect() to obtain the resulting
     * collision rectangle; to retrieve the unmodified rectangle,
     * use Get_Collision_Rect(). Note that setting the collision
     * rectangle with Set_Collision_Rect() always sets the collision
     * rectangle prior to any transformation, so you can’t e.g. manually
     * set the collision rectangle of a rotated sprite.
     *
     * Note this class inherits from [sf::Transformable](http://www.sfml-dev.org/documentation/2.2/classsf_1_1Transformable.php).
     */
    class cActor: public Scripting::cScriptable_Object, public sf::Transformable
    {
    public:
        /**
         * Type defining the main Z layer positions. The `m_pos_z` of an
         * actor is added to this. You should only access representatives
         * of this type via the `ZLAYER_*` constants. See cActor::Z() for
         * a thorough explanation of Z coordinate handling.
         */
        typedef float ZLayer;

        const ZLayer ZLAYER_PASSIVE      = 0.01f;     //< Minimum Z position for passive objects.
        const ZLayer ZLAYER_HALFMASSIVE  = 0.04f;     //< Minimum Z position for half-massive objects.
        const ZLayer ZLAYER_CLIMBABLE    = 0.05f;     //< Minimum Z position for climbable objects.
        const ZLayer ZLAYER_MASSIVE      = 0.08f;     //< Minimum Z position for massive objects.
        const ZLayer ZLAYER_PLAYER       = 0.0999f;   //< Z position of the level player.
        const ZLayer ZLAYER_FRONTPASSIVE = 0.10f;     //< Minimum Z position for front-passive objects.
        const ZLayer ZLAYER_POS_DELTA    = 0.000001f; //< Minimum Z step.

        cActor();
        cActor(XmlAttributes& attributes, cLevel& level, const std::string type_name = "sprite");
        virtual ~cActor();

        // copy this sprite
        virtual cActor* Copy(void) const;

        bool operator==(const cActor& other) const;
        bool operator!=(const cActor& other) const;

        void Do_Update();
        virtual void Draw(sf::RenderWindow& stage) const;
        virtual void Added_To_Level(cLevel* p_level, const unsigned long& uid);

        bool Handle_Collision(cCollision* p_collision);
        virtual bool Handle_Collision_Player(cCollision* p_collision);
        virtual bool Handle_Collision_Enemy(cCollision* p_collision);
        virtual bool Handle_Collision_Massive(cCollision* p_collision);
        virtual bool Handle_Collision_Passive(cCollision* p_collision);
        virtual bool Handle_Collision_Lava(cCollision* p_collision);

        void Set_Collision_Rect(sf::FloatRect rect);
        inline const sf::FloatRect& Get_Collision_Rect() const {return m_collision_rect;}

        sf::FloatRect Get_Transformed_Collision_Rect() const;
        bool Does_Collide(const sf::FloatRect& other_rect) const;
        bool Does_Collide(const sf::Vector2f& other_point) const;
        bool Does_Collide(const cActor& other_actor) const;

        virtual void Set_Collision_Type(enum CollisionType coltype);
        inline enum CollisionType Get_Collision_Type() const {return m_coltype;}
        inline void Set_Massive_Type(enum CollisionType coltype){ Set_Collision_Type(coltype); } // For backward compatbility
        bool Is_Blocking() const;
        bool Is_Collidable() const;

        inline void Set_Name(std::string name){m_name = name;}
        inline std::string Get_Name() const {return m_name;}

        inline const unsigned long& Get_UID() const {return m_uid;}

        void Accelerate_X(const float& deltax, bool real = false);
        void Accelerate_Y(const float& deltay, bool real = false);
        void Accelerate_XY(const float& deltax, const float& deltay, bool real = false);

        inline bool Can_Be_Ground() const { return m_can_be_ground; }
        virtual bool Set_On_Ground(cActor* p_ground_object, bool set_on_top = true);
        cActor* Reset_On_Ground();

        // Set this sprite on top of the given one
        void Set_On_Top(const cActor& ground_actor, bool optimize_hor_pos = true);
        void Set_On_Side(const cActor& other, ObjectDirection side);

        inline bool Is_Spawned(){ return m_spawned; } const
        inline void Set_Spawned(bool spawned){ m_spawned = spawned; }

        /* late initialization
         * this needs linked objects to be already loaded
        */
        virtual void Init_Links(void) {};

        float Z() const;
        //protected:
        virtual void Update();
        void Update_Gravity();
        void Update_Position();

        // automatically slows down until not moving anymore
        void Auto_Slow_Down(float x_speed, float y_speed = 0);

        inline void Add_Velocity_X(const float x, const bool real = 0) { Accelerate_X(x, real); } // Legacy code...
        inline void Add_Velocity_Y(const float y, const bool real = 0) { Accelerate_Y(y, real); } // ...compatibility
        /* Add Velocity
        * max : the maximum velocity
        * real : if set the speedfactor isn't used
        */
        inline void Add_Velocity_X_Max(const float x, const float max_x, const bool real = 0)
        {
            Add_Velocity_X(x, real);

            if (m_velocity.x > max_x) {
                m_velocity.x = max_x;
            }
        }
        inline void Add_Velocity_Y_Max(const float y, const float max_y, const bool real = 0)
        {
            Add_Velocity_Y(y, real);

            if (m_velocity.y > max_y) {
                m_velocity.y = max_y;
            }
        }
        /* Add Velocity
        * min : the minimum velocity
        * real : if set the speedfactor isn't used
        */
        inline void Add_Velocity_X_Min(const float x, const float min_x, const bool real = 0)
        {
            Add_Velocity_X(x, real);

            if (m_velocity.x < min_x) {
                m_velocity.x = min_x;
            }
        }
        inline void Add_Velocity_Y_Min(const float y, const float min_y, const bool real = 0)
        {
            Add_Velocity_Y(y, real);

            if (m_velocity.y < min_y) {
                m_velocity.y = min_y;
            }
        }
        /* Change position. This is mostly a forwarding
         * to the SFML-inherited setPosition() method,
         * except for new_startpos, which, if true, will
         * also set the `m_start_pos' attribute to the
         * same value. */
        inline void Set_Pos(float x, float y, bool new_startpos = false)
        {
            if (new_startpos) {
                m_start_pos.x = x;
                m_start_pos.y = y;
            }
            setPosition(x, y);
        }
        inline void Set_Pos_X(float x, bool new_startpos = false)
        {
            Set_Pos(x, getPosition().y, new_startpos);
        }
        inline void Set_Pos_Y(float y, bool new_startpos = false)
        {
            Set_Pos(getPosition().x, y, new_startpos);
        }

        virtual void Update_Valid_Update();

        cLevel* mp_level;
        unsigned long m_uid;

        sf::Vector2f m_start_pos;

        sf::FloatRect m_collision_rect;
        std::string m_name;

        sf::RectangleShape m_debug_colrect_shape;

        float m_gravity_max;      //< Maximum velocity that can be reached by gravity effect.
        float m_gravity_accel;    //< How quickly this object falls in gravity effect.
        cActor* mp_ground_object; //< Do we stand on something, and if so, on what?
        bool m_can_be_ground;     //< Can other objects stand on us?
        GroundType m_ground_type; //< In case we are ground, what type (ice, plastic, etc.)
        sf::Vector2f m_velocity;  //< Velocity in → and ↓ direction.

        enum CollisionType m_coltype;
        float m_pos_z;
        ZLayer m_z_layer;
        bool m_update_is_valid;
        /// Is this object generated later and not loaded from the level XML?
        bool m_spawned;

    private:
        void Init();
        void Check_On_Ground();

        /** When setting an object to the side of another object using
         * Set_On_Ground() and Set_On_Side() this distance is used to
         * prevent collisions from being sent after the adjustment, i.e.
         * it’s the distance between the collision rectangles after the
         * adjustment. */
        const float COLLISION_PREVENT_DISTANCE = 0.1f;
    };

}

#endif
