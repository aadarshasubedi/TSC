#include "../core/bintree.hpp"
#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/property_helper.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_set.hpp"
#include "../objects/actor.hpp"
#include "../objects/sprite_actor.hpp"
#include "../objects/static_actor.hpp"
#include "../objects/animated_actor.hpp"
#include "../level/level_player.hpp"
#include "../core/collision.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_settings.hpp"
#include "../video/img_manager.hpp"
#include "../user/preferences.hpp"
#include "../core/tsc_app.hpp"
#include "../core/errors.hpp"
#include "../core/xml_attributes.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../core/filesystem/package_manager.hpp"
#include "level_loader.hpp"
#include "level.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

// Extern from global_game.hpp
cLevel* TSC::gp_current_level = NULL;

/**
 * Load a level from a file rather than creating a new one. The
 * level is not active; you have to add it to the cLevelScene
 * manually. Consider using the convenience overload
 * cLevelScene::Add_Level() that accepts a string level name, which
 * does all of this for you.
 *
 * \param levelfile
 * File to load the level from. This will not be touched, the file
 * will be opened exactly as specified; use cPackage_Manager if you
 * want to search for a level’s file path.
 *
 * \returns The loaded level.
 */
cLevel* cLevel::Load_From_File(fs::path filename)
{
    if (filename.empty())
        throw(InvalidLevelError("Empty level filename!"));
    if (!File_Exists(filename)) {
        std::string msg = "Level file not found: " + path_to_utf8(filename);
        throw (InvalidLevelError(msg));
    }

    // This is our loader
    cLevelLoader loader;

    // supported level format
    if (filename.extension() == fs::path(".tsclvl")  || filename.extension() == fs::path(".smclvl")) {
        loader.parse_file(filename);
    }
    else { // old, unsupported level format
        // OLD pHud_Debug->Set_Text(_("Unsupported Level format : ") + (const std::string)path_to_utf8(filename));
        return NULL;
    }

    // Our level
    cLevel* p_level = loader.Get_Level();

    // FIXME: Move this into cLevelLoader::on_end_document()
    /* late initialization
     * needed to create links to other objects
    */
    for(std::vector<cActor*>::iterator itr = p_level->m_actors.begin(); itr != p_level->m_actors.end(); ++itr) {
        cActor* obj = (*itr);

        obj->Init_Links();
    }

    debug_print("Loaded level: %s\n", path_to_utf8(p_level->m_level_filename).c_str());

    return p_level;
}

#ifdef _DEBUG
cLevel* cLevel::Construct_Debugging_Level()
{
    cLevel* p_level = new cLevel();
    p_level->m_level_filename = utf8_to_path("/tmp/debugging.tsclvl");

    cLevel_Player* p_player = new cLevel_Player();
    p_player->setPosition(50, -100);
    p_level->Add_Player(p_player);

    cStaticActor* p_actor = new cStaticActor(utf8_to_path("ground/green_3/ground/top/1.png"));
    p_actor->setPosition(0, 500); // TODO: Weird TSC coordinate system
    p_actor->m_name = "Ground 1";
    p_level->Add_Actor(p_actor);

    p_actor = new cStaticActor(utf8_to_path("ground/green_3/ground/top/1.png"));
    p_actor->setPosition(64, 500); // TODO: Weird TSC coordinate system
    p_actor->m_name = "Ground 2";
    p_level->Add_Actor(p_actor);

    p_actor = new cStaticActor(utf8_to_path("ground/green_3/ground/top/1.png"));
    p_actor->setPosition(128, 500); // TODO: Weird TSC coordinate system
    p_actor->m_name = "Ground 3";
    p_level->Add_Actor(p_actor);

    p_level->Sort_Z_Elements();
    return p_level;
}
#endif

/**
 * Create a new and empty level. Does NOT load levels from level files,
 * use the Load_From_File() static member function for that. Use
 * cPackage_Manager() to find out the correct saving path.
 *
 * \param levelfile
 * File to save the level to. This will be overwritten if it exists.
 *
 * \returns The new cLevel instance.
 */
cLevel::cLevel(fs::path levelfile)
{
    Init();
}

/**
 * This is a private constructor that is used under the hood by
 * Load_From_File() to create a level that is not yet associated
 * with a file (the public constructor overwrites files).
 */
cLevel::cLevel()
{
    Init();
}

