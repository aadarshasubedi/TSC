Guidelines for the SFML port
============================

When working on the SFML port, please adhere to the following
guidelines:

1. Do not add new features. Only port existing features from the old
   `devel` branch to the new `feature-sfml-port` branch.
2. Rewrite code only where necessary; try to stick as close to the old
   code as possible.
3. Before you start working, take some time and understand how the new
   scene/actor-based system works. Start with the `main()` function in
   `main.cpp` and follow the  flow of code until you get to the
   cLevel_Player::Update(). That should give you a thorough
   intrudocution to the system.
4. Follow the general coding guidelines as outlined in the
   `conventions` documentation page.
5. Collision rectangle size must be set via
   cActor::Set_Collision_Rect(). That method takes care of keeping
   debug rect, editor rect, and collision rect in sync. Do not
   mess with m_collision_rect and m_debug_colrect_shape directly
   for setting (when converting from old code, these direct settings
   need to be changed to a call to Set_Collision_Rect() thus).

Press CTRL+D to enable/disable drawing of the collision
rectangles. Note that you do not have to implement drawing of the
collision rectangles yourself when you implement an actor, this is
automatically done for you directly in the cActor class.

You’ll find I (Quintus) leave a lot of comments in the code that start
with “// OLD”. These are copies of code from the pre-SFML
implementation that cannot run yet because of missing functions and/or
other not yet ported things. The goal is to not have any such comments
anymore, but I need them as a visual marker for functions that are
only partly implemented yet. Otherwise I’d think I have fully ported
the function in question to SFML already.

Other notes
-----------

The collision system is slightly different. In comparison to original
TSC, the ARRAY_* types are gone and collision handling is now entirely
dependant on the massivity of an actor rather than on a hardly
predictable extra attribute. cActor::Handle_Collision() is not virtual
anymore, but has been degraded to a bare delegation method for the
other Handle_Collision_*() methods which you should override instead
(as it was already recommended with original TSC). Beware that the
more exotic massive types may not show up where you expect them; most
notably, climbable objects do not send any collisions and will not
show up in any collision handler thus, they’re just treated the same
way as passive/frontpassive objects. Halfmassive objects will trigger
Handle_Collision_Passive(). Things may change later.
