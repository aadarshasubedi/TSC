#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/property_helper.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_set.hpp"
#include "../objects/actor.hpp"
#include "../objects/sprite_actor.hpp"
#include "../objects/animated_actor.hpp"
#include "../objects/box.hpp"
#include "../scenes/scene.hpp"
#include "../core/collision.hpp"
#include "../user/preferences.hpp"
#include "../core/scene_manager.hpp"
#include "../core/tsc_app.hpp"
#include "../core/math/utilities.hpp"
#include "level_player.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

// Milliseconds to enable power jump when ducking
static const int POWER_JUMP_DELTA = 1000;
// Jump physics factors default values
static const float JUMP_POWER = 17.0f;
static const float JUMP_ACCEL = 4.0f;

const float cLevel_Player::m_default_pos_x = 200.0f;
const float cLevel_Player::m_default_pos_y = -300.0f;

cLevel_Player::cLevel_Player()
    : cAnimatedActor()
{
    m_name = "Alex";

    Set_Collision_Type(COLTYPE_PLAYER);
    m_gravity_accel = 2.8f;
    m_gravity_max = 25.0f;

    m_state = STA_FALL;
    m_direction = DIR_RIGHT;

    m_alex_type = ALEX_SMALL;

    m_god_mode = 0;

    m_walk_time = 0.0f;
    m_ghost_time = 0.0f;
    m_ghost_time_mod = 0.0f;

    // Starting with 3 lives
    m_lives = 3;
    m_goldpieces = 0;
    m_points = 0;
    m_kill_multiplier = 1.0f;
    m_last_kill_counter = 0.0f;

    // jump data
    m_up_key_time = 0.0f;
    m_force_jump = 0;
    m_next_jump_sound = 1;
    m_next_jump_power = JUMP_POWER;
    m_next_jump_accel = JUMP_ACCEL;
    m_jump_power = 0.0f;
    m_jump_accel_up = 4.5f;
    m_jump_vel_deaccel = 0.06f;

    // no movement timer
    m_no_velx_counter = 0.0f;
    m_no_vely_counter = 0.0f;

    m_shoot_counter = 0.0f;
    mp_active_object = NULL;
    m_duck_direction = DIR_UNDEFINED;

    m_is_warping = false;
    mp_active_object = NULL;

    Add_Image_Set("small_stand_left"          , "alex/small/stand_left.imgset");
    Add_Image_Set("small_stand_right"         , "alex/small/stand_right.imgset");
    Add_Image_Set("small_stand_left_holding"  , "alex/small/stand_left_holding.imgset");
    Add_Image_Set("small_stand_right_holding" , "alex/small/stand_right_holding.imgset");
    Add_Image_Set("small_walk_left"           , "alex/small/walk_left.imgset");
    Add_Image_Set("small_walk_right"          , "alex/small/walk_right.imgset");
    Add_Image_Set("small_walk_left_holding"   , "alex/small/walk_left_holding.imgset");
    Add_Image_Set("small_walk_right_holding"  , "alex/small/walk_right_holding.imgset");
    Add_Image_Set("small_fall_left"           , "alex/small/fall_left.imgset");
    Add_Image_Set("small_fall_right"          , "alex/small/fall_right.imgset");
    Add_Image_Set("small_fall_left_holding"   , "alex/small/fall_left_holding.imgset");
    Add_Image_Set("small_fall_right_holding"  , "alex/small/fall_right_holding.imgset");
    Add_Image_Set("small_jump_left"           , "alex/small/jump_left.imgset");
    Add_Image_Set("small_jump_right"          , "alex/small/jump_right.imgset");
    Add_Image_Set("small_jump_left_holding"   , "alex/small/jump_left_holding.imgset");
    Add_Image_Set("small_jump_right_holding"  , "alex/small/jump_right_holding.imgset");
    Add_Image_Set("small_dead_left"           , "alex/small/dead_left.imgset");
    Add_Image_Set("small_dead_right"          , "alex/small/dead_right.imgset");
    Add_Image_Set("small_duck_left"           , "alex/small/duck_left.imgset");
    Add_Image_Set("small_duck_right"          , "alex/small/duck_right.imgset");
    Add_Image_Set("small_climb_left"          , "alex/small/climb_left.imgset");
    Add_Image_Set("small_climb_right"         , "alex/small/climb_right.imgset");

    Load_Images(true);
}

cLevel_Player::~cLevel_Player()
{
    //
}

void cLevel_Player::Load_Images(bool new_startimage /* = false */)
{
    // not valid
    if (m_alex_type == ALEX_DEAD) {
        return;
    }

    // special alex images state
    std::string imgsetstring;

    // powerup type
    switch (m_alex_type) {
    case ALEX_SMALL:
        imgsetstring += "small";
        break;
    case ALEX_BIG:
        imgsetstring += "big";
        break;
    case ALEX_FIRE:
        imgsetstring += "fire";
        break;
    case ALEX_ICE:
        imgsetstring += "ice";
        break;
    case ALEX_CAPE:
        imgsetstring += "flying";
        break;
    case ALEX_GHOST:
        imgsetstring += "ghost";
        break;
    default:
        std::cerr << "Warning: Unhandled powerup type on player when setting image set." << std::endl;
        imgsetstring += "small";
        break;
    }

    // moving type
    switch(m_state) {
    case STA_STAY:
        imgsetstring += "_stand";
        break;
    case STA_WALK:
        imgsetstring += "_walk";
        break;
    case STA_FALL:
        imgsetstring += "_fall";
        break;
    case STA_JUMP:
        imgsetstring += "_jump";
        break;
    case STA_CLIMB:
        imgsetstring += "_climb";
        break;
    default:
        std::cerr << "Warning: Unhandled moving state on player when setting image set." << std::endl;
        imgsetstring += "_walk";
        break;
    }

    // direction prefix
    if (m_direction == DIR_LEFT)
        imgsetstring += "_left";
    else if (m_direction == DIR_RIGHT)
        imgsetstring += "_right";
    else {
        std::cerr << "Warning: Unhandled direction on player when setting image set." << std::endl;
        imgsetstring += "right";
    }

    // if holding item
    if (mp_active_object) {
        imgsetstring += "_holding";
    }

    //debug_print("Load_Images() constructed image set name: '%s'\n", imgsetstring.c_str());

    Set_Image_Set(imgsetstring, new_startimage);
}