cLevel::~cLevel()
{
    std::vector<sf::Sprite*>::iterator bgiter;
    for(bgiter=m_backgrounds.begin(); bgiter != m_backgrounds.end(); bgiter++)
        delete *bgiter;

    std::vector<cActor*>::iterator actiter;
    for(actiter=m_actors.begin(); actiter != m_actors.end(); actiter++)
        delete *actiter;
}

/**
 * Common basic initialisation for all constructors.
 */
void cLevel::Init()
{
    m_engine_version = level_engine_version;
    m_player_start_pos_x = cLevel_Player::m_default_pos_x;
    m_player_start_pos_y = cLevel_Player::m_default_pos_y;

    m_camera_limits.left = 0;
    m_camera_limits.top = 0;
    m_camera_limits.width = 4000;
    m_camera_limits.height = 1000;

    m_last_max_uid = 0;
    mp_level_player = NULL;

    cPreferences& prefs = gp_app->Get_Preferences();
    m_camera_view.reset(sf::FloatRect(0, 0, prefs.m_video_screen_w, prefs.m_video_screen_h));
}

void cLevel::Update()
{
    // Update all the actors.
    std::vector<cActor*>::iterator actiter;
    for(actiter=m_actors.begin(); actiter != m_actors.end(); actiter++)
        (*actiter)->Do_Update();

    std::unordered_map<unsigned long, Bintree<cCollision*> >::iterator iter;

    // Go through the detected collisions and act accordingly.
    for(iter=m_collisions.begin(); iter != m_collisions.end(); iter++) {
        iter->second.Traverse([] (const unsigned long& uid, cCollision*& p_coll) {
                //std::cout << "Collision bintree traverse: " << uid << std::endl;

                // Ignore NULL pointers (these shouldn't exit, but you never know.)
                if (!p_coll) {
                    std::cout << "Warning: Ignoring NULL pointer in collision." << std::endl;
                    return;
                }

                if (!p_coll->Get_Collision_Causer()->Handle_Collision(p_coll)) {
                    // Causer doesn't want to deal with this collision. Swap things
                    // round and fire on the sufferer instead.
                    p_coll->Invert();
                    p_coll->Get_Collision_Causer()->Handle_Collision(p_coll);
                }

                /* Collision data used, we don't need it anymore.
                 * As the entire collision list is deleted further below,
                 * I don’t bother setting the binary tree’s data member
                 * to a NULL pointer. */
                delete p_coll;
            });
    }

    m_collisions.clear();

    // Center camera view on player.
    // TODO: This centers on the player's upper-left corner...
    m_camera_view.setCenter(mp_level_player->getPosition());
}

void cLevel::Draw(sf::RenderWindow& stage) const
{
    // 1. Draw backgrounds
    std::vector<sf::Sprite*>::const_iterator bgiter;
    for(bgiter=m_backgrounds.begin(); bgiter != m_backgrounds.end(); bgiter++)
        stage.draw(**bgiter);

    // 2. Draw the actors in the level.
    std::vector<cActor*>::const_iterator actiter;
    for(actiter=m_actors.begin(); actiter != m_actors.end(); actiter++)
        (*actiter)->Draw(stage);
}

/**
 * Sort the actors and the backgrounds by their Z coordinates.
 *
 * \notice This is a very performance-intense operation that may
 * completely reorder the two arrays. Do not call this more often
 * than necessary, especially not once per frame!.
 *
 * TODO: Background are not yet sorted by this.
 */
void cLevel::Sort_Z_Elements()
{
    std::sort(m_actors.begin(), m_actors.end(), [] (const cActor* p_a, const cActor* p_b) {
            return p_a->Z() < p_b->Z();
        });
}

/**
 * Add the given collision to the list of collisions for this frame, if required.
 * For performance reasons, collision checking only happens when an actor moves;
 * thus, the collision object will always be created with the `causer` property
 * of the collision set to the moving actor and the `sufferer` property set to
 * the standing actor. If the standing actor really doesn’t move, this means
 * that only one collision is generated, and this will be added directly to
 * the list of collisions by this method.
 *
 * However, problems arise when both actors move. Then each of the two moving
 * actors will receive one collision object, with the actors being exactly
 * inverse. These technically two collisions however are logically only a
 * single one and should not be evaluated twice (this would for example cause
 * a Krush to be immediately killed rather than downgraded when you jump on it).
 * That’s what this method enforces:
 *
 * 1. It checks if the sufferer of the collision you want to add is already
 *    in the list of collisions. That’s not enough (could be a collision
 *    with another object), but it’s required.
 * 2. Check if the collision sufferer of the found collision is the same
 *    actor as the collision causer of the collision you’re about to add.
 *    If this is the case, you’re trying to add the described inversely
 *    mirrored collision that should not be counted. In that case, this
 *    method will not add the collision to the list of collisions to evaluate,
 *    but instead will *delete* the collision instance. You can't use it
 *    afterwards then!
 *
 * The equality check is done by utilising the UIDs of the actors, and
 * the collision list is a hashmap containing binary trees using the
 * UIDs as keys, thus these actions should be rather performant.
 *
 * \param[in] p_collision
 * The cCollision instance to add to the list of collisions to evaluate.
 * This will be memory-managed by this class afterwards, so don’t delete it;
 * this is true even if the collision is not needed (then it’s deleted
 * immediately).
 *
 * \remark This method assumes you’re only adding collisions from
 * the causer’s view of things. If you don’t, the algorithm described
 * above will fail. In other words: The causer member of `p_collision'
 * must be the moving actor.
 */
