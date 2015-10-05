/***************************************************************************
 * img_manager.cpp  -  Texture Handler/Manager
 *
 * Copyright © 2003 - 2011 Florian Richter
 * Copyright © 2013 - 2015 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/bintree.hpp"
#include "../core/errors.hpp"
#include "../core/property_helper.hpp"
#include "../core/xml_attributes.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../objects/actor.hpp"
#include "../scenes/scene.hpp"
#include "../core/scene_manager.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/filesystem/package_manager.hpp"
#include "../user/preferences.hpp"
#include "../core/tsc_app.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_settings.hpp"
#include "img_manager.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

cImage_Manager::cImage_Manager(const cPreferences* p_prefs, const cResource_Manager* p_resource_manager)
    : mp_preferences(p_prefs), mp_resource_manager(p_resource_manager)
{
    Determine_Cache_Dir();
}

cImage_Manager::~cImage_Manager()
{
    Clear();
}

/**
 * Shortcut for calling:
 *
 * ~~~~~~~~~~~{.cpp}
 * Get_Texture(utf8_to_path("your/path"));
 * ~~~~~~~~~~~
 */
const struct ConfiguredTexture& cImage_Manager::Get_Texture_UTF8(const std::string& relpath)
{
    return Get_Texture(utf8_to_path(relpath));
}

/**
 * Load a texture from the given file if it hasn’t been loaded
 * yet. If it has been loaded already, return it from the cache.
 *
 * \param[in] path
 * Path to load the texture from. This filename should be relative
 * to the pixmaps/ directory. It will first be tried to load the
 * image from the user’s directory (cache), then from the game’s
 * master directory. When a relative path is passed, the active
 * package will be searched first.
 *
 * \returns The corresponding SFML texture object.
 */
const ConfiguredTexture& cImage_Manager::Get_Texture(const fs::path& relpath)
{
    if (m_textures.count(relpath)) {
        return *(m_textures[relpath]);
    }
    else {
        fs::path masterpath;
        fs::path finalpath;

        if (relpath.is_absolute()) {
            std::cerr << "Warning: cImage_Manager::Get_Texture() received absolute path: " << path_to_utf8(relpath) << std::endl;
            masterpath = relpath;
            finalpath = relpath;
        }
        else { // Recommended usage
            Find_Image_Pathes(relpath, masterpath, finalpath);
        }

        cImage_Settings_Data* p_settings = Parse_Image_Settings(masterpath);
        sf::Texture* p_texture = new sf::Texture;

        /* Find_Image_Pathes() might return .PNG or .SETTINGS files.
         * The former can directly be loaded, the latter must have
         * a "base" key pointing to a .PNG file to be valid.
         * TODO: Support stacked .SETTINGS file, i.e. settings files
         * whose "base" key refers to another .SETTINGS file.
         * Or does the settings parser code already handle that case? */
        if (finalpath.extension() == utf8_to_path(".settings")) {
            if (!p_settings->m_base.empty()) {
                finalpath = finalpath.parent_path() / path_to_utf8(p_settings->m_base);
            }
            else {
                throw(std::runtime_error("Failed to load texture from file -- requested .settings file has no 'base' key!"));
            }
        }

        // Load sf::Image which can be edited easily
        sf::Image img;
        if (!img.loadFromFile(path_to_utf8(finalpath))) { // FIXME: Which encoding does SFML want here?
            // This can only happen if we don’t have read rights for
            // the file or a similar problem, or if the user passed an
            // absolute path into this method.
            throw(std::runtime_error("Failed to load texture from file -- do you have the file access rights set correctly?"));
        }

        // Apply X and Y mirrors to the image.
        // TODO: This should be in cImage_Settings_Data::Apply(),
        // but that function has no access to the image.
        if (p_settings->m_rotation_x)
            img.flipVertically();
        if (p_settings->m_rotation_y)
            img.flipHorizontally();

        // Upload to Graphics Card
        if (!p_texture->loadFromImage(img))
            throw(std::runtime_error("Failed to upload image to graphics card!"));

        struct ConfiguredTexture* p_conftexture = new ConfiguredTexture;
        p_conftexture->m_texture                = p_texture;
        p_conftexture->m_settings               = p_settings;
        p_conftexture->m_path                   = finalpath;

        m_textures[relpath] = p_conftexture;
        return *p_conftexture;
    }
}

/**
 * Digs out the master path for the given relative image path,
 * which is either a user or a game file. If it’s a game file,
 * also digs out the path to the cached variant of that file.
 * If no cached variant is found, both `masterpath` and `cachepath`
 * are set to the same path (the master file). User files never
 * get cached, hence `masterpath` and `cachepath` are equal for
 * those always.
 *
 * Either of the two returned pathes may be a `.settings` file, but
 * if the `masterpath` isn’t, then the `cachepath` is neither.
 */