void cLevel_Player::Update()
{
    cAnimatedActor::Update();

    // OLD if (editor_enabled) {
    // OLD   return;
    // OLD }

    // check if got stuck
    if (!m_ducked_counter) {
        // OLD Update_Anti_Stuck();
    }

    // check if starting a jump is possible
    Update_Jump_Keytime();

    // update states
    Update_Jump();
    // OLD Update_Climbing();
    // OLD Update_Falling();
    Update_Walking();
    // OLD Update_Running();
    // OLD Update_Ducking();
    Update_Staying();
    // OLD Update_Flying();
}

void cLevel_Player::Update_Walking(void)
{
    // OLD if (m_ducked_counter || !m_ground_object || (m_state != STA_STAY && m_state != STA_WALK && m_state != STA_RUN)) {
    // OLD    return;
    // OLD}

    // OLD // validate ground object in special cases (otherwise done by cActor::Update_Position() anyway).
    // OLD if ((m_ground_object->m_type == TYPE_EATO || m_ground_object->m_type == TYPE_SPIKA || m_ground_object->m_type == TYPE_ROKKO || m_ground_object->m_type == TYPE_STATIC_ENEMY) && m_invincible <= 0 && !m_god_mode) {
    // OLD     Reset_On_Ground();
    // OLD }

    cPreferences& preferences = gp_app->Get_Preferences();
    if (sf::Keyboard::isKeyPressed(preferences.m_key_left) || sf::Keyboard::isKeyPressed(preferences.m_key_right) /* || TODO: joystick */) {
        float ground_mod = 1.0f;

        if (mp_ground_object) {
            // ground type
            switch (mp_ground_object->m_ground_type) {
            case GROUND_ICE:
                ground_mod = 0.5f;
                break;
            case GROUND_SAND:
                ground_mod = 0.7f;
                break;
            case GROUND_PLASTIC:
                ground_mod = 0.85f;
                break;
            default:
                break;
            }
        }

        Move_Player((0.6f * ground_mod * Get_Vel_Modifier()), (1.2f * ground_mod * Get_Vel_Modifier()));
    }

    // If the player moves faster than 10px/frame, upgrade his state to
    // running. Move_Player() above is responsible for setting the
    // velocity.
    if (m_state == STA_WALK) {
        // update
        if (m_velocity.x > 10.0f || m_velocity.x < -10.0f) {
            m_walk_time += gp_app->Get_SceneManager().Get_Speedfactor();

            if (m_walk_time > speedfactor_fps) {
                Set_Moving_State(STA_RUN);
            }
        }
        // reset
        else if (m_walk_time) {
            m_walk_time = 0.0f;
        }
    }
}

void cLevel_Player::Update_Staying(void)
{
    // only if player is onground
    if (!mp_ground_object || m_ducked_counter || m_state == STA_JUMP || m_state == STA_CLIMB) {
        return;
    }

    cPreferences& preferences = gp_app->Get_Preferences();

    // if left and right is not pressed
    if (!sf::Keyboard::isKeyPressed(preferences.m_key_left) && !sf::Keyboard::isKeyPressed(preferences.m_key_right) /* && !pJoystick->m_left && !pJoystick->m_right */) {
        // walking
        if (m_velocity.x) {
            /* OLD if (m_ground_object->m_image && m_ground_object->m_image->m_ground_type == GROUND_ICE) {
                Auto_Slow_Down(1.1f / 5.0f);
            }
            else { */
                Auto_Slow_Down(1.1f);
            /* } */

            // stopped walking
            if (!m_velocity.x) {
                Set_Moving_State(STA_STAY);
            }
        }

        // walk on spika
        // OLD if (m_ground_object->m_type == TYPE_SPIKA) {
        // OLD     cMovingSprite* moving_ground_object = static_cast<cMovingSprite*>(m_ground_object);
        // OLD 
        // OLD     if (moving_ground_object->m_velx < 0.0f) {
        // OLD         m_walk_count -= moving_ground_object->m_velx * pFramerate->m_speed_factor * 0.1f;
        // OLD     }
        // OLD     else if (moving_ground_object->m_velx > 0.0f) {
        // OLD         m_walk_count += moving_ground_object->m_velx * pFramerate->m_speed_factor * 0.1f;
        // OLD     }
        // OLD }
        // if not moving anymore
        else if (Is_Float_Equal(m_velocity.x, 0.0f)) {
            m_walk_count = 0.0f;
        }
    }

    // if staying don't move vertical
    if (m_velocity.y > 0.0f) {
        m_velocity.y = 0.0f;
    }
}