void cLevel::Add_Collision_If_Required(cCollision* p_collision)
{
    const unsigned long& myuid    = p_collision->Get_Collision_Causer()->Get_UID();
    const unsigned long& otheruid = p_collision->Get_Collision_Sufferer()->Get_UID();

    /* Important for understanding of this method is that it requires
     * and enforces the following to conditions on the `m_collisions'
     * member variable:
     * 1. A key in the toplevel `m_collisions' hashmap is always equal
     *    to the causer member of a collision saved inside the value
     *    of that key.
     * 2. A key in one of the binary trees stored inside the hashmap
     *    is always equal to the sufferer member of the cCollision instance
     *    stored inside that same binary tree node. */

    std::unordered_map<unsigned long, Bintree<cCollision*> >::iterator iter;
    iter = m_collisions.find(otheruid);

    // If the other UID hasn't even been seen yet, the can't be another collision.
    if (iter == m_collisions.end()) {
        /* Not yet seen. There cannot be another collision.
         * Construct new collision list with us as the only collision.
         * Use the sufferer's UID as the list key, because in one frame
         * an object may collide with multiple objects, which would cause
         * all items in the collision list to have the same key, which is
         * nonsense and not useful. */
        Bintree<cCollision*> newlist(otheruid, p_collision);
        m_collisions[myuid] = newlist;
    }
    else {
        // Okay, it *has* been seen. Let's see if there is an inverse collision.

        // First, get the list of collisions on the partner.
        const Bintree<cCollision*>& collist = iter->second;

        /* The collision list contains the sufferer's UID as keys. If our
         * UID is in there, the collision already exists (invertedly).
         * Prevent the dup and free the unneeded pointer. */
        const cCollision* p_coll = collist.Fetch(myuid, NULL);
        if (p_coll) {
            delete p_collision; // Not needed anymore
            return;
        }

        /* We are not in the collision list as a sufferer. No duplicate.
         * Add us. Note that what follows is important to ensure the
         * conditions documented at the top of this method. */
        iter = m_collisions.find(myuid);
        if (iter == m_collisions.end()) { // No list, construct new one
            Bintree<cCollision*> newlist(otheruid, p_collision);
            m_collisions[myuid] = newlist;
        }
        else { // Append to existing list
            iter->second.Insert(new Bintree<cCollision*>(otheruid, p_collision));
        }
    }
}

/**
 * Check all possible collisions on the given actor. This is
 * an expensive operation as it iterates the entire actor list.
 * When a collision is found, it is added via Add_Collision_If_Required()
 * to the list of collisions that are to be handled.
 *
 * This method does not honour the type of the actor you pass in. It
 * is assumed that you check if it is something like COLTYPE_PASSIVE
 * before calling this method (to prevent the extra calling overhead).
 *
 * \param[in] actor
 * The actor to check for collisions on. It is assumed to be the
 * collision causer (i.e. the moving object, not the standing one).
 */
void cLevel::Check_Collisions_For_Actor(cActor& actor)
{
    std::vector<cActor*>::const_iterator iter;
    for(iter=m_actors.begin(); iter != m_actors.end(); iter++) {
        cActor& other = **iter;

        // Cannot collide with itself.
        if (actor == other)
            continue;

        // If the other object is of a collidable type and
        // the rectangles intersect, consider a collision.
        if (other.Is_Collidable() && actor.Does_Collide(other)) {
            Add_Collision_If_Required(new cCollision(&actor, &other));
            // MRuby Touch event is not fired here, that’s fired when the collisions are handled.
        }
    }
}