void cImage_Manager::Find_Image_Pathes(const fs::path& relpath, fs::path& masterpath, fs::path& cachepath)
{
    // Ask the package manager for the master path. This even
    // works if no package is active, because it then searches
    // the main game and the user directories anyway.
    masterpath = gp_app->Get_PackageManager().Get_Pixmap_Reading_Path(path_to_utf8(relpath), true);

    // Ok, file found. Don’t try to load it directly, but use the scaled
    // version from the image cache if possible.
    if (!masterpath.empty()) {
        /* The package manager may find both game-supplied files or user-supplied files.
         * User-supplied files are never cached, so don’t attempt to look in the cache
         * for them. Game files MAY be cached, but don’t have to be. As the package
         * manager always returns absolute pathes, determining the relative path of
         * a user file relative to the game pixmaps directory is going to have a result
         * starting with several "../" elements at the beginning, whereas for real game
         * files this will not be the case. By checking for "..", we can thus find out
         * whether the package manager gave us a user or a game file. For the game file,
         * we then can check for the cached version. */
        // FIXME: This fails in the unlikely case the user installs TSC into the
        // user data directory, e.g. as ~/.local/share/tsc. But then you’ve got
        // other problems probably.
        // FIXME: relpath is given as an argument to this method already. The Package
        // Manager should be rewritten to allow user/game file checking separately.
        fs::path gamedir       = mp_resource_manager->Get_Game_Data_Directory();
        fs::path relativepath = fs::relative(gamedir, masterpath);

        // User file?
        if ((*relativepath.begin()) == fs::path("..")) {
            // Will never be cached, so don’t look in the cache.
            cachepath = masterpath;
        }
        else {
            // Game file. May have a cached version.
            cachepath = m_cache_dir / relativepath;

            // If we have a cached version: Use that. Otherwise use the unscaled master file.
            if (!fs::exists(cachepath)) {
                cachepath = masterpath;
            }
        }
    }
    else {
        // Maybe use a dummy texture instead of crashing?
        throw(std::runtime_error("Requested texture not found!")); // TODO: Proper exception
    }
}

/**
 * Takes an absolute path to a PNG file, and reads and parses the
 * corresponding SETTINGS file, which is assumed to reside in
 * the same directory with the same name with the extension `.settings`
 * instead of `.png`. Returns the parsed result, which must be `delete`d
 * by the caller.
 */
cImage_Settings_Data* cImage_Manager::Parse_Image_Settings(fs::path masterfile)
{
    cImage_Settings_Parser parser;
    return parser.Get(masterfile.replace_extension(utf8_to_path(".settings")));
}

/**
 * Using the current preferences, determine the location of the image
 * cache. Call this after changing resolutions, because the image cache
 * directory contains the current resolution in its directory name.
 *
 * This is automatically called when constructing the object as well, so
 * you don’t have to manually do that then.
 */
void cImage_Manager::Determine_Cache_Dir()
{
    std::stringstream ss;

    ss << mp_preferences->m_video_screen_w << "x" << mp_preferences->m_video_screen_h;
    m_cache_dir = mp_resource_manager->Get_User_Imgcache_Directory() / utf8_to_path(ss.str());
}

/**
 * Clear the texture cache. This deletes all textures currently
 * in the cache and *frees* their memory. Any sprite using these
 * textures will be invalid after this method has been called.
 */
void cImage_Manager::Clear()
{
    std::map<fs::path, struct ConfiguredTexture*>::iterator iter;
    for(iter=m_textures.begin(); iter != m_textures.end(); iter++) {
        delete iter->second->m_settings;
        delete iter->second->m_texture;
        delete iter->second;
    }

    m_textures.clear();
}

/**
 * Preload the SFML textures for some heavily used images.
 * The function accepts a callback that receives the number of files
 * already processed and the total number of files intended to
 * be processed.
 *
 * Requires the global `gp_app` pointer, so don’t call this before
 * it isn’t set up.
 *
 * TODO: Currently unused.
 */