bool cLevel_Player::Handle_Collision_Massive(cCollision* p_collision)
{
    cAnimatedActor::Handle_Collision_Massive(p_collision);

    /* fixme: the collision direction is sometimes wrong as left/right if landing on a moving platform which moves upwards
    * this seems to cause alex to hang or to get stuck in it
    * easily reproducible with a low framerate and fast moving platform
    */
    //printf( "direction is %s\n", Get_Direction_Name( collision->m_direction ).c_str() );

    if (p_collision->Is_Collision_Top()) {
        m_velocity.y = 0; // Player hit the ceiling

        if (m_state != STA_FLY) {
            if (m_state != STA_CLIMB) {
                Set_Moving_State(STA_FALL);

                // If the object on top is moving downwards (like a massive moving
                // platform coming down to squash Alex) increase Alex’ downwards
                // velocity accordingly.
                cActor* p_moving_object = p_collision->Get_Collision_Sufferer();

                // add its velocity
                if (p_moving_object->m_velocity.y > 0) {
                    m_velocity.y += p_moving_object->m_velocity.y;
                }

                if (p_moving_object->Get_Collision_Type() == COLTYPE_MASSIVE) {
                    // OLD pAudio->Play_Sound("wall_hit.wav", RID_ALEX_WALL_HIT);
                    // OLD 
                    // OLD // create animation
                    // OLD cParticle_Emitter* anim = new cParticle_Emitter(m_sprite_manager);
                    // OLD anim->Set_Emitter_Rect(m_col_rect.m_x, m_col_rect.m_y + 6, m_col_rect.m_w);
                    // OLD anim->Set_Image(pVideo->Get_Package_Surface("animation/particles/light.png"));
                    // OLD anim->Set_Quota(4);
                    // OLD anim->Set_Pos_Z(m_pos_z - 0.000001f);
                    // OLD anim->Set_Time_to_Live(0.3f);
                    // OLD anim->Set_Color(Color(static_cast<Uint8>(150), 150, 150, 200), Color(static_cast<Uint8>(rand() % 55), rand() % 55, rand() % 55, 0));
                    // OLD anim->Set_Speed(2, 0.6f);
                    // OLD anim->Set_Scale(0.6f);
                    // OLD anim->Set_Direction_Range(0, 180);
                    // OLD anim->Set_Fading_Alpha(1);
                    // OLD anim->Set_Fading_Size(1);
                    // OLD anim->Emit();
                    // OLD pActive_Animation_Manager->Add(anim);
                }
            }
        }
        // flying
        else {
            m_velocity.x -= m_velocity.x * 0.05f * gp_app->Get_SceneManager().Get_Speedfactor();

            // too slow
            if (m_velocity.x > -5.0f && m_velocity.x < 5.0f) {
                Stop_Flying();
            }
        }

    }
    else if (p_collision->Is_Collision_Bottom()) {
        // flying
        if (m_state == STA_FLY) {
            m_velocity.y = 0.0f;
            m_velocity.x -= m_velocity.x * 0.05f * gp_app->Get_SceneManager().Get_Speedfactor();

            // too slow
            if (m_velocity.x > -5.0f && m_velocity.x < 5.0f) {
                Stop_Flying();
            }
        }
        // not flying
        else {
            // jumping
            if (m_state == STA_JUMP) {
                /* hack : only pick up after some time of jumping because moving objects collide move before the player
                 * also see cMovingSprite::Validate_Collision_Object_On_Top
                 *
                 * FIXME: Is this out of date with the SFML changes? -- Quintus, 2015-08-09
                */
                if (m_jump_power < 6.0f) {
                    m_velocity.y = 0.0f;
                }
            }
            // all other states
            else {
                m_velocity.y = 0.0f;
                Set_Moving_State(STA_STAY);
            }
        }
    }
    else if (p_collision->Is_Collision_Left()) {
        m_velocity.x = 0.0f;
        m_velocity.y -= m_velocity.y * 0.01f * gp_app->Get_SceneManager().Get_Speedfactor();
        Set_On_Side(*p_collision->Get_Collision_Sufferer(), DIR_RIGHT);

        if (m_state == STA_WALK || m_state == STA_RUN) {
            m_walk_count = 0.0f;
        }
        // flying
        else if (m_state == STA_FLY) {
            // box collision
            // OLD if (col_obj->m_type == TYPE_BONUS_BOX || col_obj->m_type == TYPE_SPIN_BOX) {
            // OLD     cBaseBox* box = static_cast<cBaseBox*>(col_obj);
            // OLD     box->Activate_Collision(collision->m_direction);
            // OLD }

            Stop_Flying();
        }
    }
    else if (p_collision->Is_Collision_Right()) {
        m_velocity.x = 0.0f;
        m_velocity.y -= m_velocity.y * 0.01f * gp_app->Get_SceneManager().Get_Speedfactor();
        Set_On_Side(*p_collision->Get_Collision_Sufferer(), DIR_LEFT);

        if (m_state == STA_WALK || m_state == STA_RUN) {
            m_walk_count = 0.0f;
        }
        // flying
        else if (m_state == STA_FLY) {
            // box collision
            // OLD if (col_obj->m_type == TYPE_BONUS_BOX || col_obj->m_type == TYPE_SPIN_BOX) {
            // OLD     cBaseBox* box = static_cast<cBaseBox*>(col_obj);
            // OLD     box->Activate_Collision(collision->m_direction);
            // OLD }

            Stop_Flying();
        }
    }

    // OLD if (collision->m_array == ARRAY_ACTIVE) {
    // OLD     // send collision
    // OLD     Send_Collision(collision);
    // OLD }


    return false;
}

void cLevel_Player::Move_Player(float velocity, float vel_wrongway)
{
    if (m_direction == DIR_LEFT) {
        velocity *= -1;
        vel_wrongway *= -1;
    }

    // OLD // get collision list
    // OLD cObjectCollisionType* col_list = Collision_Check_Relative(velocity, 0.0f, 0.0f, 0.0f, COLLIDE_ONLY_BLOCKING);
    // OLD // if collision with a blocking object
    // OLD bool is_col = 0;
    // OLD 
    // OLD // check collisions
    // OLD for (cObjectCollision_List::iterator itr = col_list->objects.begin(); itr != col_list->objects.end(); ++itr) {
    // OLD     cObjectCollision* col_obj = (*itr);
    // OLD 
    // OLD     // massive object is blocking
    // OLD     if (col_obj->m_obj->m_massive_type == MASS_MASSIVE) {
    // OLD         is_col = 1;
    // OLD         break;
    // OLD     }
    // OLD }
    // OLD 
    // OLD delete col_list;
    // OLD 
    // OLD // don't move if colliding
    // OLD if (is_col) {
    // OLD     if (Is_Float_Equal(m_velx, 0.0f)) {
    // OLD         Set_Moving_State(STA_STAY);
    // OLD     }
    // OLD     // slow down
    // OLD     else {
    // OLD         m_velx -= (m_velx * 0.1f) * pFramerate->m_speed_factor;
    // OLD 
    // OLD         if (m_velx > -0.1f && m_velx < 0.1f) {
    // OLD             m_velx = 0.0f;
    // OLD         }
    // OLD     }
    // OLD 
    // OLD     return;
    // OLD }

    // move right
    if (m_direction == DIR_RIGHT) {
        if (m_velocity.x > 0.0f) {
            Add_Velocity_X_Max(velocity, 10.0f * Get_Vel_Modifier());
        }
        else {
            // small smoke clouds under foots
            if (m_velocity.x < 0.0f) {
                // OLD Generate_Feet_Clouds();
            }

            // slow down
            Add_Velocity_X(vel_wrongway);
        }
    }
    // move left
    else if (m_direction == DIR_LEFT) {
        if (m_velocity.x < 0.0f) {
            Add_Velocity_X_Min(velocity, -10.0f * Get_Vel_Modifier());
        }
        else {
            // small smoke clouds under foots
            if (m_velocity.x > 0.0f) {
                // OLD Generate_Feet_Clouds();
            }

            // slow down
            Add_Velocity_X(vel_wrongway);
        }
    }

    // start walking
    if (m_state == STA_STAY) {
        Set_Moving_State(STA_WALK);
    }
}

