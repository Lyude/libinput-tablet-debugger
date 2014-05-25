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

include_directories(${LIBINPUT_INCLUDE_DIRS} ${CURSES_INCLUDE_DIR}
	${UDEV_INCLUDE_DIRS})
link_directories(${LIBINPUT_LIBRARY_DIRS} ${UDEV_LIBRARY_DIRS})
add_definitions(${LIBINPUT_CFLAG_OTHER})

add_executable(li-tablet-debug main.c)

target_link_libraries(li-tablet-debug ${LIBINPUT_LIBRARIES}
	${CURSES_LIBRARIES} ${UDEV_LIBRARIES} ${PANEL_LIBRARIES})
