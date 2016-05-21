/* $XConsortium: uwm.c,v 1.23 88/11/16 09:41:32 jim Exp $ */
#include <X11/copyright.h>

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/*
 * MODIFICATION HISTORY
 *
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab. Convert to X11.
 */

#ifndef lint
static char *sccsid = "%W%	%G%";
#endif

char *ProgramName;

#include "uwm.h"
#include <ctype.h>
#include <X11/Xproto.h>

#ifdef PROFIL
#include <signal.h>
/*
 * Dummy handler for profiling.
 */
ptrap()
{
    exit(0);
}
#endif

#define gray_width 16
#define gray_height 16
static char gray_bits[] = {
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa
};



Bool NeedRootInput=FALSE;
Bool ChkMline();
char *sfilename;
extern FILE *yyin;

/*
 * Main program.
 */
main(argc, argv, environ)
int argc;
char **argv;
char **environ;
{
    int hi;			/* Button event high detail. */
    int lo;			/* Button event low detail. */
    int x, y;                   /* Mouse X and Y coordinates. */
    int root_x, root_y;         /* Mouse root X and Y coordinates. */
    int cur_x, cur_y;		/* Current mouse X and Y coordinates. */
    int down_x, down_y;		/* mouse X and Y at ButtonPress. */
    int str_width;              /* Width in pixels of output string. */
    int pop_width, pop_height;  /* Pop up window width and height. */
    int context;		/* Root, window, or icon context. */
    int ptrmask;		/* for QueryPointer */
    Bool func_stat;		/* If true, function swallowed a ButtonUp. */
    Bool delta_done;		/* If true, then delta functions are done. */
    Bool local = FALSE;		/* If true, then do not use system defaults. */
    register Binding *bptr;	/* Pointer to Bindings list. */
    char *root_name;		/* Root window name. */
    char *display = NULL;	/* Display name pointer. */
    char message[128];		/* Error message buffer. */
    char *rc_file;		/* Pointer to $HOME/.uwmrc. */
    Window event_win;           /* Event window. */
    Window sub_win;		/* Subwindow for XUpdateMouse calls. */
    Window root;		/* Root window for QueryPointer. */
    XWindowAttributes event_info;	/* Event window info. */
    XEvent button_event; 	 /* Button input event. */
    GC gc;			/* graphics context for gray background */
    XImage grayimage;		/* for gray background */
    XGCValues xgc;		/* to create font GCs */
    char *malloc();
    Bool fallbackMFont = False,	/* using default GC font for menus, */
         fallbackPFont = False,	/* popups, */
         fallbackIFont = False;	/* icons */
    XSetWindowAttributes attr;	/* for setting save under flag */
    unsigned long whitepix, blackpix, borderpix, popborderpix;

    ProgramName = argv[0];

#ifdef PROFIL
    signal(SIGTERM, ptrap);
#endif

    /*
     * Set up internal defaults.
     */
    SetVarDefaults();

    /* 
     * Parse the command line arguments.
     */
    Argv = argv;
    Environ = environ;
    argc--, argv++;
    while (argc) {
        if (**argv == '-') {
            if (strcmp(*argv, "-display") == 0 || strcmp(*argv, "-d") == 0) {
		argc--, argv++;
		if (argc <= 0)
		    Usage ();
		display = *argv;
	    } else if (strcmp(*argv, "-f") == 0) {
                argc--, argv++;
                if ((argc == 0) || (Startup_File[0] != '\0'))
                    Usage();
                strncpy(Startup_File, *argv, NAME_LEN);
            } else if (strcmp(*argv, "-b") == 0)
                local = TRUE;
            else 
		Usage();
        } else
	  Usage ();
	argc--, argv++;
    }

    /*
     * Initialize the default bindings.
     */
    if (!local)
        InitBindings();

    /* 
     * Open the display.
     */
    if ((dpy = XOpenDisplay(display)) == NULL)
        Error("Unable to open display");
    scr = DefaultScreen(dpy); 

    SetEnvironment ();

    LoadXDefaults ();

    /*
     * Set XErrorFunction to be non-terminating.
     */
    XSetErrorHandler(XError);


    /*
     * Force child processes to disinherit the TCP file descriptor.
     * This helps shell commands forked and exec'ed from menus
     * to work properly.
     */
    if ((status = fcntl(ConnectionNumber(dpy), F_SETFD, 1)) == -1) {
        perror("uwm: child cannot disinherit TCP fd");
        Error("TCP file descriptor problems");
    }

    /*
     * Read in and parse $HOME/.uwmrc, if it exists.
     */
    sfilename = rc_file = malloc(NAME_LEN);
    sprintf(rc_file, "%s/.uwmrc", getenv("HOME"));
    if ((yyin = fopen(rc_file, "r")) != NULL) {
        Lineno = 1;
        yyparse();
        fclose(yyin);
        if (Startup_File_Error)
            Error("Bad .uwmrc file...aborting");
    }

    /* 
     * Read in and parse the startup file from the command line, if
     * specified.
     */
    if (Startup_File[0] != '\0') {
        sfilename = Startup_File;
        if ((yyin = fopen(Startup_File, "r")) == NULL) {
    	sprintf(message, "Cannot open startup file '%s'", Startup_File);
            Error(message);
        }
        Lineno = 1;
        yyparse();
        fclose(yyin);
        if (Startup_File_Error)
            Error("Bad startup file...aborting");
    }

    /*
     * Verify the menu bindings.
     */
    VerifyMenuBindings();
    if (Startup_File_Error)
        Error("Bad startup file...aborting");

    /*
     * Verify the menu variables.
     */
    VerifyMenuVariables();

    /*
     * If the root window has not been named, name it.
     */
    status = XFetchName(dpy, RootWindow(dpy, scr), &root_name);
    if (root_name == NULL) 
    	XStoreName(dpy, RootWindow(dpy, scr), " X Root Window ");
    else free(root_name);


    ScreenHeight = DisplayHeight(dpy, scr);
    ScreenWidth = DisplayWidth(dpy, scr);

    /*
     * set the default colors
     */

    {
	Bool whiteset = False, blackset = False, borderset = False;
	Colormap cmap = DefaultColormap (dpy, scr);
	XColor cdef;

	/*
	 * the model is foreground = black,  background = white
	 */

	if (DisplayCells (dpy, scr) > 2) {
	    if (PBackgroundName[0]) {
		if (XParseColor (dpy, cmap, PBackgroundName, &cdef) &&
		    XAllocColor (dpy, cmap, &cdef)) {
		    whitepix = cdef.pixel;
		    whiteset = True;
		}
	    }
	    if (PForegroundName[0]) {
		if (XParseColor (dpy, cmap, PForegroundName, &cdef) &&
		    XAllocColor (dpy, cmap, &cdef)) {
		    blackpix = cdef.pixel;
		    blackset = True;
		}
	    }
	    if (PBorderColor[0]) {
		if (XParseColor (dpy, cmap, PBorderColor, &cdef) &&
		    XAllocColor (dpy, cmap, &cdef)) {
		    borderpix = cdef.pixel;
		    borderset = True;
		}
	    }
	}
	if (!whiteset) whitepix = WhitePixel (dpy, scr);
	if (!blackset) blackpix = BlackPixel (dpy, scr);
	if (!borderset) borderpix = blackpix;
    }


    /*
     * Create and store the icon background pixmap.
     */
    GrayPixmap = (Pixmap)XCreatePixmap(dpy, RootWindow(dpy, scr), 
    	gray_width, gray_height, DefaultDepth(dpy,scr));
    xgc.foreground = blackpix;
    xgc.background = whitepix;
    gc = XCreateGC(dpy, GrayPixmap, GCForeground+GCBackground, &xgc);
    grayimage.height = gray_width;
    grayimage.width = gray_height;
    grayimage.xoffset = 0;
    grayimage.format = XYBitmap;
    grayimage.data = (char *)gray_bits;
    grayimage.byte_order = LSBFirst;
    grayimage.bitmap_unit = 8;
    grayimage.bitmap_bit_order = LSBFirst;
    grayimage.bitmap_pad = 16;
    grayimage.bytes_per_line = 2;
    grayimage.depth = 1;
    XPutImage(dpy, GrayPixmap, gc, &grayimage, 0, 0,
	 0, 0, gray_width, gray_height);
    XFreeGC(dpy, gc);
    
    /*
     * Set up icon window, icon cursor and pop-up window color parameters.
     * The world needs to be consistent, so we choose a default of black on
     * white.  Note that this means that sizing windows will look "backwards"
     * from what they used to be.
     */
    if (Reverse) {
	PBackground = MBackground = blackpix;
	ITextBackground = MTextBackground = PTextBackground = blackpix;
	ITextForeground = MTextForeground = PTextForeground = whitepix;
    } else {
	PBackground = MBackground = whitepix;
	ITextBackground = MTextBackground = PTextBackground = whitepix;
	ITextForeground = MTextForeground = PTextForeground = blackpix;
    }
    IBackground = GrayPixmap;
    IBorder = MBorder = PBorder = borderpix;

    /*
     * Store all the cursors.
     */
    StoreCursors();

    /* 
     * grab the mouse buttons according to the map structure
     */
    Grab_Buttons();

    /*
     * Set initial focus to PointerRoot.
     */
    XSetInputFocus(dpy, PointerRoot, None, CurrentTime);

    /*
     * register an interest for colormap tracking
     */
    {
    Window	root_return, parent_return, *children_return;
    int		nchildren_return;
    initColormap ();

    /* 
     * watch for initial window mapping and window destruction
     */
    x_error_occurred = FALSE;
    (void) uwmExpressInterest (RootWindow (dpy, scr),
 			SubstructureNotifyMask|SubstructureRedirectMask|FocusChangeMask|
			(NeedRootInput ? EVENTMASK|OwnerGrabButtonMask : 0));
    XSync (dpy, 0);
    if (x_error_occurred && 
	last_error_event.request_code == X_ChangeWindowAttributes &&
	last_error_event.error_code == BadAccess) {
        Error("can't select for root window events; is another window manager running?");
    }

    /*
     * discover existing windows
     */

    if (XQueryTree (dpy, RootWindow(dpy, scr), &root_return,
		    &parent_return, &children_return, &nchildren_return)) {
	int	i;

	for (i = 0; i < nchildren_return; i++)
	    (void) getWindowInfo (children_return[i]);
    }
    }

    /*
     * Retrieve the information structure for the specifed fonts and
     * set the global font information pointers.
     */
    IFontInfo = XLoadQueryFont(dpy, IFontName);
    if (IFontInfo == NULL) {
        fprintf(stderr, "uwm: Unable to open icon font '%s', using server default.\n",
                IFontName);
	IFontInfo = XQueryFont(dpy, DefaultGC(dpy, scr)->gid);
	fallbackIFont = True;
    }
    PFontInfo = XLoadQueryFont(dpy, PFontName);
    if (PFontInfo == NULL) {
        fprintf(stderr, "uwm: Unable to open resize font '%s', using server default.\n",
                PFontName);
	if (fallbackIFont)
	    PFontInfo = IFontInfo;
	else
	    PFontInfo = XQueryFont(dpy, DefaultGC(dpy, scr)->gid);
	fallbackPFont = True;
    }
    MFontInfo = XLoadQueryFont(dpy, MFontName);
    if (MFontInfo == NULL) {
        fprintf(stderr, "uwm: Unable to open menu font '%s', using server default.\n",
                MFontName);
	if (fallbackIFont || fallbackPFont)
	    MFontInfo = fallbackPFont ? PFontInfo : IFontInfo;
	else
	    MFontInfo = XQueryFont(dpy, DefaultGC(dpy, scr)->gid);
	fallbackMFont = True;
    }

    /*
     * Calculate size of the resize pop-up window.
     */
    str_width = XTextWidth(PFontInfo, PText, strlen(PText));
    pop_width = str_width + (PPadding << 1);
    PWidth = pop_width + (PBorderWidth << 1);
    pop_height = PFontInfo->ascent + PFontInfo->descent + (PPadding << 1);
    PHeight = pop_height + (PBorderWidth << 1);

    /*
     * Create the pop-up window.  Create it at (0, 0) for now.  We will
     * move it where we want later.
     */
    Pop = XCreateSimpleWindow(dpy, RootWindow(dpy, scr),
                        0, 0,
                        pop_width, pop_height,
                        PBorderWidth,
                        PBorder, PBackground);
    if (Pop == FAILURE) Error("Can't create pop-up dimension display window.");
    attr.save_under = True;
    XChangeWindowAttributes (dpy, Pop, CWSaveUnder, &attr);

    /*
     * Create the menus for later use.
     */
    CreateMenus();

    /*
     * Create graphics context.
     */
    xgc.foreground = ITextForeground;
    xgc.background = ITextBackground;
    xgc.font = IFontInfo->fid;
    xgc.graphics_exposures = FALSE;
    IconGC = XCreateGC(dpy, 
    	RootWindow(dpy, scr),
	GCForeground+GCBackground+GCGraphicsExposures
		       +(fallbackIFont ? 0 : GCFont), &xgc);
    xgc.foreground = MTextForeground;
    xgc.background = MTextBackground;
    xgc.font = MFontInfo->fid;
    MenuGC = XCreateGC(dpy, 
    	RootWindow(dpy, scr),
	GCForeground+GCBackground+(fallbackMFont ? 0 : GCFont), &xgc);
    xgc.function = GXinvert;
    xgc.plane_mask = MTextForeground ^ MTextBackground;
    MenuInvGC = XCreateGC(dpy, 
    	RootWindow(dpy, scr), GCForeground+GCFunction+GCPlaneMask, &xgc);
    xgc.foreground = PTextForeground;
    xgc.background = PTextBackground;
    xgc.font = PFontInfo->fid;
    PopGC = XCreateGC(dpy, 
    	RootWindow(dpy, scr),
	GCForeground+GCBackground+(fallbackPFont ? 0 : GCFont), &xgc);
    xgc.line_width = DRAW_WIDTH;
    xgc.foreground = DRAW_VALUE;
    xgc.function = DRAW_FUNC;
    xgc.subwindow_mode = IncludeInferiors;
    DrawGC = XCreateGC(dpy, RootWindow(dpy, scr), 
      GCLineWidth+GCForeground+GCFunction+GCSubwindowMode, &xgc);


    /*
     * Tell the user we're alive and well.
     */
    XBell(dpy, VOLUME_PERCENTAGE(Volume));

    /* 
     * Main command loop.
     */
    while (TRUE) {

        delta_done = func_stat = FALSE;

        /*
         * Get the next mouse button event.  Spin our wheels until
         * a ButtonPressed event is returned.
         * Note that mouse events within an icon window are handled
         * in the "GetButton" function or by the icon's owner if
         * it is not uwm.
         */
        while (TRUE) {
            if (!GetButton(&button_event)) continue;
            if (button_event.type == ButtonPress) break;
        }

	/* save mouse coords in case we want them later for a delta action */
	down_x = ((XButtonPressedEvent *)&button_event)->x;
	down_y = ((XButtonPressedEvent *)&button_event)->y;
        /*
         * Okay, determine the event window and mouse coordinates.
         */
        status = XTranslateCoordinates(dpy, 
				    RootWindow(dpy, scr), RootWindow(dpy, scr),
                                    ((XButtonPressedEvent *)&button_event)->x, 
				    ((XButtonPressedEvent *)&button_event)->y,
                                    &x, &y,
                                    &event_win);

        if (status == FAILURE) continue;

        /*
         * Determine the event window and context.
         */
        if (event_win == 0) {
                event_win = RootWindow(dpy, scr);
                context = ROOT;
        } else {
            if (IsIcon(event_win, 0, 0, FALSE, NULL))
                context = ICON;
            else context = WINDOW;
        }

        /*
         * Get the button event detail.
         */
        lo = ((XButtonPressedEvent *)&button_event)->button;
        hi = ((XButtonPressedEvent *)&button_event)->state;

        /*
         * Determine which function was selected and invoke it.
         */
        for(bptr = Blist; bptr; bptr = bptr->next) {

            if ((bptr->button != lo) ||
                (((int)bptr->mask & ModMask) != hi))
                continue;

            if (bptr->context != context)
                continue;

            if (!(bptr->mask & ButtonDown))
                continue;

            /*
             * Found a match! Invoke the function.
             */
            if ((*bptr->func)(event_win,
                              (int)bptr->mask & ModMask,
                              bptr->button,
                              x, y,
                              bptr->menu)) {
                func_stat = TRUE;
                break;
            }
        }

        /*
         * If the function ate the ButtonUp event, then restart the loop.
         */
        if (func_stat) continue;

        while(TRUE) {
            /*
             * Wait for the next button event.
             */
            if (XPending(dpy) && GetButton(&button_event)) {
    
                /*
                 * If it's not a release of the same button that was pressed,
                 * don't do the function bound to 'ButtonUp'.
                 */
                if (button_event.type != ButtonRelease)
                    break;
                if (lo != ((XButtonReleasedEvent *)&button_event)->button)
                    break;
                if ((hi|ButtonMask(lo)) != 
		     ((XButtonReleasedEvent *)&button_event)->state)
                    break;
        
                /*
                 * Okay, determine the event window and mouse coordinates.
                 */
                status = XTranslateCoordinates(dpy, 
				    RootWindow(dpy, scr), RootWindow(dpy, scr),
                                    ((XButtonReleasedEvent *)&button_event)->x,
				    ((XButtonReleasedEvent *)&button_event)->y,
                                    &x, &y,
                                    &event_win);

                if (status == FAILURE) break;

                if (event_win == 0) {
                        event_win = RootWindow(dpy, scr);
                        context = ROOT;
                } else {
                    if (IsIcon(event_win, 0, 0, FALSE, NULL))
                        context = ICON;
                    else context = WINDOW;
                }
        
                /*
                 * Determine which function was selected and invoke it.
                 */
                for(bptr = Blist; bptr; bptr = bptr->next) {
        
                    if ((bptr->button != lo) ||
                        (((int)bptr->mask & ModMask) != hi))
                        continue;
        
                    if (bptr->context != context)
                        continue;
        
                    if (!(bptr->mask & ButtonUp))
                        continue;
        
                    /*
                     * Found a match! Invoke the function.
                     */
                    (*bptr->func)(event_win,
                                  (int)bptr->mask & ModMask,
                                  bptr->button,
                                  x, y,
                                  bptr->menu);
                }
                break;
            }
    
            XQueryPointer(dpy, RootWindow(dpy, scr), 
	    	&root, &event_win, &root_x, &root_y, &cur_x, &cur_y, &ptrmask);
            if (!delta_done &&
                ((abs(cur_x - x) > Delta) || (abs(cur_y - y) > Delta))) {
                /*
                 * Delta functions are done once (and only once.)
                 */
                delta_done = TRUE;

                /*
                 * Determine the new event window's coordinates.
		 * from the original ButtonPress event
                 */
                status = XTranslateCoordinates(dpy, 
			  RootWindow(dpy, scr), RootWindow(dpy, scr),
			  down_x, down_y, &x, &y, &event_win);
                if (status == FAILURE) break;

                /*
                 * Determine the event window and context.
                 */
                if (event_win == 0) {
                        event_win = RootWindow(dpy, scr);
                        context = ROOT;
                } else {
                    if (IsIcon(event_win, 0, 0, FALSE, NULL))
                        context = ICON;
                    else context = WINDOW;
                }
    
                /*
                 * Determine which function was selected and invoke it.
                 */
                for(bptr = Blist; bptr; bptr = bptr->next) {
        
                    if ((bptr->button != lo) ||
                        (((int)bptr->mask & ModMask) != hi))
                        continue;
        
                    if (bptr->context != context)
                        continue;
        
                    if (!(bptr->mask & DeltaMotion))
                        continue;
        
                    /*
                     * Found a match! Invoke the function.
                     */
                    if ((*bptr->func)(event_win,
                                      (int)bptr->mask & ModMask,
                                      bptr->button,
                                      x, y,
                                      bptr->menu)) {
                        func_stat = TRUE;
                        break;
                    }
                }
                /*
                 * If the function ate the ButtonUp event,
                 * then restart the loop.
                 */
                if (func_stat) break;
            }
        }
    }
}