float cLevel_Player::Get_Vel_Modifier(void) const
{
    float vel_mod = 1.0f;

    // if running key is pressed or always run
    cPreferences& preferences = gp_app->Get_Preferences();
    if (preferences.m_always_run || sf::Keyboard::isKeyPressed(preferences.m_key_action) /* || TODO: Joystick */) {
        vel_mod = 1.5f;
    }

    if (m_invincible_star > 0.0f) {
        vel_mod *= 1.2f;
    }

    if (m_state == STA_RUN) {
        vel_mod *= 1.2f;
    }

    return vel_mod;
}

void cLevel_Player::Set_Moving_State(Moving_state new_state)
{
    if (new_state == m_state) {
        return;
    }

    // can not change from linked state
    if (m_state == STA_OBJ_LINKED) {
        return;
    }

    // was falling
    if (m_state == STA_FALL) {
        // reset slow fall/parachute
        // OLD Parachute(0);
    }
    // was flying
    else if (m_state == STA_FLY) {
        // OLD Change_Size(0, -(m_images[ALEX_IMG_FALL].m_image->m_h - m_images[ALEX_IMG_FLY].m_image->m_h));
        // OLD Set_Image_Num(ALEX_IMG_FALL + m_direction);
        // OLD 
        // OLD // reset flying rotation
        // OLD m_rot_z = 0.0f;
    }
    // was jumping
    else if (m_state == STA_JUMP) {
        // reset variables
        m_up_key_time = 0.0f;
        m_jump_power = 0.0f;
        m_force_jump = 0;
        m_next_jump_sound = 1;
        m_next_jump_power = JUMP_POWER;
        m_next_jump_accel = JUMP_ACCEL;
    }
    // was running
    else if (m_state == STA_RUN) {
        m_walk_time = 0.0f;
        m_running_particle_counter = 0.0f;
    }

    // # before new state is set
    if (new_state == STA_STAY) {
        if (mp_ground_object) {
            m_velocity.y = 0.0f;
        }
    }
    else if (new_state == STA_FALL) {
        Reset_On_Ground();
        m_walk_count = 0.0f;
        m_jump_power = 0.0f;
    }
    else if (new_state == STA_FLY) {
        Reset_On_Ground();
    }
    else if (new_state == STA_JUMP) {
        Reset_On_Ground();
    }
    else if (new_state == STA_CLIMB) {
        Reset_On_Ground();
        m_velocity.x = 0.0f;
        m_velocity.y = 0.0f;
    }

    m_state = new_state;
    Load_Images();

    // # after new state is set
    if (m_state == STA_FLY) {
        // OLD Release_Item(1);
    }
}

void cLevel_Player::Action_Interact(input_identifier key_type)
{
    // TODO
    // This method mainly checks for level exits and warps
    // There’s MUCH to do here still!
    if (key_type == INP_UP) {
        // OLD <missing>
    }
    else if (key_type == INP_DOWN) {
        // OLD <missing>
    }
    else if (key_type == INP_LEFT) {
        // Search for colliding level exit objects
        // OLD <missing>

        // direction
        if (m_state != STA_FLY) {
            if (m_direction != DIR_LEFT) {
                // play stop sound if already running
                // OLD if (m_velx > 12.0f && m_ground_object) {
                // OLD     pAudio->Play_Sound("player/run_stop.ogg", RID_ALEX_STOP);
                // OLD }

                m_direction = DIR_LEFT;
                Load_Images();
            }
        }
    }
    else if (key_type == INP_RIGHT) {
        // Search for colliding level exit objects
        // OLD <missing>

        // direction
        if (m_state != STA_FLY) {
            if (m_direction != DIR_RIGHT) {
                // play stop sound if already running
                // OLD if (m_velx < -12.0f && m_ground_object) {
                // OLD     pAudio->Play_Sound("player/run_stop.ogg", RID_ALEX_STOP);
                // OLD }

                m_direction = DIR_RIGHT;
                Load_Images();
            }
        }

    }
    else if (key_type == INP_SHOOT) {
        // FIXME: This one is appearently superflous, because
        // the shoot key is handled one layer up at cLevel_Scene::Handle_Keydown_Event().
        // OLD Action_Shoot();
    }
    else if (key_type == INP_JUMP) {
        // FIXME: Same here
        Action_Jump();
    }
    else if (key_type == INP_ITEM) {
        // OLD pHud_Itembox->Request_Item();
    }
    else if (key_type == INP_EXIT) {
        // OLD <missing>
    }
}

void cLevel_Player::Action_Jump(bool enemy_jump /* = 0 */)
{
    if (m_ducked_counter) {
        // power jump
        if (m_ducked_counter > POWER_JUMP_DELTA) {
            m_force_jump = 1;
            m_next_jump_power += 2.0f;
            m_next_jump_accel += 0.2f;
        }
        // stop ducking after setting power jump
        Stop_Ducking();
    }

    // enemy jump
    if (enemy_jump) {
        m_force_jump = 1;
        m_next_jump_sound = 0;
        m_next_jump_power += 1.0f;
        m_next_jump_accel += 0.1f;
    }

    // start keytime
    Start_Jump_Keytime();
    // check if starting a jump is possible
    Update_Jump_Keytime();
}

