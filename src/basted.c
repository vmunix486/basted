#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 256
#define MAX_COLS  256

/* The file name storage so it can be stored */
char current_file[256] = {0};
/* The tingy that does the buffer... Buffet. I'm hungry. */
char lines[MAX_LINES][MAX_COLS];
int line_count = 1;

int cx = 0; /* This is the cursor's x position, or side to side in english */
int cy = 0; /* This is the cursor's y position, or up and down in english */

/* Stuff for dawing the text */
Display *dpy;
Window win;
GC gc;
XFontStruct *font;

int char_width;
int line_height;

/* Draw all 500 quadrillion pixels */
void redraw() {
	XClearWindow(dpy, win);

	/* Draw the alphanumeric characters */
	for (int i = 0; i < line_count; i++) {
		XDrawString(dpy, win, gc,
				10,
				20 + i * line_height,
				lines[i],
				strlen(lines[i]));
	}

	/* Draw the cursor that can move and is a square and can eat letters and can make new letters */
	int x = 10 + cx * char_width;
	int y = 20 + cy * line_height;

	XDrawLine(dpy, win, gc,	
			x, 
			y - font->ascent, 
			x, 
			y + font->descent);
}

/* Load the files so you can edit them */
void load_file(const char *filename) {
	/* FILE *f = fopen(filename, "r");
	if (!f) return; */

	/* Remember the file name, ALWAYS */
	strncpy(current_file, filename, sizeof(current_file) - 1);
	current_file[sizeof(current_file) - 1] = '\0';

	FILE *f = fopen(filename, "r");
	if (!f) {
		/* The file doesn't actuallya exist yet but thats a-okay */
		line_count = 1;
		lines[0][0] = '\0';
		cx = cy = 0;
		return;
	}

	line_count = 0;
	cx = cy = 0;

	char buffer[MAX_COLS];

	while (fgets(buffer, sizeof(buffer), f)) {
		buffer[strcspn(buffer, "\n")] = '\0'; /* remove newlines */

		strncpy(lines[line_count], buffer, MAX_COLS - 1);
		lines[line_count][MAX_COLS - 1] = '\0';

		line_count++;
		if (line_count >= MAX_LINES) break;
	}
	fclose(f);

	if (line_count == 0) {
		line_count = 1;
			lines[0][0] = '\0';
	}
}

/* The part that saves the files */
void save_file() {
	/* If there is no file name specified, then ask the user where they want to dump the file. */
	if (current_file[0] == '\0') {
		printf("No filename specified!\n");
		printf("File location:  ");
		fflush(stdout);

		if (!fgets(current_file, sizeof(current_file), stdin)) {
			printf("\nInput failed.\n");
			return;
		}

		/* remove newlines */
		current_file[strcspn(current_file, "\n")] = '\0';

		if (current_file[0] == '\0') {
			printf("No filename entered.\n");
			return;
		}
	}

	printf("Saving the file to: %s\n", current_file);

	FILE *f = fopen(current_file, "w");
	if (!f) {
		perror("fopen");
		return;
	}

	for (int i = 0; i < line_count; i++) {
		fprintf(f, "%s\n", lines[i]);
	}

	fclose(f);
}