/*
 * Initialize the default bindings.  First, write the character array
 * out to a temp file, then point the parser to it and read it in.
 * Afterwards, we unlink the temp file.
 */
InitBindings()
{
    char *mktemp();
    char *tempfile;		/* Temporary filename. */
    register FILE *fp;		/* Temporary file pointer. */
    register char **ptr;	/* Default bindings string array pointer. */

    /*
     * Create and write the temp file.
     */
    tempfile = malloc (strlen (TEMPFILE) + 1);
    if (!tempfile) {
	perror("uwm: cannot allocate name for temp file");
	exit (1);
    }
    strcpy (tempfile, TEMPFILE);
    sfilename = mktemp(tempfile);
    if ((fp = fopen(tempfile, "w")) == NULL) {
        perror("uwm: cannot create temp file");
        exit(1);
    }
    for (ptr = DefaultBindings; *ptr; ptr++) {
        fputs(*ptr, fp);
        fputc('\n', fp);
    }
    fclose(fp);

    /*
     * Read in the bindings from the temp file and parse them.
     */
    if ((yyin = fopen(tempfile, "r")) == NULL) {
        perror("uwm: cannot open temp file");
        exit(1);
    }
    Lineno = 1;
    yyparse();
    fclose(yyin);
    unlink(tempfile);
    if (Startup_File_Error)
        Error("Bad default bindings...aborting");

    /*
     * Parse the system startup file, if one exists.
     */
    if ((yyin = fopen(SYSFILE, "r")) != NULL) {
        sfilename = SYSFILE;
        Lineno = 1;
        yyparse();
        fclose(yyin);
        if (Startup_File_Error)
            Error("Bad system startup file...aborting");
    }
}

