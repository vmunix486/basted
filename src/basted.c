#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 256
#define MAX_COLS  256

/* Default Window Size */
int win_width = 640;
int win_height = 480;

/* Scroll offset */
int scroll_y = 0;

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

int max_cols;

/* Draw all 500 quadrillion pixels */
void redraw(void) {
	XClearWindow(dpy, win);
	
	XWindowAttributes attr;
	XGetWindowAttributes(dpy, win, &attr);
	win_width = attr.width;
	win_height = attr.height;

	/* Part that calculates the max amount of characters per line */
	if (char_width <= 0) {
		max_cols = 80; /* Just as a fallback if it explodes */
	} else {
		max_cols = (win_width - 20) / char_width;
		/* if (max_cols < 10) max_cols = 10;  Temporary sanity check*/
		if (max_cols < 1) max_cols = 1;
	}

	/* Debugging shart */
	/* printf("win_width= %d char_width= %d\n max_cols= %d\n",
		win_width, char_width, (win_width - 20) / char_width); */


	int y = 20 - scroll_y;

	/* cursor stuff */
	int cursor_px = 10;
	int cursor_py = 20;
	int found_cursor = 0;

	/* Draw the alphanumeric characters */
	for (int i = 0; i < line_count; i++) {
		char *line = lines[i];
		int len = strlen(line);

		int start = 0;

		while (start < len) {
			if (y > win_height - line_height) break;

			int end = start + max_cols;

			if (end >= len) {
				int segment_len = len - start;
				/* cursor stuff */
				if (!found_cursor && i == cy) {
					if (cx >= start && cx <= len) {
						int rel = cx - start;
						cursor_px = 10 + rel * char_width;
						cursor_py = y;
						found_cursor = 1;
					}
				}

				XDrawString(dpy, win, gc, 10, y,
						line + start,
						len - start);
				break;
			}

			int wrap = (end > len) ? end : len - 1;

			/* Find the last space typed */
			while (wrap > start && line[wrap] != ' ') {
				wrap--;
			}
			
			/* If no space found, then do a hard wrap */
			if (wrap == start) {
			    wrap = end;
			}
			
			/* cursor stuff */
			if (!found_cursor && i == cy) {
				if (cx >= start && cx <= len) {
					int rel = cx - start;
					cursor_px = 10 + rel * char_width;
					cursor_py = y;
					found_cursor = 1;
				}
			}
			
			/* Draw segment */
			XDrawString(dpy, win, gc, 10, y,
					line + start,
					wrap - start);
			/* move start ofrwatd */
			if (wrap < len && line[wrap] == ' ')
				start = wrap + 1;
			else
				start = wrap;

			y += line_height;
		}

		y += line_height;
	}
	/* Draw the cursor */
	XDrawLine(dpy, win, gc,
			cursor_px,
			cursor_py - font->ascent,
			cursor_px,
			cursor_py + font->descent);

}

/* Load the files so you can edit them */
void load_file(const char *filename) {
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
		int j = 0;
		for (int i = 0; buffer[i] && j < MAX_COLS - 1; i++) {
			if (buffer[i] == '\t') {
				int spaces = 5;
				for (int k = 0; k < spaces && j < MAX_COLS - 1; k++) {
					lines[line_count][j++] = ' ';
				}
			} else {
				lines[line_count][j++] = buffer[i];
		}
	}

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
void save_file(void) {
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
			printf("basted - the BASic Text EDitor version 0.3.\n");
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
	void update_title(void) {
		char title[512];

		if (current_file[0] != '\0') {
			snprintf(title, sizeof(title), "basted 0.5 - %s", current_file);
		} else {
			snprintf(title, sizeof(title), "basted 0.5");
		}

		XStoreName(dpy, win, title);
	}

	/* Select the events, or also known as inputs, or also known as 
	 * k e y p r e s s */
	XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask | ButtonPressMask);

	/* Show the window so you can see it, and the note before is actually wrong but it's 10PM EST and I'm tired and lazy so I don't feel like changing it */
	XMapWindow(dpy, win);
	
	XWindowAttributes attr;
	XGetWindowAttributes(dpy, win, &attr);

	win_width = attr.width;
	win_height = attr.height;

	/* Context the graphics so they have context and are graphiked */
	gc = DefaultGC(dpy, screen);

	/* Load the font to it's not
	 * 1. Nothing
	 * 2. All the missing character boxes
	 * 3. Compilation error */
	font = XLoadQueryFont(dpy, "fixed");
	if (!font) {
		fprintf(stderr, "Failed to load font. Please install the fonts. I don't even know why I'm typing this right now because you should have fonts installed, and if you don't, then you can't read this anyways.\n");
		exit(1);
	}
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
			win_width = ev.xconfigure.width;
			win_height = ev.xconfigure.height;

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
				if (ev.xkey.state & ControlMask) {
					while (cx > 0 && lines[cy][cx - 1] == ' ')
						cx--;
					while (cx > 0 && lines[cy][cx - 1] != ' ')
						cx--;
					lines[cy][cx] = '\0';
				}
				else {
					if (cx > 0) {
						int len = strlen(lines[cy]);
						memmove(&lines[cy][cx - 1], &lines[cy][cx], len - cx + 1);
						cx--;
					}	
			}
			}
			else if (keysym == XK_Delete) {

				int len = strlen(lines[cy]);

				if (cx < len) {
					memmove(&lines[cy][cx],
							&lines[cy][cx + 1],
							len - cx);
				}
			}

			else if (keysym == XK_Tab) {
				int spaces = 5;
				for (int i = 0; i < spaces && cx < MAX_COLS - 1; i++) {
					lines[cy][cx++] = ' ';
				}

				lines[cy][cx] = '\0';
			}

			else if (keysym == XK_Return) {
				if (cy + 1 < MAX_LINES) {
					cy++;
					cx = 0;
					line_count++;
				}
			}
			else if (keysym == XK_Left) {
				
				if (ev.xkey.state & ControlMask) {
					while (cx > 0 && lines[cy][cx - 1] == ' ')
						cx--;

					while (cx > 0 && lines[cy][cx - 1] != ' ')
						cx--;
				} else {
					if (cx > 0) cx--;
				}
			}
			else if (keysym == XK_Right) {
				
				int len = strlen(lines[cy]);

				if (ev.xkey.state & ControlMask) {
					while (cx < len && lines[cy][cx] == ' ')
						cx++;

					while (cx < len && lines[cy][cx] != ' ')
						cx++;
				} else {
					if (cx < len) cx++;
				}
			}
			
			else if (keysym == XK_Down && cy < line_count - 1) {
				cy++;
			}
			else if (keysym == XK_Up && cy < line_count + 1) {
				cy --;
			}
			else if (keysym == XK_BackSpace) {
				if (cx > 0) {
					cx--;
					lines[cy][cx] = '\0';
				}
			}
			else if (keysym == XK_Home) {
				cx = 0;
			}
			else if (keysym == XK_End) {
				cx = strlen(lines[cy]);
			}	
			
			/* scrolling keys */
			if (keysym == XK_Page_Down) {
				scroll_y += line_height * 5;
			}
			else if (keysym == XK_Page_Up) {
				scroll_y -= line_height * 5;
				if (scroll_y < 0) scroll_y = 0;
			}
			else if (n > 0 && isprint((unsigned char)buf[0]) && cx < MAX_COLS -1) {
				int len = strlen(lines[cy]);
				if (len < MAX_COLS - 1) {
					/* waddle the text to the right, to the right to the right to the right, not the left, not the left, not the left... */
					memmove(&lines[cy][cx + 1], &lines[cy][cx], len - cx + 1);
					/* insert the characters like its fortnite (I don't play fortnite) */
					lines[cy][cx] = buf[0];
					cx++;
				}

				/* more scrolling stuff */
				if (scroll_y < 0) scroll_y = 0;
			}
		
			/* I'm sorry but all the else if's remind me of Yandere Simulator :skull: */	
			/* Redraw after every keypress. 
			 * 
			 * NOTE: This sounds catastropically bad on old computers.
			 * Update 4.6.26: It is. I can see it waiting to refresh on my Pentium III 1Ghz*/
			redraw();
		}
	}

	/* Close the window so it is closed and goned */
	XCloseDisplay(dpy);
	return 0;
}