void cLevel_Player::Action_Stop_Interact(input_identifier key_type)
{
    cPreferences& preferences = gp_app->Get_Preferences();

    // Action
    if (key_type == INP_ACTION) {
        // OLD Release_Item();
    }
    // Down
    else if (key_type == INP_DOWN) {
        // OLD Stop_Ducking();
    }
    // Left
    else if (key_type == INP_LEFT) {
        // if key in opposite direction is still pressed only change direction
        if (sf::Keyboard::isKeyPressed(preferences.m_key_right) /* || pJoystick->m_right */) {
            m_direction = DIR_RIGHT;
        }
        else {
            Hold();
        }
    }
    // Right
    else if (key_type == INP_RIGHT) {
        // if key in opposite direction is still pressed only change direction
        if (sf::Keyboard::isKeyPressed(preferences.m_key_left) /* || pJoystick->m_left */) {
            m_direction = DIR_LEFT;
        }
        else {
            Hold();
        }
    }
    // Jump
    else if (key_type == INP_JUMP) {
        Action_Stop_Jump();
    }
    // Shoot
    else if (key_type == INP_SHOOT) {
        Action_Stop_Shoot();
    }
}

void cLevel_Player::Action_Stop_Jump(void)
{
    Stop_Flying();
    m_up_key_time = 0;
}

void cLevel_Player::Action_Stop_Shoot(void)
{
    // nothing
}

 void cLevel_Player::Hold(void)
{
    if (!mp_ground_object || (m_state != STA_WALK && m_state != STA_RUN)) {
        return;
    }

    Set_Moving_State(STA_STAY);
}

void cLevel_Player::Stop_Ducking(void)
{
    if (!m_ducked_counter) {
        return;
    }

    // OLD // get space needed to stand up
    // OLD const float move_y = -(m_images[ALEX_IMG_STAND].m_image->m_col_h - m_image->m_col_h);
    // OLD 
    // OLD cObjectCollisionType* col_list = Collision_Check_Relative(0.0f, move_y, 0.0f, 0.0f, COLLIDE_ONLY_BLOCKING);
    // OLD 
    // OLD // failed to stand up because something is blocking
    // OLD if (col_list->size()) {
    // OLD     // set ducked time again to stop possible power jump while in air
    // OLD     m_ducked_counter = 1;
    // OLD     delete col_list;
    // OLD     return;
    // OLD }
    // OLD 
    // OLD delete col_list;
    // OLD 
    // OLD // unset ducking image ( without Check_out_of_Level from cMovingSprite )
    // OLD cSprite::Move(0.0f, move_y, 1);
    // OLD Set_Image_Num(ALEX_IMG_STAND + m_direction);
    // OLD 
    // OLD m_ducked_counter = 0;
    // OLD m_ducked_animation_counter = 0.0f;
    // OLD Set_Moving_State(STA_STAY);
}

void cLevel_Player::Start_Jump_Keytime(void)
{
    cPreferences& preferences = gp_app->Get_Preferences();

    if (m_god_mode || m_state == STA_STAY || m_state == STA_WALK || m_state == STA_RUN || m_state == STA_FALL || m_state == STA_FLY || m_state == STA_JUMP || (m_state == STA_CLIMB && !sf::Keyboard::isKeyPressed(preferences.m_key_up))) {
        m_up_key_time = speedfactor_fps / 4;
    }
}

void cLevel_Player::Update_Jump_Keytime(void)
{
    // handle jumping start
    if (m_force_jump || (m_up_key_time && (mp_ground_object || m_god_mode || m_state == STA_CLIMB))) {
        Start_Jump();
    }
}

void cLevel_Player::Start_Jump(float deaccel /* = 0.08f */)
{
    // play sound
    if (m_next_jump_sound) {
        // small
        if (m_alex_type == ALEX_SMALL) {
            if (m_force_jump) {
                // OLD pAudio->Play_Sound("player/jump_small_power.ogg", RID_ALEX_JUMP);
            }
            else {
                // OLD pAudio->Play_Sound("player/jump_small.ogg", RID_ALEX_JUMP);
            }
        }
        // ghost
        else if (m_alex_type == ALEX_GHOST) {
            // OLD pAudio->Play_Sound("player/jump_ghost.ogg", RID_ALEX_JUMP);
        }
        // big
        else {
            if (m_force_jump) {
                // OLD pAudio->Play_Sound("player/jump_big_power.ogg", RID_ALEX_JUMP);
            }
            else {
                // OLD pAudio->Play_Sound("player/jump_big.ogg", RID_ALEX_JUMP);
            }
        }
    }

    bool jump_key = 0;
    cPreferences& preferences = gp_app->Get_Preferences();

    // if jump key pressed
    if (sf::Keyboard::isKeyPressed(preferences.m_key_jump) /* || (pPreferences->m_joy_analog_jump && pJoystick->m_up) || pJoystick->Button(pPreferences->m_joy_button_jump) */ ) {
        jump_key = 1;
    }

    // todo : is this needed ?
    // avoid that we are set on the ground again
    // OLD Col_Move(0.0f, -1.0f, 1, 1);

    // fly
    if (m_alex_type == ALEX_CAPE && !m_force_jump && m_state == STA_RUN && jump_key && ((m_direction == DIR_RIGHT && m_velocity.x > 14) || (m_direction == DIR_LEFT && m_velocity.x < -14))) {
        m_velocity.y = -m_next_jump_power * 0.5f;
        Set_Moving_State(STA_FLY);
    }
    // jump
    else {
        m_jump_accel_up = m_next_jump_accel;
        m_jump_vel_deaccel = deaccel;

        if (jump_key) {
            m_jump_power = m_next_jump_power * 0.59f;
        }
        else {
            m_jump_power = m_next_jump_power * 0.12f;
        }

        // Issue jump event
        // OLD Scripting::cJump_Event evt;
        // OLD evt.Fire(pActive_Level->m_mruby, this);

        m_velocity.y = -m_next_jump_power;
        Set_Moving_State(STA_JUMP);
    }

    // jump higher when running
    if (m_velocity.x < 0.0f) {
        m_velocity.y += m_velocity.x / 9.5f;
    }
    else if (m_velocity.x > 0.0f) {
        m_velocity.y -= m_velocity.x / 9.5f;
    }

    // slow down if running
    m_velocity.x = m_velocity.x * 0.9f;

    // jump with velx if ducking but only into the opposite start duck direction to get out of a hole
    if (m_ducked_counter) {
        if (m_direction == DIR_RIGHT && m_duck_direction != m_direction) {
            if (m_velocity.x < 5.0f) {
                m_velocity.x += 2.0f;
            }
        }
        else if (m_direction == DIR_LEFT && m_duck_direction != m_direction) {
            if (m_velocity.x > -5.0f) {
                m_velocity.x -= 2.0f;
            }
        }
    }

    // reset variables
    m_up_key_time = 0.0f;
    m_force_jump = 0;
    m_next_jump_sound = 1;
    m_next_jump_power = JUMP_POWER;
    m_next_jump_accel = JUMP_ACCEL;
}