/*
 * Verify menu bindings by checking that a menu that is mapped actually
 * exists.  Stash a pointer in the binding to the relevant menu info data
 * structure.
 * Check nested menu consistency.
 */
VerifyMenuBindings()
{
    Binding *bptr;
    MenuLink *mptr;

    for(bptr = Blist; bptr; bptr = bptr->next) {
        if (bptr->func == Menu) {
            for(mptr = Menus; mptr; mptr = mptr->next) {
                if(!(strcmp(bptr->menuname, mptr->menu->name))) {
                    bptr->menu = mptr->menu;
                    break;
                }
            }
            if (mptr == NULL) {
                fprintf(stderr,
                        "uwm: non-existent menu reference: \"%s\"\n",
                        bptr->menuname);
                Startup_File_Error = TRUE;
            }
        }
    }
    CheckMenus();
}

/*
 * Verify that the menu variables have reasonable values
 */
VerifyMenuVariables()
{
    /*
     * If we pushrelative, we divide the window size by
     * the push variable. If it's zero, we die a sad death.
     * So lets use the default push value in this case.
     */
    if (!Pushval && Push) Pushval = DEF_PUSH;
}


/*
 * Check nested menu consistency by verifying that every menu line that
 * calls another menu references a menu that actually exists.
 */
CheckMenus()
{
    MenuLink *ptr;
    Bool errflag = FALSE;

    for(ptr = Menus; ptr; ptr = ptr->next) {
        if (ChkMline(ptr->menu))
            errflag = TRUE;
    }
    if (errflag)
        Error("Nested menu inconsistency");
}