void cImage_Manager::Preload_Textures(std::function<void (unsigned int files_done, unsigned int files_total)> cb)
{
    // image files
    vector<fs::path> image_files;

    // player
    vector<fs::path> player_small_images    = Get_Directory_Files(mp_resource_manager->Get_Game_Pixmaps_Directory() / utf8_to_path("alex/small"), ".png", false, false);
    vector<fs::path> player_big_images      = Get_Directory_Files(mp_resource_manager->Get_Game_Pixmaps_Directory() / utf8_to_path("alex/big"), ".png", false, false);
    vector<fs::path> player_fire_images     = Get_Directory_Files(mp_resource_manager->Get_Game_Pixmaps_Directory() / utf8_to_path("alex/fire"), ".png", false, false);
    vector<fs::path> player_ice_images      = Get_Directory_Files(mp_resource_manager->Get_Game_Pixmaps_Directory() / utf8_to_path("alex/ice"), ".png", false, false);
    vector<fs::path> player_ghost_images    = Get_Directory_Files(mp_resource_manager->Get_Game_Pixmaps_Directory() / utf8_to_path("alex/ghost"), ".png", false, false);

    image_files.insert(image_files.end(), player_small_images.begin(), player_small_images.end());
    image_files.insert(image_files.end(), player_big_images.begin(), player_big_images.end());
    image_files.insert(image_files.end(), player_fire_images.begin(), player_fire_images.end());
    image_files.insert(image_files.end(), player_ice_images.begin(), player_ice_images.end());
    image_files.insert(image_files.end(), player_ghost_images.begin(), player_ghost_images.end());

    // Mushrooms
    image_files.push_back(utf8_to_path("game/items/mushroom_red.png"));
    image_files.push_back(utf8_to_path("game/items/mushroom_green.png"));
    image_files.push_back(utf8_to_path("game/items/mushroom_blue.png"));
    image_files.push_back(utf8_to_path("game/items/mushroom_ghost.png"));
    // Fireberry
    image_files.push_back(utf8_to_path("game/items/fireberry_1.png"));
    image_files.push_back(utf8_to_path("game/items/fireberry_2.png"));
    image_files.push_back(utf8_to_path("game/items/fireberry_3.png"));
    // Star
    image_files.push_back(utf8_to_path("game/items/lemon_1.png"));
    // Feather
    //image_files.push_back( "game/items/feather_1.png" );
    // Yellow Goldpiece
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/1.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/2.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/3.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/4.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/5.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/6.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/7.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/8.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/9.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/10.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/1_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/2_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/3_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/4_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/5_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/6_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/7_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/8_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/9_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/yellow/10_falling.png"));
    // Red Goldpiece
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/1.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/2.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/3.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/4.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/5.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/6.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/7.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/8.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/9.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/10.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/1_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/2_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/3_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/4_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/5_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/6_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/7_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/8_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/9_falling.png"));
    image_files.push_back(utf8_to_path("game/items/goldpiece/red/10_falling.png"));

    // Brown Box
    image_files.push_back(utf8_to_path("game/box/brown1_1.png"));

    // Light animation
    image_files.push_back(utf8_to_path("animation/light_1/1.png"));
    image_files.push_back(utf8_to_path("animation/light_1/2.png"));
    image_files.push_back(utf8_to_path("animation/light_1/3.png"));
    // Particle animations
    image_files.push_back(utf8_to_path("animation/particles/fire_1.png"));
    image_files.push_back(utf8_to_path("animation/particles/fire_2.png"));
    image_files.push_back(utf8_to_path("animation/particles/fire_3.png"));
    image_files.push_back(utf8_to_path("animation/particles/fire_4.png"));
    image_files.push_back(utf8_to_path("animation/particles/smoke.png"));
    image_files.push_back(utf8_to_path("animation/particles/smoke_black.png"));
    image_files.push_back(utf8_to_path("animation/particles/light.png"));
    image_files.push_back(utf8_to_path("animation/particles/dirt.png"));
    image_files.push_back(utf8_to_path("animation/particles/ice_1.png"));
    image_files.push_back(utf8_to_path("animation/particles/cloud.png"));
    image_files.push_back(utf8_to_path("animation/particles/axis.png"));

    // Ball
    image_files.push_back(utf8_to_path("animation/fireball/1.png"));
    image_files.push_back(utf8_to_path("animation/iceball/1.png"));

    // HUD
    image_files.push_back(utf8_to_path("game/alex_l.png"));
    image_files.push_back(utf8_to_path("game/gold_m.png"));
    image_files.push_back(utf8_to_path("game/itembox.png"));

    unsigned int loaded_files = 0;
    unsigned int file_count = image_files.size();

    // load images
    for (vector<fs::path>::iterator itr = image_files.begin(); itr != image_files.end(); ++itr) {
        // get filename
        fs::path filename = (*itr);

        // preload (cache) image
        Get_Texture(filename);

        // count files
        loaded_files++;

        // Callback
        cb(loaded_files, file_count);
    }
}
