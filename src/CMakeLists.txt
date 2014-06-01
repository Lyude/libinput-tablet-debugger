# Copyright ©2014 Lyude
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that the
# above copyright notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting documentation, and that
# the name of the copyright holders not be used in advertising or publicity
# pertaining to distribution of the software without specific, written prior
# permission.  The copyright holders make no representations about the
# suitability of this software for any purpose.  It is provided "as is" without
# express or implied warranty.
#
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
# SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

cmake_minimum_required(VERSION 2.6)

include_directories(${LIBINPUT_INCLUDE_DIRS} ${CURSES_INCLUDE_DIR}
	${UDEV_INCLUDE_DIRS})
link_directories(${LIBINPUT_LIBRARY_DIRS} ${UDEV_LIBRARY_DIRS})
add_definitions(${LIBINPUT_CFLAG_OTHER})

add_executable(li-tablet-debug main.c)

target_link_libraries(li-tablet-debug ${LIBINPUT_LIBRARIES}
	${CURSES_LIBRARIES} ${UDEV_LIBRARIES} ${PANEL_LIBRARIES})