/* ACTUAL PROGRAM HOLY <bleep> */
int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) {
			printf("basted - the BASic Text EDitor version 0.2.\n");
			printf("Usage: basted [file]\n");
			return 0;
		} else {
			load_file(argv[1]);
		}
	}
	/* Connect to the X server that is hopefully running and is hopefully XLibre because XLibre is fast and good and is the GOAT */
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "Could not find X server. This could be because of many reasons.\n\n If your X Server is not running, that could be a reason why. It's 2026, and you got swap. Your computer is not going to explode.\n\n The second reason is that you are on some weird VNC or RDP forwarding, or ssh -X just did not work.\n\n The last reason I can think of is that your computer is really <bleep>-ing cursed. If you are wondering, this was made for UNIX and UNIX-like operating systems. It will probably not work on DesqView/X or any of the Windows X Servers.\n\n");
		exit(1);
	}

	int screen = DefaultScreen(dpy);

	/* Actually create the window so you can see it and interact with it and use it and oh wait I just used a word with teh same meaning two times in a row. */
	win = XCreateSimpleWindow(
		dpy,
		RootWindow(dpy, screen),
		100, 100, 640, 480,
		1,
		BlackPixel(dpy, screen),
		WhitePixel(dpy, screen)
	);

	/* set the title of the window */

	/* static title bar name */
	/* XStoreName(dpy, win, "basted 0.2"); */

	/* dynamic title bar name */
	void update_title() {
		char title[512];

		if (current_file[0] != '\0') {
			snprintf(title, sizeof(title), "basted 0.2 - %s", current_file);
		} else {
			snprintf(title, sizeof(title), "basted 0.2 - [No Name]");
		}

		XStoreName(dpy, win, title);
	}

	/* Select the events, or also known as inputs, or also known as 
	 * k e y p r e s s */
	XSelectInput(dpy, win, ExposureMask | KeyPressMask);

	/* Show the window so you can see it, and the note before is actually wrong but it's 10PM EST and I'm tired and lazy so I don't feel like changing it */
	XMapWindow(dpy, win);

	/* Context the graphics so they have context and are graphiked */
	gc = DefaultGC(dpy, screen);

	/* Load the font to it's not
	 * 1. Nothing
	 * 2. All the missing character boxes
	 * 3. Compilation error */
	font = XLoadQueryFont(dpy, "fixed");
	XSetFont(dpy, gc, font->fid);

	char_width = font->max_bounds.width;
	line_height = font->ascent + font->descent;

	/* more file loading stuff */
	if (argc > 1) {
		load_file(argv[1]);
		update_title();
		printf("Loaded file: %s\n", current_file);
	}

	/* The event that is always looping, like that one picture of that snake eating itself or like walking on a mobius strip or whatever */
	XEvent ev;

	while (1) {
		XNextEvent(dpy, &ev);

		/* Redraw the window when it is exposed like the Internet Anarchist */
		if (ev.type == Expose) {
			redraw();
		}

		/* Look at the keyboard input so it can input the keyboard into the keyboard reader, and then charge your key account. If you do not have enough keys to buy the product, then your keyboard will decline. If you have sufficient keys, then you will be able to buy the product and have a good day. */
		if (ev.type == KeyPress) {
			char buf[32];
			KeySym keysym;

			int n = XLookupString(&ev.xkey, buf, sizeof(buf), &keysym, NULL);
			/* look for CTRL+S for saving */
			if ((ev.xkey.state & ControlMask) && keysym == XK_s) {
				save_file();
				update_title();
				printf("Saving the file to: %s\n", current_file);
			}

			/* Babysit the keys so they don't make awful things like 4Chan or Reddit or Fedora or Microslop Windows or GNOME */
			if (keysym == XK_Escape) {
				break;
			}
			else if (keysym == XK_BackSpace) {
				if (cx > 0) {
					cx--;
					lines[cy][cx] = '\0';
				}
			}
			else if (keysym == XK_Return) {
				if (cy + 1 < MAX_LINES) {
					cy++;
					cx = 0;
					line_count++;
				}
			}
			else if (keysym == XK_Left && cx > 0) {
				cx--;
			}
			else if (keysym == XK_Right && cx < strlen(lines[cy])) {
				cx++;
			}
			else if (keysym == XK_Down && cy < line_count - 1) {
				cy++;
			}
			else if (n > 0 && isprint((unsigned char)buf[0]) && cx < MAX_COLS -1) {
				lines[cy][cx++] = buf[0];
				lines[cy][cx] = '\0';
			}
			/* I'm sorry but all the else if's remind me of Yandere Simulator :skull: */
			
			/* Redraw after every keypress. 
			 * 
			 * NOTE: This sounds catastropically bad on old computers.*/
			redraw();
		}
	}

	/* Close the window so it is closed and goned */
	XCloseDisplay(dpy);
	return 0;
}
