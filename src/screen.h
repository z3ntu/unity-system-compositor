/*
 * Copyright © 2014-2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USC_SCREEN_H_
#define USC_SCREEN_H_

#include <mir_toolkit/common.h>
#include <functional>

namespace usc
{

class Screen
{
public:
    virtual ~Screen() = default;

    virtual void turn_on() = 0;
    virtual void turn_off() = 0;

protected:
    Screen() = default;
    Screen(Screen const&) = delete;
    Screen& operator=(Screen const&) = delete;
};

}

#endif