Bool ChkMline(menu)
MenuInfo *menu;
{
    MenuLine *ptr;
    MenuLink *lptr;
    Bool errflag = FALSE;

    for(ptr = menu->line; ptr; ptr = ptr->next) {
        if (ptr->type == IsMenuFunction) {
            for(lptr = Menus; lptr; lptr = lptr->next) {
                if(!(strcmp(ptr->text, lptr->menu->name))) {
                    ptr->menu = lptr->menu;
                    break;
                }
            }
            if (lptr == NULL) {
                fprintf(stderr,
                        "uwm: non-existent menu reference: \"%s\"\n",
                        ptr->text);
                errflag = TRUE;
            }
        }
    }
    return(errflag);
}

/*
 * Grab the mouse buttons according to the bindings list.
 */
Grab_Buttons()
{
    Binding *bptr;

    for(bptr = Blist; bptr; bptr = bptr->next)
        if ((bptr->context & (WINDOW | ICON | ROOT)) == ROOT) {

	    /* don't grab buttons if you don't have to - allow application
	    access to buttons unless context includes window or icon */

	    NeedRootInput = TRUE;
	}
	else { 
	    /* context includes a window, so must grab */
	    Grab(bptr->mask);
	}
}

/*
 * Grab a mouse button according to the given mask.
 */
