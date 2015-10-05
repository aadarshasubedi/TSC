#ifndef TSC_SPRITE_ACTOR_HPP
#define TSC_SPRITE_ACTOR_HPP

namespace TSC {

    /**
     * Abstract class for actors wrapping an SFML sprite.
     * You should not use this class bare. Use the cStaticActor
     * and cAnimatedActor subclasses instead.
     */
    class cSpriteActor: public cActor
    {
    public:
        cSpriteActor();
        cSpriteActor(XmlAttributes& attributes, cLevel& level, const std::string type_name = "sprite");
        virtual ~cSpriteActor();

        virtual void Draw(sf::RenderWindow& stage) const;
        virtual void Added_To_Level(cLevel* p_level, const unsigned long& uid);

        void Set_Dimensions(int width, int height);

        /* Make this actor temporaryly invisible. For permanently
         * invisible actors (that do not have an assocated sf::Sprite,
         * for example enemy stoppers), inherit directly from cActor
         * rather than from this class and do not define an sf::Sprite
         * member. Invisibility is off by default. */
        inline bool Is_Invisible(){ return m_invisible; } const
        inline void Set_Invisible(bool invis){ m_invisible = invis; }

    protected:
        sf::Sprite m_sprite;
        bool m_invisible;

    private:
        void Init();
    };

}

#endif
