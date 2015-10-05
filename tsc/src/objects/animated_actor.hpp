#ifndef TSC_ANIMATED_ACTOR_HPP
#define TSC_ANIMATED_ACTOR_HPP
namespace TSC {

    /**
     * Actors that do not always show the same graphic, but rather
     * show different ones depending on the situation are instances
     * of this class or of one of its subclasses. The methods for
     * managing the multitude of textures associated with such
     * an instance comes from the cImageSet class — note this
     * class does multiple inheritance for this.
     *
     * Use Set_Image_Num() as inherited from cImageSet() to specify
     * which image of the sequence to show, as only that method allows
     * you to refer to what was defined in the image
     * sets.
     */
    class cAnimatedActor: public cSpriteActor, public cImageSet
    {
    public:
        cAnimatedActor();
        cAnimatedActor(XmlAttributes& attributes, cLevel& level, const std::string type_name = "animatedsprite");
        virtual ~cAnimatedActor();

        virtual void Draw(sf::RenderWindow& stage) const;
        virtual void Update();

        virtual void Set_Image_Set_Image(const struct ConfiguredTexture* p_new_image, bool new_startimage = false);
        virtual std::string Get_Identity(void);
    };

}
#endif