Grab(mask)
unsigned int mask;
{
    unsigned int m = LeftMask | MiddleMask | RightMask;

    switch (mask & m) {
    case LeftMask:
        XGrabButton(dpy, LeftButton,  mask & ModMask,
		RootWindow(dpy, scr), TRUE, EVENTMASK,
		GrabModeAsync, GrabModeAsync, None, LeftButtonCursor);
        break;

    case MiddleMask:
        XGrabButton(dpy, MiddleButton,  mask & ModMask,
		RootWindow(dpy, scr), TRUE, EVENTMASK,
		GrabModeAsync, GrabModeAsync, None, MiddleButtonCursor);
        break;

    case RightMask:
        XGrabButton(dpy, RightButton,  mask & ModMask,
		RootWindow(dpy, scr), TRUE, EVENTMASK,
		GrabModeAsync, GrabModeAsync, None, RightButtonCursor);
        break;
    }
}

/*
 * Restore cursor to normal state.
 */
ResetCursor(button)
int button;
{

    switch (button) {
    case LeftButton:
        XChangeActivePointerGrab(
		dpy, EVENTMASK, LeftButtonCursor, CurrentTime);
        break;

    case MiddleButton:
        XChangeActivePointerGrab(
		dpy, EVENTMASK, MiddleButtonCursor, CurrentTime);
        break;

    case RightButton:
        XChangeActivePointerGrab(
		dpy, EVENTMASK, RightButtonCursor, CurrentTime);
        break;
    }
}