void cLevel_Player::Update_Jump(void)
{
    // jumping keytime
    if (m_up_key_time) {
        m_up_key_time -= gp_app->Get_SceneManager().Get_Speedfactor();

        if (m_up_key_time < 0.0f) {
            m_up_key_time = 0.0f;
        }
    }

    // only if jumping
    if (m_state != STA_JUMP) {
        return;
    }

    cPreferences& preferences = gp_app->Get_Preferences();

    // jumping physics
    if (sf::Keyboard::isKeyPressed(preferences.m_key_jump) /* || (pPreferences->m_joy_analog_jump && pJoystick->m_up) || pJoystick->Button(pPreferences->m_joy_button_jump) */ ) {
        Add_Velocity_Y(-(m_jump_accel_up + (m_velocity.y * m_jump_vel_deaccel) / Get_Vel_Modifier()));
        m_jump_power -= gp_app->Get_SceneManager().Get_Speedfactor();
    }
    else {
        Add_Velocity_Y(0.5f);
        m_jump_power -= 6.0f * gp_app->Get_SceneManager().Get_Speedfactor();
    }

    // left right physics
    if ((sf::Keyboard::isKeyPressed(preferences.m_key_left) /* || pJoystick->m_left */) && !m_ducked_counter) {
        const float max_vel = -10.0f * Get_Vel_Modifier();

        if (m_velocity.x > max_vel) {
            Add_Velocity_X_Min((-1.1f * Get_Vel_Modifier()) - (m_velocity.x / 100), max_vel);
        }

    }
    else if ((sf::Keyboard::isKeyPressed(preferences.m_key_right) /* || pJoystick->m_right */) && !m_ducked_counter) {
        const float max_vel = 10.0f * Get_Vel_Modifier();

        if (m_velocity.x < max_vel) {
            Add_Velocity_X_Max((1.1f * Get_Vel_Modifier()) + (m_velocity.x / 100), max_vel);
        }
    }
    // slow down
    else {
        Auto_Slow_Down(0.2f);
    }

    // if no more jump power set to falling
    if (m_jump_power <= 0.0f) {
        Set_Moving_State(STA_FALL);
    }
}

void cLevel_Player::Stop_Flying(bool parachute /* = 1 */)
{
    if (m_state != STA_FLY) {
        return;
    }

    if (parachute) {
        Parachute(1);
    }

    Set_Moving_State(STA_FALL);
}

void cLevel_Player::Parachute(bool enable)
{
    if (m_parachute == enable) {
        return;
    }

    m_parachute = enable;

    if (m_parachute) {
        m_gravity_max = 10.0f;
    }
    else {
        m_gravity_max = 25.0f;
    }
}

void cLevel_Player::DownGrade(bool force /* = 0 */)
{
    DownGrade_Player(true, force);
}

