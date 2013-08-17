/*
 * Copyright (C) 2013 Simon Busch <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <Settings.h>

#include "settingsadapter.h"

namespace luna
{

SettingsAdapter::SettingsAdapter()
{
}

bool SettingsAdapter::tabletUi() const
{
    return Settings::LunaSettings()->tabletUi;
}

qreal SettingsAdapter::dpi() const
{
    return Settings::LunaSettings()->dpi;
}

qreal SettingsAdapter::compatDpi() const
{
    return Settings::LunaSettings()->compatDpi;
}

int SettingsAdapter::displayWidth() const
{
    return Settings::LunaSettings()->displayWidth;
}

int SettingsAdapter::displayHeight() const
{
    return Settings::LunaSettings()->displayHeight;
}

bool SettingsAdapter::displayFps() const
{
    return Settings::LunaSettings()->debug_piranhaDisplayFps;
}

bool SettingsAdapter::showReticle() const
{
    return Settings::LunaSettings()->showReticle;
}

} // namespace luna