/*
 * error routine for .uwmrc parser
 */
yyerror(s)
char*s;
{
    fprintf(stderr, "uwm: %s: %d: %s\n", sfilename, Lineno, s);
    Startup_File_Error = TRUE;
}

/*
 * Print usage message and quit.
 */
Usage()
{
    fprintf (stderr,
	     "usage:  %s [-display host:dpy] [-b] [-f filename]\n\n",
	     Argv[0]);
    fputs("The -b option bypasses system and default bindings\n", stderr);
    fputs("The -f option specifies an additional startup file\n", stderr);
    exit(1);
}

/*
 * error handler for X I/O errors
 */
XIOError(dsp)
Display *dsp;
{
    perror("uwm");
    exit(3);
}

SetVarDefaults()
{
    strcpy(IFontName, DEF_FONT);
    strcpy(PFontName, DEF_FONT);
    strcpy(MFontName, DEF_FONT);
    Delta = DEF_DELTA;
    IBorderWidth = DEF_ICON_BORDER_WIDTH;
    HIconPad = DEF_ICON_PADDING;
    VIconPad = DEF_ICON_PADDING;
    PBorderWidth = DEF_POP_BORDER_WIDTH;
    PPadding = DEF_POP_PADDING;
    MBorderWidth = DEF_MENU_BORDER_WIDTH;
    HMenuPad = DEF_MENU_PADDING;
    VMenuPad = DEF_MENU_PADDING;
    Volume = DEF_VOLUME;
    Pushval = DEF_PUSH;
    FocusSetByUser = FALSE;
    PBackgroundName[0] = '\0';		/* so that default is computed */
    PForegroundName[0] = '\0';		/* so that default is computed */
    PBorderColor[0] = '\0';		/* so that default is computed */
}

