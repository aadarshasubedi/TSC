#ifndef TSC_STATIC_ACTOR_HPP
#define TSC_STATIC_ACTOR_HPP
namespace TSC {

    /**
     * Nonmoving actors whose sprite always shows an unanimated single
     * image. This is most notably the case for ground tiles.
     */
    class cStaticActor: public cSpriteActor
    {
    public:
        cStaticActor();
        cStaticActor(XmlAttributes& attributes, cLevel& level, const std::string type_name = "sprite");
        cStaticActor(boost::filesystem::path relative_texture_path);
        virtual ~cStaticActor();

        virtual void Draw(sf::RenderWindow& stage) const;

        void Set_Texture(boost::filesystem::path relative_texture_path);
        inline const ConfiguredTexture* Get_Texture() const {return mp_texture;} // Use Set_Texture() to change things!
    protected:
        boost::filesystem::path m_rel_texture_path;
        const struct ConfiguredTexture* mp_texture;
    private:
        void Init();
    };

}
#endif
