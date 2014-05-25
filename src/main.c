/*
 * Copyright ©2014 Lyude
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
 * Copyright © 2014 Red Hat, Inc.
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
#include <string.h>

#include <ncurses.h>
#include <libinput.h>
#include <libudev.h>

static struct udev *udev;
static struct libinput *li;
static const char *seat = "seat0";

static struct libinput_device ** tablets = NULL;
static int tablet_count = 0;

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


static void
handle_new_device(struct libinput_event *ev) {
	struct libinput_device *dev = libinput_event_get_device(ev);
	int * user_data = malloc(sizeof(int));

	if (!libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_STYLUS))
		return;

	libinput_device_ref(dev);

	*user_data = tablet_count; // store the position of the tablet with libinput

	tablets = realloc(tablets,
			  sizeof(struct libinput_device*) * ++tablet_count);
	tablets[tablet_count - 1] = dev;

	libinput_device_set_user_data(dev, user_data);
}

static void
handle_removed_device(struct libinput_event *ev) {
	struct libinput_device *dev = libinput_event_get_device(ev);
	size_t tablets_len;
	int * pos;
	
	if (!libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_STYLUS))
		return;

	tablets_len = sizeof(struct libinput_device *) * tablet_count;
	pos = libinput_device_get_user_data(dev);

	memmove(&tablets[*pos], &tablets[*pos] + 1,
		tablets_len - sizeof(struct libinput_device *) *
		              (tablet_count - *pos));

	tablets = realloc(tablets, tablets_len - sizeof(struct libinput_device*));
	tablet_count--;
	libinput_device_unref(dev);
}

static void
refresh_display() {
	/* Maybe someday I'll implement a damage system. Maybe. */
	clear();
	printw("Tablets Connected: %d\n", tablet_count);

	refresh();
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

		refresh_display();
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

	/* Handle already-pending device added events */
	if (handle_tablet_events())
		fprintf(stderr, "Expected device added events on startup but got none. "
				"Maybe you don't have the right permissions?\n");

	refresh_display();

	while (poll(fds, 3, -1) > -1) {
		if (fds[1].revents)
			break;
		else if (fds[2].revents) {
			switch(getch()) {
			case 'q':
				goto exit_mainloop;
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

	mainloop();

	endwin();
	udev_unref(udev);

	return 0;	
}
