/*
 * Copyright © 2014 Canonical Ltd.
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

#include "powerd_mediator.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QDBusServiceWatcher>

#include <iostream>

/* Struct for reply to getBrightnessParams DBus call */
struct BrightnessParams
 {
    int dim_brightness;
    int min_brightness;
    int max_brightness;
    int default_brightness;
    bool auto_brightness_supported;
};
Q_DECLARE_METATYPE(BrightnessParams);

/* Not used but required by qDBusRegisterMetaType*/
QDBusArgument &operator<<(QDBusArgument &argument, const BrightnessParams &mystruct)
{
    argument.beginStructure();
    argument << mystruct.dim_brightness <<
                mystruct.min_brightness <<
                mystruct.max_brightness <<
                mystruct.default_brightness <<
                mystruct.auto_brightness_supported;
    argument.endStructure();
    return argument;
}

/* Needed to un-marshall powerd response */
const QDBusArgument &operator>>(const QDBusArgument &argument, BrightnessParams &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.dim_brightness >>
                mystruct.min_brightness >>
                mystruct.max_brightness >>
                mystruct.default_brightness >>
                mystruct.auto_brightness_supported;
    argument.endStructure();
    return argument;
}

PowerdMediator::PowerdMediator()
    : dim_brightness{10},
      normal_brightness{102},
      current_brightness{0},
      min_brightness_{10},
      max_brightness_{102},
      auto_brightness_supported_{false},
      auto_brightness_requested{false},
      backlight_state{BacklightState::normal},
      acquired_sys_state{false},
      powerd_interface{new QDBusInterface("com.canonical.powerd",
          "/com/canonical/powerd", "com.canonical.powerd", QDBusConnection::systemBus())},
      service_watcher{new QDBusServiceWatcher("com.canonical.powerd",
          QDBusConnection::systemBus(), QDBusServiceWatcher::WatchForRegistration)}
{
    qDBusRegisterMetaType<BrightnessParams>();

    if (!powerd_interface->isValid())
    {
        //Powerd interface is not available right now, wait for it to become available
        connect(service_watcher.get(),
                SIGNAL(serviceRegistered(QString const&)),
                this, SLOT(powerd_registered()));
    }
    else
    {
        init_brightness_params();
    }
}

PowerdMediator::~PowerdMediator() = default;

void PowerdMediator::set_dim_backlight()
{
    change_backlight_state(BacklightState::dim);
}

void PowerdMediator::set_normal_backlight()
{
    if (auto_brightness_requested)
        change_backlight_state(BacklightState::automatic);
    else
        change_backlight_state(BacklightState::normal);
}

void PowerdMediator::turn_off_backlight()
{
    change_backlight_state(BacklightState::off);
}

void PowerdMediator::change_backlight_values(int dim, int normal)
{
    if (normal >= min_brightness_ && normal <= max_brightness_)
        normal_brightness = normal;
    if (dim >= min_brightness_ && dim <= max_brightness_)
        dim_brightness = dim;
}

void PowerdMediator::enable_auto_brightness(bool enable)
{
    auto_brightness_requested = enable;
    if (auto_brightness_supported_ && enable)
        change_backlight_state(BacklightState::automatic);
    else
        change_backlight_state(BacklightState::normal);
}

bool PowerdMediator::auto_brightness_supported()
{
    return auto_brightness_supported_;
}

int PowerdMediator::min_brightness()
{
    return min_brightness_;
}

int PowerdMediator::max_brightness()
{
    return max_brightness_;
}

void PowerdMediator::powerd_registered()
{
    init_brightness_params();
}

void PowerdMediator::set_brightness(int brightness)
{
    normal_brightness = brightness;

    if (backlight_state != BacklightState::automatic)
        powerd_interface->call("setUserBrightness", brightness);
}

void PowerdMediator::change_backlight_state(BacklightState new_state)
{
    if (backlight_state == new_state)
        return;

    if (new_state == BacklightState::automatic)
    {
        powerd_interface->call("userAutobrightnessEnable", true);
        backlight_state = BacklightState::automatic;
        return;
    }

    switch (new_state)
    {
        case BacklightState::normal:
            current_brightness = normal_brightness;
            break;
        case BacklightState::off:
            current_brightness = 0;
            break;
        case BacklightState::dim:
            current_brightness = dim_brightness;
            break;
        default:
            std::cerr << "unknown backlight state" << std::endl;
            break;
    }

    powerd_interface->call("setUserBrightness", current_brightness);
    backlight_state = new_state;
}

void PowerdMediator::set_sys_state_for(MirPowerMode mode)
{
    if (mode == MirPowerMode::mir_power_mode_off)
        release_sys_state();
    else
        acquire_sys_state();
}

void PowerdMediator::release_sys_state()
{
    if (acquired_sys_state)
        powerd_interface->call("clearSysState", sys_state_cookie);
    acquired_sys_state = false;
}

void PowerdMediator::acquire_sys_state()
{
    if (!acquired_sys_state)
    {
        QDBusReply<QString> reply = powerd_interface->call("requestSysState", "com.canonical.Unity.Screen", 1);
        if (reply.isValid())
        {
            sys_state_cookie = reply.value();
            acquired_sys_state = true;
        }
    }
}

void PowerdMediator::init_brightness_params()
{
    QDBusReply<BrightnessParams> reply = powerd_interface->call("getBrightnessParams");
    if (reply.isValid())
    {
        auto const& params = reply.value();
        dim_brightness = params.dim_brightness;
        normal_brightness = params.default_brightness;
        min_brightness_ = params.min_brightness;
        max_brightness_ = params.max_brightness;
        auto_brightness_supported_ = params.auto_brightness_supported;
        std::cerr << "initialized brightness parameters" << std::endl;
    }
    else
    {
        std::cerr << "getBrightnessParams call failed with: ";
        std::cerr << reply.error().message().toStdString() << std::endl;
    }
}
