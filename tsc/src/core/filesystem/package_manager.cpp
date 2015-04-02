/***************************************************************************
 * package_manager.cpp  -  Package Manager
 *
 * Copyright © 2009 - 2011 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../global_basic.hpp"
#include "../property_helper.hpp"
#include "../errors.hpp"
#include "../global_basic.hpp"
#include "../global_game.hpp"
#include "../errors.hpp"
#include "../property_helper.hpp"
#include "../xml_attributes.hpp"
#include "../../user/preferences.hpp"
#include "../../scripting/scriptable_object.hpp"
#include "../../objects/actor.hpp"
#include "../../scenes/scene.hpp"
#include "../scene_manager.hpp"
#include "../../video/img_manager.hpp"
#include "resource_manager.hpp"
#include "package_manager.hpp"
#include "../tsc_app.hpp"
#include "filesystem.hpp"
#include "package_manager.hpp"

namespace fs = boost::filesystem;
namespace errc = boost::system::errc;

namespace TSC {
/* *** *** *** *** *** *** PackageInfo *** *** *** *** *** *** *** *** *** *** *** */
PackageInfo :: PackageInfo()
    : found_user(false), found_game(false), hidden(false)
{
}

/* *** *** *** *** *** *** cPackage_Loader *** *** *** *** *** *** *** *** *** *** *** */

cPackage_Loader :: cPackage_Loader()
    : xmlpp::SaxParser()
{
}

cPackage_Loader :: ~cPackage_Loader()
{
}

PackageInfo cPackage_Loader :: Get_Package_Info(void)
{
    return m_package;
}

void cPackage_Loader :: parse_file(fs::path filename)
{
    xmlpp::SaxParser::parse_file(path_to_utf8(filename));
}

void cPackage_Loader :: on_start_document()
{
    // reset to defaults
    m_package = PackageInfo();
}

void cPackage_Loader :: on_end_document()
{
}

void cPackage_Loader :: on_start_element(const Glib::ustring& name, const xmlpp::SaxParser::AttributeList& properties)
{
    if (name == "property" || name == "Property") {
        std::string key;
        std::string value;

        xmlpp::SaxParser::AttributeList::const_iterator iter;
        for (iter = properties.begin(); iter != properties.end(); ++iter) {
            xmlpp::SaxParser::Attribute attr = *iter;

            if (attr.name == "name" || attr.name == "Name")
                key = attr.value;
            else if (attr.name == "value" || attr.name == "Value")
                value = attr.value;
        }

        m_current_properties[key] = value;
    }
}

void cPackage_Loader :: on_end_element(const Glib::ustring& name)
{
    if (name == "property" || name == "Property")
        return;

    if (name == "use" || name == "Use") {
        std::string package = m_current_properties["package"];
        if (!package.empty())
            m_package.dependencies.push_back(package);
    }
    else if (name == "settings" || name == "Settings") {
        m_package.name = m_current_properties["name"];
        m_package.hidden = static_cast<bool>(string_to_int(m_current_properties["hidden"]));
        m_package.desc = m_current_properties["description"];
        m_package.menu_level = m_current_properties["menu_level"];
    }

    m_current_properties.clear();
}

/* *** *** *** *** *** *** cPackage_Manager *** *** *** *** *** *** *** *** *** *** *** */

cPackage_Manager :: cPackage_Manager(const cResource_Manager& resource_manager)
{
    cout << "Initializing Package Manager" << endl;

    // Scan user data dir first so any user "packages.xml" will override the same in the game data dire
    Scan_Packages(resource_manager.Get_User_Data_Directory() / utf8_to_path("packages"), fs::path(), true);
    Scan_Packages(resource_manager.Get_Game_Data_Directory() / utf8_to_path("packages"), fs::path(), false);
    Fix_Package_Paths();
}

cPackage_Manager :: ~cPackage_Manager(void)
{
}

static bool operator< (const PackageInfo& p1, const PackageInfo& p2)
{
    return p1.name < p2.name;
}

std::vector<PackageInfo> cPackage_Manager :: Get_Packages(void)
{
    std::vector<PackageInfo> packages;
    std::map<std::string, PackageInfo>::const_iterator it;
    for (it = m_packages.begin(); it != m_packages.end(); it++) {
        packages.push_back(it->second);
    }
    std::sort(packages.begin(), packages.end());

    return packages;
}

PackageInfo cPackage_Manager :: Get_Package(const std::string& name)
{
    if (m_packages.find(name) != m_packages.end()) {
        return m_packages[name];
    }

    return PackageInfo();
}

void cPackage_Manager :: Set_Current_Package(const std::string& name)
{
    if (m_packages.find(name) == m_packages.end())
        m_current_package = std::string();
    else
        m_current_package = name;

    Build_Search_Path();
    Init_User_Paths();
}

std::string cPackage_Manager :: Get_Current_Package(void)
{
    return m_current_package;
}

void cPackage_Manager :: Init_User_Paths(void)
{
    // Levels
    if (!Dir_Exists(Get_User_Level_Path()))
        fs::create_directories(Get_User_Level_Path());

    // Campaign
    if (!Dir_Exists(Get_User_Campaign_Path()))
        fs::create_directories(Get_User_Campaign_Path());

    // World
    if (!Dir_Exists(Get_User_World_Path()))
        fs::create_directories(Get_User_World_Path());

    // Savegame
    if (!Dir_Exists(Get_User_Savegame_Path()))
        fs::create_directories(Get_User_Savegame_Path());

    // Screenshot
    if (!Dir_Exists(Get_User_Screenshot_Path()))
        fs::create_directories(Get_User_Screenshot_Path());
}

fs::path cPackage_Manager :: Get_User_Data_Path(void)
{
    return m_search_path[m_package_start];
}

fs::path cPackage_Manager :: Get_Game_Data_Path(void)
{
    return m_search_path[m_package_start + 1];
}

fs::path cPackage_Manager :: Get_User_Level_Path(void)
{
    return Get_User_Data_Path() / utf8_to_path("levels");
}

fs::path cPackage_Manager :: Get_Game_Level_Path(void)
{
    return Get_Game_Data_Path() / utf8_to_path("levels");
}

/**
 * Searches for a level with the given name in the following directories:
 *
 * 1. User level directory in package
 * 2. Game level directory in package
 * 3. General User level directory
 * 4. General Game level directory
 *
 * If a level of the given filename is found, returns an absolute boost
 * filesystem path to it. If such a level is not found, returns an
 * empty boost::filesystem::path.
 *
 * \param[in] name
 * Level file stem, e.g. "lvl_1". Note this does *not* include any
 * directory path nor a file extension.
 *
 * \returns boost::filesystem::path pointing to the level file or empty
 * path.
 *
 * TODO: Compatibility code for file extensions other than .tsclvl.
 */
fs::path cPackage_Manager :: Find_Level(const std::string& name)
{
    // 1. User level directory in package (err, what’s that?)
    fs::path result = Get_User_Level_Path() / utf8_to_path(name + ".tsclvl");
    if (fs::exists(result))
        return result;

    // 2. Game level directory in package
    result = Get_Game_Level_Path() / utf8_to_path(name + ".tsclvl");
    if (fs::exists(result))
        return result;

    // 3. General User level directory
    // Should not be required as Build_Search_Path() has it anyway?
    result = gp_app->Get_ResourceManager().Get_User_Level_Directory() / utf8_to_path(name + ".tsclvl");
    if (fs::exists(result))
        return result;

    // 4. General Game level directory
    // Should not be required as Build_Search_Path() has it anyway?
    result = gp_app->Get_ResourceManager().Get_Game_Level_Directory() / utf8_to_path(name + ".tsclvl");
    if (fs::exists(result))
        return result;

    // Not found
    return fs::path();
}

