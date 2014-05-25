# Copyright ©2014 Lyude
#
# This file is free software: you may copy it, redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 2 of this License or (at your option) any
# later version.
#
# This file is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6)
project(libinput-tablet-debugger C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBINPUT REQUIRED libinput)
find_package(Curses REQUIRED)
find_package(UDev REQUIRED)

add_definitions("-std=gnu99")

add_subdirectory(src)
