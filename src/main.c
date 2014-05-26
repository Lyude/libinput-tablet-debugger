/*
 * Copyright ©2013 Lyude
 *
 * This file is free software: you may copy it, redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of this License or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Some of this code has been adapted from event-debug.c from the libinput
 * project, to which the following copyrights and permissions apply:
 *
 * Copyright © 2013 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <termios.h>

#include <ncurses.h>
#include <panel.h>

#include <libinput.h>
#include <libudev.h>

#include "field_locations.h"

#define CURRENT_PANEL (

static struct udev *udev;
static struct libinput *li;
static const char *seat = "seat0";

static int tablet_count = 0;

struct tablet_panel {
	WINDOW * window;
	PANEL * panel;
	struct libinput_device * dev;
};

static struct tablet_panel placeholder_panel;

static int
open_restricted(const char *path, int flags, void *user_data)
{
	int fd=open(path, flags);
	return fd < 0 ? -errno : fd;
}

static void
close_restricted(int fd, void *user_data)
{
	close(fd);
}

const static struct libinput_interface interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted
};

static int
open_udev() {
	printf("Setting up udev...\n");

	udev = udev_new();
	if (!udev) {
		fprintf(stderr, "Failed to initialize udev\n");
		return 1;
	}

	li = libinput_udev_create_for_seat(&interface, NULL, udev, seat);
	if (!li) {
		fprintf(stderr, "Failed to initialize context from udev\n");
		return 1;
	}

	return 0;
}

/*static void*/
/*log_handler(enum libinput_log_priority priority,*/
	    /*void *user_data,*/
	    /*const char *format,*/
	    /*va_list args)*/
/*{*/
	/*[> do nothing for right now <]*/
/*}*/

static inline void
update_display() {
	update_panels();
	doupdate();
}

static void
paint_panel(struct tablet_panel * panel) {
	mvwprintw(panel->window, TABLET_SYSTEM_NAME_ROW,
		  TABLET_SYSTEM_NAME_COLUMN,
		  "System name: %s", libinput_device_get_sysname(panel->dev));
}

static struct tablet_panel *
new_tablet_panel(struct libinput_device * dev) {
	struct tablet_panel * panel;

	panel = malloc(sizeof(struct tablet_panel));
	panel->window = newwin(0, 0, 0, 0);
	panel->panel = new_panel(panel->window);
	panel->dev = dev;

	set_panel_userptr(panel->panel, panel);

	return panel;
}

static void
handle_new_device(struct libinput_event *ev) {
	struct libinput_device *dev = libinput_event_get_device(ev);
	struct tablet_panel * panel;

	if (!libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_STYLUS))
		return;

	libinput_device_ref(dev);

	panel = new_tablet_panel(dev);
	bottom_panel(panel->panel);
	paint_panel(panel);

	/* If this is our only tablet, get rid of the placeholder panel */
	if (++tablet_count == 1) {
		hide_panel(placeholder_panel.panel);
		update_display();
	}

	libinput_device_set_user_data(dev, panel);
}

static void
handle_removed_device(struct libinput_event *ev) {
	struct libinput_device *dev = libinput_event_get_device(ev);
	struct tablet_panel * panel;
	
	if (!libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_STYLUS))
		return;

	panel = libinput_device_get_user_data(dev);
	del_panel(panel->panel);
	delwin(panel->window);
	free(panel);

	if (--tablet_count == 0) {
		show_panel(placeholder_panel.panel);
		update_display();
	}

	libinput_device_unref(dev);
}

static int
handle_tablet_events() {
	struct libinput_event *ev;

	libinput_dispatch(li);
	while ((ev = libinput_get_event(li))) {
		switch (libinput_event_get_type(ev)) {
		case LIBINPUT_EVENT_NONE:
			abort();
		case LIBINPUT_EVENT_DEVICE_ADDED:
			handle_new_device(ev);
			break;
		case LIBINPUT_EVENT_DEVICE_REMOVED:
			handle_removed_device(ev);
			break;
		default:
			break;
		}
	}
	
	return 0;
}

static void
mainloop() {
	struct pollfd fds[3];
	sigset_t mask;

	fds[0].fd = libinput_get_fd(li);
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGWINCH);

	fds[1].fd = signalfd(-1, &mask, SFD_NONBLOCK);
	fds[1].events = POLLIN;
	fds[1].revents = 0;

	fds[2].fd = 0;
	fds[2].events = POLLIN;
	fds[2].revents = 0;

	if (fds[1].fd == -1 ||
	    sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
		fprintf(stderr, "Failed to set up signal handling (%s)\n",
				strerror(errno));
	}

	/* Create the placeholder panel */
	placeholder_panel.window = newwin(0, 0, 0, 0);
	placeholder_panel.panel = new_panel(placeholder_panel.window);
	wprintw(placeholder_panel.window,
		"There are no currently detected tablets.\n"
		"Make sure your tablet is plugged in, and that you have the "
		"right permissions.");
	show_panel(placeholder_panel.panel);
	update_display();

	/* Handle already-pending device added events */
	if (handle_tablet_events())
		fprintf(stderr, "Expected device added events on startup but got none. "
				"Maybe you don't have the right permissions?\n");

	while (poll(fds, 3, -1) > -1) {
		if (fds[1].revents) {
			int sig;
			
			/* Check what type of signal is waiting */
			sigwait(&mask, &sig);

			if (sig == SIGINT)
				break;

			endwin();
			refresh();

			/* Resize all of the panels and their windows */
			for (PANEL * p = panel_above(NULL);
			     p != NULL;
			     p = panel_below(p))
			{
				struct tablet_panel * panel =
					(void*)panel_userptr(p);

				paint_panel(panel);
			}
			update_display();
		}
		else if (fds[2].revents) {
			switch(getch()) {
			case 'q':
				goto exit_mainloop;
			case KEY_LEFT:
				top_panel(panel_below(NULL));
				show_panel(panel_above(NULL));

				update_display();
				break;
			case KEY_RIGHT:
				bottom_panel(panel_above(NULL));
				show_panel(panel_above(NULL));

				update_display();
				break;
			}
		}
		else
			handle_tablet_events();
	}
exit_mainloop:

	close(fds[1].fd);
}

int
main()
{
	printf("libinput tablet debugger\n"
	       "©2014 Lyude <thatslyude@gmail.com>\n"
	       "This is free software; see the source for copying conditions. "
	       "There is NO warranty; not even for MERCHANTABILITY or FITNESS "
	       "FOR A PARTICULAR PURPOSE.\n");

	if (open_udev() == 1) {
		fprintf(stderr, "Cannot continue.\n");
		return 1;
	}

	/* setup ncurses */
	initscr();
	raw();
	keypad(stdscr, true);
	noecho();
	clear();

	mainloop();

	endwin();
	udev_unref(udev);

	return 0;	
}