/**
 * Add a new actor to the level. This method takes care of inserting it at
 * the correct position in the actors list by looking at its Z coordinate
 * (cActor::Z()). The cActor instance is memory-managed by this class
 * after you added it with this method.
 *
 * \param[in] p_actor
 * The cActor instance to add to the level.
 * \param[in] uid
 * (0) You can specify a UID to assign to this actor in this level.
 * If you don’t (or pass 0 here), then the next available UID will
 * be automatically used.
 *
 * \remark If the actor has already an UID set it will be ignored and
 * overwritten by this method. Use the `uid` element to assign a
 * UID you want; this is because UIDs are level-specific and if you
 * transplanted the same actor into another level, you’d have to
 * assign a new UID also.
 *
 * \remark If the actor has already a level set, it will be ignored
 * and overwritten by the level you’re adding it to.
 */
void cLevel::Add_Actor(cActor* p_actor, const unsigned long& uid /* = 0 */)
{
    /* UID 0 is reserved. If we get that, compute the next free
     * UID. Otherwise, assign the given UID and adjust the mantissa
     * accordingly. It could be considered to have the actor not know
     * its UID at all (as the UID is merely a key to find a cActor
     * instance), but somehow I don’t like that. */
    if (uid == 0) {
        p_actor->Added_To_Level(this, Get_Next_UID());
    }
    else {
        p_actor->Added_To_Level(this, uid);
    }

    /* Insert the new actor at the proper position in the actors array
     * depending on its Z position. This way we don’t have to re-sort
     * the entire array (using Sort_Z_Elements()). */
    float newz = p_actor->Z();
    std::vector<cActor*>::iterator iter;
    for(iter=m_actors.begin(); iter != m_actors.end(); iter++) {
        if ((*iter)->Z() >= newz) {
            break;
        }
    }
    m_actors.insert(iter, p_actor); // iter points to the past-the-end element, thus this is append operation if the new Z is high enough.
}

/**
 * Add Alex to the level. You can only call this once;
 * if you try otherwise, you’ll get an exception.
 */
void cLevel::Add_Player(cLevel_Player* p_levelplayer)
{
    if (mp_level_player) {
        throw(std::runtime_error("Tried to add multiple level players!"));
    }

    mp_level_player = p_levelplayer;

    mp_level_player->Added_To_Level(this, 0); // Reserved UID 0 for the level player
    m_actors.push_back(mp_level_player);
}

/**
 * Compare two levels. Two levels are equal if they belong to
 * the same level file.
 */
bool cLevel::operator==(const cLevel& other) const
{
    return m_level_filename == other.m_level_filename;
}

/**
 * Inverse of operator==().
 */
bool cLevel::operator!=(const cLevel& other) const
{
    return !(*this == other);
}

fs::path cLevel::Get_Music_Filename() const
{
    return gp_app->Get_PackageManager().Get_Relative_Music_Path(m_musicfile);
}

void cLevel::Set_Music(fs::path filename)
{
    // add music dir
    if (!filename.is_absolute())
        filename = gp_app->Get_PackageManager().Get_Music_Reading_Path(path_to_utf8(filename));

    // already set
    if (m_musicfile.compare(filename) == 0) {
        return;
    }

    m_musicfile = filename;
    // check if music is available
    m_valid_music = File_Exists(filename);
}

void cLevel::Set_Filename(fs::path filename, bool rename_old /* = true */)
{
    // erase file type and directory
    filename = Trim_Filename(filename, 0, 0);

    // if invalid
    if (path_to_utf8(filename).length() < 2) {
        return;
    }

    // add level file type
    if (filename.extension() != fs::path(".tsclvl"))
        filename.replace_extension(".tsclvl");

    // add level dir if we aren’t absolute yet
    filename = fs::absolute(filename, gp_app->Get_PackageManager().Get_User_Level_Path());

    // rename file
    if (rename_old) {
        fs::rename(m_level_filename, filename);
    }

    m_level_filename = filename;
}

void cLevel::Set_Author(const std::string& name)
{
    m_author = name;
}

void cLevel::Set_Version(const std::string& level_version)
{
    m_version = level_version;
}

void cLevel::Set_Description(const std::string& level_description)
{
    m_description = level_description;
}

void cLevel::Set_Difficulty(const uint8_t level_difficulty)
{
    m_difficulty = level_difficulty;

    // if invalid
    if (m_difficulty > 100) {
        m_difficulty = 0;
    }
}

void cLevel::Set_Land_Type(const LevelLandType level_land_type)
{
    m_land_type = level_land_type;
}