void cLevel_Player::DownGrade_Player(bool delayed /* = true */, bool force /* = false */, bool ignore_invincible /* = false */)
{
    if (m_god_mode)
        return;
    if (m_invincible && !ignore_invincible)
        return;

    // already dead
    if (m_alex_type == ALEX_DEAD) {
        return;
    }

    /* FIXME: Does it have any use to delay the downgrade to the next
     * frame? I guess not, so I comment this out now... -- Quintus
    if (delayed) {
        Game_Action = GA_DOWNGRADE_PLAYER;
        if (force) {
            Game_Action_Data_Middle.add("downgrade_force", "1");
            if (ignore_invincible) {
                Game_Action_Data_Middle.add("downgrade_ignore_invincible", "1");
            }
        }

        return;
        } */

    // if not weakest state or not forced
    if (m_alex_type != ALEX_SMALL && !force) {
        // OLD pAudio->Play_Sound("player/powerdown.ogg", RID_ALEX_POWERDOWN);

        // power down
        Set_Type(ALEX_SMALL);

        m_invincible = speedfactor_fps * 2.5f;
        m_invincible_mod = 0.0f;

        // OLD pHud_Itembox->Request_Item();

        // Issue the Downgrade event
        // OLD Scripting::cDowngrade_Event evt(1, 2); // downgrades = 1, max. downgrades = 2
        // OLD evt.Fire(pActive_Level->m_mruby, this);

        return;
    }

    Set_Type(ALEX_DEAD, 0, 0);
    // OLD pHud_Time->Reset();
    // OLD pHud_Points->Clear();
    Ball_Clear();
    // OLD pHud_Lives->Add_Lives(-1);
    // OLD pAudio->Fadeout_Music(1700);

    // lost a live
    if (m_lives >= 0) {
        // OLD pAudio->Play_Sound(utf8_to_path("player/dead.ogg"), RID_ALEX_DEATH);
    }
    // game over
    else {
        // OLD pAudio->Play_Sound(pPackage_Manager->Get_Music_Reading_Path("game/lost_1.ogg"), RID_ALEX_DEATH);
    }

    // dying animation
    Set_Image_Set("small_dead_left");

    /* TODO: Instead of making a mini mainloop here, add a function
     * to cSceneManager that allows disabling of the Update() step
     * in the main loop, as this is all what this really wants. */

    float i;
    sf::Event input_event;
    sf::RenderWindow& stage = gp_app->Get_RenderWindow();
    for(i = 0.0f; i < 7.0f; i += gp_app->Get_SceneManager().Get_Speedfactor()) {
        cPreferences& preferences = gp_app->Get_Preferences();
        cScene& current_scene = gp_app->Get_SceneManager().Current_Scene();

        while (stage.pollEvent(input_event)) {
            if (input_event.type == sf::Event::KeyPressed) {
                if (input_event.key.code == sf::Keyboard::Escape) {
                    goto animation_end;
                }
                else if (input_event.key.code == preferences.m_key_screenshot) {
                    // OLD pVideo->Save_Screenshot();
                }
            }
            // OLD joystick stuff
            // OLD else if (input_event.type == SDL_JOYBUTTONDOWN) {
            // OLD     if (input_event.jbutton.button == pPreferences->m_joy_button_exit) {
            // OLD         goto animation_end;
            // OLD     }
            // OLD }
        }

        // move up
        move(0.0f, -13.0f);
        // draw
        current_scene.Draw(stage);
        // OLD pFramerate->Update()
    }

    // very small delay until falling animation
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 300000000; // 0.3 seconds
    nanosleep(&t, NULL);

    // OLD pFramerate->Reset();
    // OLD m_walk_count = 0.0f;
    // OLD 
    // OLD for (i = 0.0f; m_col_rect.m_y < pActive_Camera->m_y + game_res_h; i++) {
    // OLD     while (SDL_PollEvent(&input_event)) {
    // OLD         if (input_event.type == SDL_KEYDOWN) {
    // OLD             if (input_event.key.keysym.sym == SDLK_ESCAPE) {
    // OLD                 goto animation_end;
    // OLD             }
    // OLD             else if (input_event.key.keysym.sym == pPreferences->m_key_screenshot) {
    // OLD                 pVideo->Save_Screenshot();
    // OLD             }
    // OLD         }
    // OLD         else if (input_event.type == SDL_JOYBUTTONDOWN) {
    // OLD             if (input_event.jbutton.button == pPreferences->m_joy_button_exit) {
    // OLD                 goto animation_end;
    // OLD             }
    // OLD         }
    // OLD     }
    // OLD 
    // OLD     m_walk_count += pFramerate->m_speed_factor * 0.75f;
    // OLD 
    // OLD     if (m_walk_count > 4.0f) {
    // OLD         m_walk_count = 0.0f;
    // OLD     }
    // OLD 
    // OLD     // move down
    // OLD     Move(0.0f, 14.0f);
    // OLD 
    // OLD     if (m_walk_count > 2.0f) {
    // OLD         Set_Image_Num(ALEX_IMG_DEAD);
    // OLD     }
    // OLD     else {
    // OLD         Set_Image_Num(ALEX_IMG_DEAD + 1);
    // OLD     }
    // OLD 
    // OLD     // draw
    // OLD     Draw_Game();
    // OLD     // render
    // OLD     pVideo->Render();
    // OLD     pFramerate->Update();
    // OLD }

animation_end:

    // game over
    if (m_lives < 0) {
        //gp_app->Get_SceneManager().Push_Scene(new cGameOverScene());
        std::cerr << "DEBUG: Lives < 0!" << std::endl;
    }

    // clear
    while (stage.pollEvent(input_event)) { /* Just pop all events */ }
    // OLD // OLD pFramerate->Reset();

    // game over
    if (m_lives < 0) {
    // OLD     Game_Action = GA_ENTER_MENU;
    // OLD     // reset saved data
    // OLD     Game_Action_Data_Middle.add("reset_save", "1");
    // OLD     Game_Action_Data_Middle.add("load_menu", int_to_string(MENU_MAIN));

        std::cerr << "DEBUG: GAME OVER activated." << std::endl;
    }
    // custom level
    // OLD else if (Game_Mode_Type == MODE_TYPE_LEVEL_CUSTOM) {
    // OLD     Game_Action = GA_ENTER_MENU;
    // OLD     Game_Action_Data_Middle.add("load_menu", int_to_string(MENU_START));
    // OLD     Game_Action_Data_Middle.add("menu_start_current_level", path_to_utf8(Trim_Filename(pActive_Level->m_level_filename, 0, 0)));
    // OLD     // reset saved data
    // OLD     Game_Action_Data_Middle.add("reset_save", "1");
    // OLD }
    // back to overworld
    else {
        Set_Type(ALEX_SMALL, 0, 0);
        // OLD Game_Action = GA_ENTER_WORLD;
        std::cerr << "DEBUG: Normal non-gameover death of Alex activated." << std::endl;
    }

    // OLD // fade out
    // OLD Game_Action_Data_Start.add("music_fadeout", "1500");
    // OLD Game_Action_Data_Start.add("screen_fadeout", CEGUI::PropertyHelper::intToString(EFFECT_OUT_BLACK));
    // OLD Game_Action_Data_Start.add("screen_fadeout_speed", "3");
    // OLD // delay unload level
    // OLD Game_Action_Data_Middle.add("unload_levels", "1");
    // OLD Game_Action_Data_End.add("screen_fadein", CEGUI::PropertyHelper::intToString(EFFECT_IN_BLACK));
}

void cLevel_Player::Ball_Clear(void) const
{
    // OLD // destroy all fireballs from the player
    // OLD for (cSprite_List::iterator itr = m_sprite_manager->objects.begin(); itr != m_sprite_manager->objects.end(); ++itr) {
    // OLD     cSprite* obj = (*itr);
    // OLD 
    // OLD     if (obj->m_type == TYPE_BALL) {
    // OLD         cBall* ball = static_cast<cBall*>(obj);
    // OLD 
    // OLD         // if from player
    // OLD         if (ball->m_origin_type == TYPE_PLAYER) {
    // OLD             obj->Destroy();
    // OLD         }
    // OLD     }
    // OLD }
}

void cLevel_Player::Set_Type(SpriteType item_type, bool animation /* = 1 */, bool sound /* = 1 */, bool temp_power /* = 0 */)
{
    if (item_type == TYPE_PLAYER) {
        Set_Type(ALEX_SMALL, animation, sound, temp_power);
    }
    else if (item_type == TYPE_MUSHROOM_DEFAULT) {
        Set_Type(ALEX_BIG, animation, sound, temp_power);
    }
    else if (item_type == TYPE_MUSHROOM_BLUE) {
        Set_Type(ALEX_ICE, animation, sound, temp_power);
    }
    else if (item_type == TYPE_MUSHROOM_GHOST) {
        Set_Type(ALEX_GHOST, animation, sound, temp_power);
    }
    else if (item_type == TYPE_FIREPLANT) {
        Set_Type(ALEX_FIRE, animation, sound, temp_power);
    }
}

void cLevel_Player::Set_Type(Alex_type new_type, bool animation /* = 1 */, bool sound /* = 1 */, bool temp_power /* = 0 */)
{
    // already set
    if (m_alex_type == new_type) {
        return;
    }

    // play sound
    if (sound) {
        if (new_type == ALEX_BIG) {
            // OLD pAudio->Play_Sound("item/mushroom.ogg", RID_MUSHROOM);
        }
        else if (new_type == ALEX_FIRE) {
            // OLD pAudio->Play_Sound("item/fireplant.ogg", RID_FIREPLANT);
        }
        else if (new_type == ALEX_ICE) {
            // OLD pAudio->Play_Sound("item/mushroom_blue.wav", RID_MUSHROOM_BLUE);
        }
        else if (new_type == ALEX_CAPE) {
            // OLD pAudio->Play_Sound("item/feather.ogg", RID_FEATHER);
        }
        else if (new_type == ALEX_GHOST) {
            // OLD pAudio->Play_Sound("item/mushroom_ghost.ogg", RID_MUSHROOM_GHOST);
        }
    }

    if (!temp_power) {
        // was flying
        if (m_alex_type == ALEX_CAPE) {
            Stop_Flying(0);
        }
    }

    // remember old type
    Alex_type old_type = m_alex_type;

    // draw animation and set new type
    if (animation) {
        Draw_Animation(new_type);

        if (temp_power) {
            m_alex_type = old_type;
            m_alex_type_temp_power = new_type;
            // Draw_Animation ends with the new type images
            Load_Images();
        }

    }
    // only set type
    else {
        if (temp_power) {
            m_alex_type_temp_power = new_type;
        }
        else {
            m_alex_type = new_type;
            Load_Images();
        }
    }

    // nothing more needed for setting the temp power
    if (temp_power) {
        return;
    }

    // to ghost
    if (m_alex_type == ALEX_GHOST) {
        m_ghost_time = gp_app->Get_SceneManager().Get_Speedfactor() * 10;
        m_alex_type_temp_power = old_type;
    }
    // was ghost
    else if (old_type == ALEX_GHOST) {
        m_ghost_time = 0;
        m_ghost_time_mod = 0;
        m_alex_type_temp_power = ALEX_DEAD;

        // check if we were standing on ghost ground and now need to fall
        // because the ground is gone when we materialize again.
        if (mp_ground_object) {
            cBaseBox* box = dynamic_cast<cBaseBox*>(mp_ground_object);

            if (box) {
                // ghost box
                if (box->m_box_invisible == BOX_GHOST) {
                    Reset_On_Ground();
                }
            }
        }
    }
}

/**
 * Freezes level gameplay and draws the animation for making Alex
 * look different (e.g. the animation that plays when he collects
 * an ice powerup).
 */
void cLevel_Player::Draw_Animation(Alex_type new_mtype)
{
    // already set or invalid
    if (new_mtype == m_alex_type || m_alex_type == ALEX_DEAD || new_mtype == ALEX_DEAD) {
        return;
    }
/*
    Alex_type alex_type_old = m_alex_type;
    bool parachute_old = m_parachute;

    float posx_old = m_pos_x;
    float posy_old = m_pos_y;

    // Change_Size needs new state size
    m_alex_type = alex_type_old;
    Parachute(parachute_old);
    Load_Images();

    // correct position for bigger alex
    if (alex_type_old == ALEX_SMALL && (new_mtype == ALEX_BIG || new_mtype == ALEX_FIRE || new_mtype == ALEX_ICE || new_mtype == ALEX_CAPE || new_mtype == ALEX_GHOST)) {
        Change_Size(-5.0f, -12.0f);
    }
    // correct position for small alex
    else if ((alex_type_old == ALEX_BIG || alex_type_old == ALEX_FIRE || alex_type_old == ALEX_ICE || new_mtype == ALEX_CAPE || alex_type_old == ALEX_GHOST) && new_mtype == ALEX_SMALL) {
        Change_Size(5.0f, 12.0f);
    }

    float posx_new = m_pos_x;
    float posy_new = m_pos_y;

    // draw animation
    for (unsigned int i = 0; i < 7; i++) {
        // set to current type
        if (i % 2) {
            m_alex_type = alex_type_old;
            Parachute(parachute_old);
            Load_Images();

            Set_Pos(posx_old, posy_old);
        }
        // set to new type
        else {
            m_alex_type = new_mtype;
            if (new_mtype != ALEX_CAPE) {
                Parachute(0);
            }
            Load_Images();

            // always set the ghost type to draw the ghost rect until it ends
            if (i < 6 && alex_type_old == ALEX_GHOST) {
                m_alex_type = alex_type_old;
            }

            Set_Pos(posx_new, posy_new);
        }

        // draw
        Draw_Game();
        pVideo->Render();

        // frame delay
        SDL_Delay(120);
    }

    pFramerate->Reset();
    */
}