static char *get_default (d, prog, name)
    Display *d;
    char *prog;
    char *name;
{
    char *option;
    char buf[256];

    option = XGetDefault (d, prog, name);
    if (!option) {
	strcpy (buf, name);
	if (isascii(buf[0]) && islower(buf[0])) buf[0] = toupper(buf[0]);
	option = XGetDefault (d, prog, buf);
    }

    return option;
}

static int parse_boolean (s)
    char *s;
{
    char *cp;
    static struct _boolpair {
	char *name;
	int value;
    } table[] = {
	{ "on", 1 }, { "yes", 1 }, { "t", 1 }, { "true", 1 },
	{ "off", 0 }, { "no", 0 }, { "f", 0 }, { "false", 0 },
	{ NULL, -1 }};
    struct _boolpair *bpp;

    if (!s) return -1;

    for (cp = s; *cp; cp++) {
	if (isascii(*cp) && isupper(*cp)) *cp = tolower(*cp);
    }

    for (bpp = table; bpp->name; bpp++) {
	if (strcmp (s, bpp->name) == 0) return bpp->value;
    }

    return -1;
}

LoadXDefaults ()
{
    Keyword *k;
    char *option;

    if (!dpy) return;

    for (k = KeywordTable; k->name; k++) {
	switch (k->type) {
	  case IsBoolTrue:
	    option = get_default (dpy, ProgramName, k->name);
	    if (option) {
		switch (parse_boolean (option)) {
		  case 0:
		    *(k->bptr) = FALSE;
		    break;
		  case 1:
		    *(k->bptr) = TRUE;
		    break;
		}
	    }
	    continue;
	  case IsBoolFalse:
	    option = get_default (dpy, ProgramName, k->name);
	    if (option) {
		switch (parse_boolean (option)) {
		  case 0:
		    *(k->bptr) = TRUE;
		    break;
		  case 1:
		    *(k->bptr) = FALSE;
		    break;
		}
	    }
	    continue;
	  case IsString:
	    option = get_default (dpy, ProgramName, k->name);
	    if (option) {
		if (k->sptr) 
		  strcpy (k->sptr, option);
		else
		  fprintf (stderr,
			   "%s:  no string variable to set for \"%s\"\n",
			   ProgramName, k->name);
	    }
	    continue;
	  case IsNumeric:
	    option = get_default (dpy, ProgramName, k->name);
	    if (option) {
		if (k->nptr) 
		  *(k->nptr) = atoi (option);
		else
		  fprintf (stderr,
			   "%s:  no numeric variable to set for \"%s\"\n",
			   ProgramName, k->name);
	    }
	    continue;
	}
    }

    return;
}

extern char **environ;
char **newenviron;

/*
 * make a new copy of environment that has room for DISPLAY
 */

SetEnvironment ()
{
    int nenvvars;
    char **newPtr, **oldPtr;
    static char displaybuf[256];

    /* count number of environment variables */
    for (oldPtr = environ; *oldPtr; oldPtr++) ;

    nenvvars = (oldPtr - environ);
    newenviron = (char **) malloc ((nenvvars + 2) * sizeof(char **));
    if (!newenviron) {
	fprintf (stderr,
		 "%s:  unable to allocate %d pointers for environment\n",
		 ProgramName, nenvvars + 2);
	exit (1);
    }

    /* put DISPLAY=displayname as first element */
    strcpy (displaybuf, "DISPLAY=");
    strcpy (displaybuf + 8, DisplayString (dpy));
    newPtr = newenviron;
    *newPtr++ = displaybuf;

    /* copy pointers to other variables */
    for (oldPtr = environ; *oldPtr; oldPtr++) {
	if (strncmp (*oldPtr, "DISPLAY=", 8) != 0) {
	    *newPtr++ = *oldPtr;
	}
    }
    *newPtr = NULL;
    return;
}

