/***************************************************************************
 * img_manager.h
 *
 * Copyright © 2003 - 2011 Florian Richter
 * Copyright © 2013 - 2014 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TSC_IMG_MANAGER_HPP
#define TSC_IMG_MANAGER_HPP

namespace TSC {

    /**
     * Struct that contains the information of the texture’s `.settings` file
     * next to the texture itself.
     */
    struct ConfiguredTexture
    {
        sf::Texture* m_texture;
        cImage_Settings_Data* m_settings;
        boost::filesystem::path m_path;
    };

    /* *** *** *** *** *** *** cImage_Manager *** *** *** *** *** *** *** *** *** *** *** */

    /**
     * This class manages the SFML textures. As loading a texture is
     * a performance-intense operation, the number of textures should
     * be kept as low as possible. Thus, you can just ask this class
     * for a texture, and if it is loaded already, it will be returned,
     * otherwise it will be loaded and then cached for later calls.
     */
    class cImage_Manager
    {
    public:
        cImage_Manager(const cPreferences* p_prefs, const cResource_Manager* p_resource_manager);
        virtual ~cImage_Manager(void);

        void Preload_Textures(std::function<void (unsigned int files_done, unsigned int files_total)> cb);
        const struct ConfiguredTexture& Get_Texture(const boost::filesystem::path& relpath);
        const struct ConfiguredTexture& Get_Texture_UTF8(const std::string& relpath);

        void Clear();
        void Determine_Cache_Dir();
    private:
        void Find_Image_Pathes(const boost::filesystem::path& relpath, boost::filesystem::path& masterpath, boost::filesystem::path& cachepath);
        cImage_Settings_Data* Parse_Image_Settings(boost::filesystem::path masterfile);

        std::map<boost::filesystem::path, struct ConfiguredTexture*> m_textures;
        boost::filesystem::path m_cache_dir;

        const cPreferences* mp_preferences;
        const cResource_Manager* mp_resource_manager;
    };

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif
