/***************************************************************************
 * color.h
 *
 * Copyright © 2005 - 2011 Florian Richter
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

#ifndef TSC_COLOR_HPP
#define TSC_COLOR_HPP

namespace TSC {

    typedef sf::Color Color;

    // Generic Colors
    static const sf::Color blue = sf::Color(static_cast<uint8_t>(150), 200, 225);
    static const sf::Color darkblue = sf::Color(static_cast<uint8_t>(0), 0, 128);
    static const sf::Color lightblue = sf::Color(static_cast<uint8_t>(41), 167, 255);
    static const sf::Color black = sf::Color(static_cast<uint8_t>(0), 0, 0);
    static const sf::Color blackalpha128 = sf::Color(static_cast<uint8_t>(0), 0, 0, 128);
    static const sf::Color blackalpha192 = sf::Color(static_cast<uint8_t>(0), 0, 0, 192);
    static const sf::Color white = sf::Color(static_cast<uint8_t>(255), 255, 255);
    static const sf::Color whitealpha128 = sf::Color(static_cast<uint8_t>(255), 255, 255, 128);
    static const sf::Color grey = sf::Color(static_cast<uint8_t>(128), 128, 128);
    static const sf::Color lightgrey = sf::Color(static_cast<uint8_t>(64), 64, 64);
    static const sf::Color lightgreyalpha64 = sf::Color(static_cast<uint8_t>(64), 64, 64, 64);
    static const sf::Color green = sf::Color(static_cast<uint8_t>(0), 230, 0);
    static const sf::Color lightgreen = sf::Color(static_cast<uint8_t>(20), 253, 20);
    static const sf::Color lightgreenalpha64 = sf::Color(static_cast<uint8_t>(30), 230, 30, 64);
    static const sf::Color yellow = sf::Color(static_cast<uint8_t>(255), 245, 10);
    static const sf::Color greenyellow = sf::Color(static_cast<uint8_t>(154), 205, 50);
    static const sf::Color darkgreen = sf::Color(static_cast<uint8_t>(1), 119, 34);
    static const sf::Color red = sf::Color(static_cast<uint8_t>(250), 0, 0);
    static const sf::Color lightred = sf::Color(static_cast<uint8_t>(255), 40, 20);
    static const sf::Color lila = sf::Color(static_cast<uint8_t>(200), 0, 255);
    static const sf::Color orange = sf::Color(static_cast<uint8_t>(248), 191, 38);
    static const sf::Color lightorange = sf::Color(static_cast<uint8_t>(255), 220, 100);

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif
