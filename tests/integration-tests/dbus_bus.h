/*
 * Copyright © 2015 Canonical Ltd.
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
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef USC_TEST_DBUS_BUS_H_
#define USC_TEST_DBUS_BUS_H_

#include <string>
#include <sys/types.h>

namespace usc
{
namespace test
{

class DBusBus
{
public:
    DBusBus();
    ~DBusBus();

    std::string address();

private:
    std::string address_;
    pid_t pid;
};

}
}

#endif