fs::path cPackage_Manager :: Get_Menu_Level_Path(void)
{
    // determine level for the menu
    fs::path result;
    std::string level;

    // User specified menu level
    level = gp_app->Get_Preferences().m_menu_level;
    if (!level.empty()) {
        level = level + ".tsclvl";

        result = Get_User_Level_Path() / level;
        if (fs::exists(result))
            return result;

        result = Get_Game_Level_Path() / level;
        if (fs::exists(result))
            return result;

        result = gp_app->Get_ResourceManager().Get_User_Level_Directory() / level;
        if (fs::exists(result))
            return result;

        result = gp_app->Get_ResourceManager().Get_Game_Level_Directory() / level;
        if (fs::exists(result))
            return result;
    }

    // Package menu
    if (!m_current_package.empty()) {
        level = m_packages[m_current_package].menu_level;
        if (!level.empty()) {
            level = level + ".tsclvl";

            result = Get_User_Level_Path() / level;
            if (fs::exists(result))
                return result;

            result = Get_Game_Level_Path() / level;
            if (fs::exists(result))
                return result;
        }
    }

    // Default menu level
    level = gp_app->Get_Preferences().m_menu_level_default + ".tsclvl";

    result = gp_app->Get_ResourceManager().Get_User_Level_Directory() / level;
    if (fs::exists(result))
        return result;

    return gp_app->Get_ResourceManager().Get_Game_Level_Directory() / level;
}

fs::path cPackage_Manager :: Get_User_Campaign_Path(void)
{
    return Get_User_Data_Path() / utf8_to_path("campaigns");
}

fs::path cPackage_Manager :: Get_Game_Campaign_Path(void)
{
    return Get_Game_Data_Path() / utf8_to_path("campaigns");
}

fs::path cPackage_Manager :: Get_User_World_Path(void)
{
    return Get_User_Data_Path() / utf8_to_path("worlds");
}

fs::path cPackage_Manager :: Get_Game_World_Path(void)
{
    return Get_Game_Data_Path() / utf8_to_path("worlds");
}

fs::path cPackage_Manager :: Get_Scripting_Path(const std::string& package, const std::string& script)
{
    if (package.empty()) {
        // For core scripts, only check game directory
        return gp_app->Get_ResourceManager().Get_Game_Scripting(script);
    }
    else if (m_packages.find(package) != m_packages.end()) {
        fs::path result;
        PackageInfo info = m_packages[package];

        // Check user data directory, then game data directory
        result = info.user_data_dir / "scripting" / utf8_to_path(script);
        if (!fs::exists(result))
            result = info.game_data_dir / "scripting" / utf8_to_path(script);

        return result;
    }

    return fs::path();
}

fs::path cPackage_Manager :: Get_User_Savegame_Path(void)
{
    fs::path result = gp_app->Get_ResourceManager().Get_User_Savegame_Directory();
    if (m_current_package.empty())
        return result;

    result = result / utf8_to_path(m_current_package);
    result.replace_extension("");
    return result;
}

fs::path cPackage_Manager :: Get_User_Screenshot_Path(void)
{
    fs::path result = gp_app->Get_ResourceManager().Get_User_Screenshot_Directory();
    if (m_current_package.empty())
        return result;

    result = result / utf8_to_path(m_current_package);
    result.replace_extension("");
    return result;
}

fs::path cPackage_Manager :: Get_Pixmap_Reading_Path(const std::string& pixmap, bool use_settings /* = false */)
{
    std::vector<std::string> ext;
    if (use_settings)
        ext.push_back(".settings");

    return Find_Reading_Path("pixmaps", utf8_to_path(pixmap), ext);
}

fs::path cPackage_Manager :: Get_Sound_Reading_Path(const std::string& sound)
{
    std::vector<std::string> ext;
    return Find_Reading_Path("sounds", utf8_to_path(sound), ext);
}

fs::path cPackage_Manager :: Get_Music_Reading_Path(const std::string& music)
{
    std::vector<std::string> ext;
    return Find_Reading_Path("music", utf8_to_path(music), ext);
}

fs::path cPackage_Manager :: Get_Relative_Pixmap_Path(fs::path path)
{
    return Find_Relative_Path("pixmaps", path);
}

fs::path cPackage_Manager :: Get_Relative_Sound_Path(fs::path path)
{
    return Find_Relative_Path("sounds", path);
}

fs::path cPackage_Manager :: Get_Relative_Music_Path(fs::path path)
{
    return Find_Relative_Path("music", path);
}

void cPackage_Manager :: Scan_Packages( fs::path base, fs::path path, bool user_packages )
{
    fs::path subdir(base / path);
    fs::directory_iterator end_iter;

    if(fs::exists(subdir) && fs::is_directory(subdir)) {
        for(fs::directory_iterator dir_iter(subdir) ; dir_iter != end_iter ; ++dir_iter) {
            fs::path entry = dir_iter->path();
            if(entry.extension() == fs::path(".tscpkg")) {
                // Determine package name and load info
                Load_Package_Info(entry, user_packages);
            }
            else {
                Scan_Packages( base, path / entry.filename(), user_packages );
            }
        }
    }
}

void cPackage_Manager :: Load_Package_Info( const fs::path& dir, bool user_package )
{
    // Read package information
    fs::path file = dir / "package.xml";
    if(!File_Exists(file)) {
        cout << "Warning: packages without 'package.xml' will be ignored: " << dir << endl;
        return;
    }

    cPackage_Loader loader;
    loader.parse_file(file);

    // Examine name and create package if it doesn't exist
    PackageInfo info = loader.Get_Package_Info();
    if(info.name.empty()) {
        cout << "Warning: packages without a name will be ignored: " << dir << endl;
        return;
    }

    if(m_packages.find(info.name) == m_packages.end()) {
        m_packages[info.name] = PackageInfo();
        m_packages[info.name].name = info.name;
    }

    PackageInfo& the_package = m_packages[info.name];

    // Avoid duplicates
    if(user_package && the_package.found_user) {
        cout << "Warning: duplicate user package will not be added: " << dir << endl;
        cout << "         original found at: " << the_package.user_data_dir;
        return;
    }

    if(!user_package && the_package.found_game) {
        cout << "Warning: duplicate game package will not be added: " << dir << endl;
        cout << "         original found at: " << the_package.game_data_dir;
        return;
    }

    // Print found message
    cout << "Found " << (user_package ? "user" : "game") << " package " << info.name << ":" << dir << endl;

    // Set internal information
    if(user_package) {
        the_package.found_user = true;
        the_package.user_data_dir = dir;
    }
    else {
        the_package.found_game = true;
        the_package.game_data_dir = dir;
    }

    // Merge package information (name already set above)

    // hidden: false overrides true
    if(user_package)
        the_package.hidden = info.hidden; // user is set first, take value directly
    else
        the_package.hidden = the_package.hidden && info.hidden;

    // user description overrides game description
    if(user_package || the_package.desc.empty())
        the_package.desc = info.desc;

    // user menu level overrides game menu level
    if(user_package || the_package.menu_level.empty())
        the_package.menu_level = info.menu_level;

    // dependencies are appended (user dependencies will therefore be first)
    the_package.dependencies.insert(the_package.dependencies.end(), info.dependencies.begin(), info.dependencies.end());
}

