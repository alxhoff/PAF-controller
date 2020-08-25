#ifndef __PAF_WIFI_H__
#define __PAF_WIFI_H__


/**
 * @file paf_wifi.h
 * @author Alex Hoffman
 * @date 25 August 2020
 * @brief Wifi functionality of board
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2020
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */

void paf_wifi_init_ap(void);
void paf_wifi_init_station(const char *ssid, const char *passwd);
void register_connect_wifi(void);

#endif // __PAF_WIFI_H__