void cPackage_Manager :: Fix_Package_Paths( void )
{
    for(std::map<std::string, PackageInfo>::iterator it = m_packages.begin(); it != m_packages.end(); it++) {
        PackageInfo& i = it->second;

        if(i.user_data_dir.empty()) {
            // No user package path found, map from game package path
            fs::path rel = fs::relative(gp_app->Get_ResourceManager().Get_Game_Data_Directory() / utf8_to_path("packages"), i.game_data_dir);
            i.user_data_dir = gp_app->Get_ResourceManager().Get_User_Data_Directory() / utf8_to_path("packages") / rel;
        }
        else if(i.game_data_dir.empty()) {
            // No game package path found, map from user package path
            fs::path rel = fs::relative(gp_app->Get_ResourceManager().Get_User_Data_Directory() / utf8_to_path("packages"), i.user_data_dir);
            i.game_data_dir = gp_app->Get_ResourceManager().Get_Game_Data_Directory() / utf8_to_path("packages") / rel;
        }
    }
}

/**
 * Set the search path depending on all found packages.
 * Requires the global `gp_app` pointer, so don’t call this
 * before it has been set.
 */
void cPackage_Manager :: Build_Search_Path ( void )
{
    m_search_path.clear();
    m_package_start = 0;

    // First add skin package if any
    if(!gp_app->Get_Preferences().m_skin.empty()) {
        std::vector<std::string> processed;
        Build_Search_Path_Helper( gp_app->Get_Preferences().m_skin, processed );

        // The starting position in the search path for packages and not skin items
        m_package_start = m_search_path.size();
    }

    // Add the current package if any
    if(!m_current_package.empty()) {
        std::vector<std::string> processed;
        Build_Search_Path_Helper( m_current_package, processed );
    }

    // Add default data directories to search path
    m_search_path.push_back(gp_app->Get_ResourceManager().Get_User_Data_Directory());
    m_search_path.push_back(gp_app->Get_ResourceManager().Get_Game_Data_Directory());
}

void cPackage_Manager :: Build_Search_Path_Helper(const std::string& package, std::vector<std::string>& processed)
{
    // Avoid search loops
    if (std::find(processed.begin(), processed.end(), package) != processed.end())
        return;

    std::map<std::string, PackageInfo>::const_iterator item(m_packages.find(package));
    if (item == m_packages.end())
        return;

    // Add user and game search paths
    processed.push_back(package);
    m_search_path.push_back(item->second.user_data_dir);
    m_search_path.push_back(item->second.game_data_dir);

    // Add any dependent package search paths
    for (std::vector<std::string>::const_iterator dep_it = item->second.dependencies.begin(); dep_it != item->second.dependencies.end(); ++dep_it)
        Build_Search_Path_Helper(*dep_it, processed);
}

fs::path cPackage_Manager :: Find_Reading_Path(fs::path dir, fs::path resource, std::vector<std::string> extra_ext)
{
    fs::path path;
    for (std::vector<fs::path>::const_iterator it = m_search_path.begin(); it != m_search_path.end(); ++it) {
        path = *it / dir / resource;
        if (fs::exists(path)) {
            return path;
        }
        else {
            for (std::vector<std::string>::const_iterator it_ext = extra_ext.begin(); it_ext != extra_ext.end(); ++it_ext) {
                path.replace_extension(*it_ext);
                if (fs::exists(path)) {
                    return path;
                }
            }
        }
    }

    // Not found
    return fs::path();
}

fs::path cPackage_Manager :: Find_Relative_Path(fs::path dir, fs::path path)
{
    for (std::vector<fs::path>::const_iterator it = m_search_path.begin(); it != m_search_path.end(); ++it) {
        fs::path subdir(*it / dir);
        if (subdir.filename() == ".")
            subdir.remove_filename();

        // Determine if abspath is under subdir
        // This was borrowed from http://stackoverflow.com/questions/15541263/how-to-determine-if-file-is-contained-by-path-with-boost-filesystem-v3
        fs::path tmp_path(path);
        tmp_path.remove_filename();

        if (std::distance(subdir.begin(), subdir.end()) > std::distance(tmp_path.begin(), tmp_path.end()))
            continue;

        if (!std::equal(subdir.begin(), subdir.end(), tmp_path.begin()))
            continue;

        // Found the path in the search path that it is under
        return fs::relative(subdir, path);
    }

    return fs::path();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC
