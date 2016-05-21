/* $XConsortium: uwm.h,v 1.15 88/10/23 12:08:18 jim Exp $ */
 
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
 * 001 -- R. Kittell, DEC Storage A/D May 19, 1986
 *  Added global vars for warp options.
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab, Port to X11
 */
 
#include <errno.h>
#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
 
#define MIN(x, y)	((x) <= (y) ? (x) : (y))
#define MAX(x, y)	((x) >= (y) ? (x) : (y))
#define VOLUME_PERCENTAGE(x)	((x)*14) 
#define ModMask 0xFF
#define ButtonMask(b)	(((b)==Button1) ? Button1Mask : \
			  (((b)==Button2) ? Button2Mask : Button3Mask))

#define FALSE	False
#define TRUE	True
 

#define DEF_DELTA		1
#ifndef DEF_FONT
#define DEF_FONT		"6x10"
#endif
#define DEF_FUNC		GXcopy
#define DEF_ICON_BORDER_WIDTH 	2
#define DEF_ICON_PADDING	4
#define DEF_POP_BORDER_WIDTH 	2
#define DEF_POP_PADDING		4
#define DEF_MENU_BORDER_WIDTH 	2
#define DEF_MENU_PADDING	4
#define DEF_VOLUME		4
#define DEF_PUSH		5
 
#define PTEXT_LENGTH		9
#define INIT_PTEXT		{'0', '0', '0', '0', 'x', '0', '0', '0', '0'}
#ifndef TEMPFILE
#define TEMPFILE		"/tmp/uwm.XXXXXX"
#endif
#ifndef SYSFILE
#define SYSFILE			"/usr/lib/X11/uwm/system.uwmrc"
#endif
 
#define CURSOR_WIDTH		16
#define CURSOR_HEIGHT		16
 
#define MAX_ZAP_VECTORS		8
#define MAX_BOX_VECTORS		16
 
#define DRAW_WIDTH		0 	/* use fastest hardware draw */
#define DRAW_VALUE		0xfd
#define DRAW_FUNC		GXxor
#define DRAW_PLANES		1
 
#define ROOT			0x1
#define WINDOW			0x2
#define ICON			0x4
 
#define FAILURE			0
#define NAME_LEN		256	/* Maximum length of filenames. */
#define EVENTMASK		(ButtonPressMask | ButtonReleaseMask)
 
#define DrawBox() XDrawSegments(dpy, RootWindow(dpy, scr),DrawGC,box,num_vectors)
#define DrawZap() XDrawSegments(dpy, RootWindow(dpy, scr),DrawGC,zap,num_vectors)
 
/*
 * Keyword table entry.
 */
typedef struct _keyword {
    char *name;
    int type;
    Bool *bptr;
    int *nptr;
    char *sptr;
    Bool (*fptr)();
} Keyword;
 
/*
 * Keyword table type entry.
 */
#define IsString	1
#define IsBoolTrue	2
#define IsBoolFalse	3
#define IsFunction	4
#define IsMenuMap	5
#define IsMenu		6
#define IsDownFunction	7
#define IsParser	8
#define IsNumeric	9
#define IsQuitFunction	10
 
/*
 * Button/key binding type.
 */
typedef struct _binding {
    struct _binding *next;
    int context;
    int mask;
    int button;
    Bool (*func)();
    char *menuname;
    struct _menuinfo *menu;
} Binding;
 
/*
 * Key expression type.
 */
typedef struct _keyexpr {
    char *name;
    int mask;
} KeyExpr;
 
/*
 * Context expression type.
 */
typedef struct _contexpr {
    char *name;
    int mask;
} ContExpr;
 
/*
 * Button modifier type.
 */
typedef struct _buttonmodifier {
    char *name;
    int mask;
} ButtonModifier;
 
/*
 * Button modifier mask definitions.
 * bits 13 and 14 unused in key masks, according to X.h
 * steal bit 15, since we don't use AnyModifier
 */

#define DeltaMotion	(1<<13)
#define ButtonUp	(1<<14)
#define ButtonDown	AnyModifier
#define ButtonMods	DeltaMotion+ButtonUp+ButtonDown

/* 
 * Button and mask redefinitions, for X11
 */
#define LeftMask 	Button1Mask
#define MiddleMask 	Button2Mask
#define RightMask 	Button3Mask
#define LeftButton	Button1
#define MiddleButton	Button2
#define RightButton	Button3

/*
 * WindowInfo data type.
 */

typedef struct _windowInfo {
    Window	window;
    Colormap	colormap;		/* private to colormap.c */
    Bool	in_window;		/* private to colormap.c */
    long	event_mask;		/* combined event mask */
    long	uwm_event_mask;		/* events uwm gets to see */
    Bool	been_mapped;		/* true if have been mapped */
    XSizeHints	stashedhints;		/* save a copy away */
} WindowInfo;
WindowInfo *getWindowInfo();


/*
 * MenuInfo data type.
 */
typedef struct _menuinfo {
    char *name;			/* Name of this menu. */
    Window w;			/* Menu window. */
    int iheight;		/* Height of an item. */
    int width;			/* Width of this menu. */
    int height;			/* Height of this menu. */
    Pixmap image;		/* Saved image of the menu. */
    char *foreground;		/* Name of foreground color. */
    char *background;		/* Name of background color. */
    char *fghighlight;		/* Name of hightlight foreground color. */
    char *bghighlight;		/* Name of highlight background color. */
    XColor fg;			/* Foreground color definition. */
    XColor bg;			/* Background color definition. */
    XColor hlfg;		/* Foreground highlight color definition. */
    XColor hlbg;		/* Background highlight color definition. */
    struct _menuline *line;	/* Linked list of menu items. */
} MenuInfo;
 
/*
 * Menu Line data type.
 */
typedef struct _menuline {
    struct _menuline *next;	/* Pointer to next line. */
    char *name;			/* Name of this line. */
    int type;			/* IsShellCommand, IsText, IsTextNL... */
    Window w;			/* Subwindow for this line. */
    char *text;			/* Text string to be acted upon. */
    Bool (*func)();		/* Window manager function to be invoked. */
    struct _menuinfo *menu;	/* Menu to be invoked. */
    char *foreground;		/* Name of foreground color. */
    char *background;		/* Name of background color. */
    XColor fg;			/* Foreground color definition. */
    XColor bg;			/* Background color definition. */
} MenuLine;
 
/*
 * MenuLine->type definitions.
 */
#define IsShellCommand		1
#define IsText			2
#define IsTextNL		3
#define IsUwmFunction		4
#define IsMenuFunction		5
#define IsImmFunction		6     /* Immediate (context-less) function. */
 
/*
 * Menu Link data type.  Used by the parser when creating a linked list
 * of menus.
 */
typedef struct _menulink {
    struct _menulink *next;	/* Pointer to next MenuLink. */
    struct _menuinfo *menu;	/* Pointer to the menu in this link. */
} MenuLink;

typedef int Pixel;
 
/*
 * External variable definitions.
 */
extern int errno;
extern Window Pop;		/* Pop-up dimension display window. */
extern Window Frozen;		/* Contains window id of "gridded" window. */
extern XFontStruct *IFontInfo;	/* Icon text font information. */
extern XFontStruct *PFontInfo;	/* Pop-up text font information. */
extern XFontStruct *MFontInfo;	/* Menu text font information. */
extern Pixmap GrayPixmap;	/* Gray pixmap. */
extern Pixel IBorder;		/* Icon window border pixmap. */
extern Pixmap IBackground;	/* Icon window background pixmap. */
extern Pixel PBorder;		/* Pop-Up Window border pixmap. */
extern Pixel PBackground;	/* Pop-up Window background pixmap. */
extern Pixel MBorder;		/* Menu Window border pixmap. */
extern Pixel MBackground;	/* Menu Window background pixmap. */
extern Cursor ArrowCrossCursor; /* Arrow cross cursor. */
extern Cursor TextCursor;	/* Text cursor used in icon windows. */
extern Cursor IconCursor;	/* Icon Cursor. */
extern Cursor MenuCursor;	/* Menu cursor. */
extern Cursor MenuMaskCursor;	/* Menu mask cursor. */
extern Cursor LeftButtonCursor;	/* Left button main cursor. */
extern Cursor MiddleButtonCursor;/* Middle button main cursor. */
extern Cursor RightButtonCursor;/* Right button main cursor. */
extern Cursor TargetCursor;	/* Target (select-a-window) cursor. */
extern int ScreenWidth;		/* Display screen width. */
extern int ScreenHeight;	/* Display screen height. */
extern int ITextForeground;	/* Icon window text forground color. */
extern int ITextBackground;	/* Icon window text background color. */
extern int IBorderWidth;	/* Icon window border width. */
extern int PTextForeground;	/* Pop-up window text forground color. */
extern int PTextBackground;	/* Pop-up window text background color. */
extern int PWidth;		/* Pop-up window width (including borders). */
extern int PHeight;		/* Pop-up window height (including borders). */
extern int PBorderWidth;	/* Pop-up window border width. */
extern int MTextForeground;	/* Menu window text forground color. */
extern int MTextBackground;	/* Menu window text background color. */
extern int MBorderWidth;	/* Menu window border width. */
extern int PPadding;		/* Pop-up window padding. */
extern int Delay;		/* Delay between clicks of double click. */
extern int Delta;		/* Mouse movement slop. */
extern int HIconPad;		/* Icon horizontal padding. */
extern int VIconPad;		/* Icon vertical padding. */
extern int HMenuPad;		/* Menu horizontal padding. */
extern int VMenuPad;		/* Menu vertical padding. */
extern int MaxColors;		/* Maximum number of colors to use. */
extern int Pushval;		/* Number of pixels to push window by. */
extern int Volume;		/* Audible alarm volume. */
extern int status;		/* Routine return status. */
extern int Maxfd;		/* Maximum file descriptors for select(2). */
extern MenuLink *Menus;		/* Linked list of menus. */
extern GC  IconGC;		/* graphics context for icon */
extern GC  PopGC;		/* graphics context for pop */
extern GC  MenuGC;		/* graphics context for menu */
extern GC  MenuInvGC;		/* graphics context for menu background */
extern GC  DrawGC;		/* graphics context for zap */
 
extern Bool Autoselect;		/* Warp mouse to default menu selection? */
extern Bool Freeze;		/* Freeze server during move/resize? */
extern Bool Grid;		/* Should the m/r box contain a 9 seg. grid. */
extern Bool NWindow;		/* Normalize windows? */
extern Bool NIcon;		/* Normalize icons? */
extern Bool Push;		/* Relative=TRUE, Absolute=FALSE. */
extern Bool ResizeRelative;	/* Relative=TRUE, Absolute=FALSE. */
extern Bool Reverse;		/* Reverse video? */
extern Bool WarpOnRaise;	/* Warp to upper right corner on raise. */
extern Bool WarpOnIconify;      /* Warp to icon center on iconify. */
extern Bool WarpOnDeIconify;    /* Warp to upper right corner on de-iconify. */
extern Bool Zap;		/* Should the the zap effect be used. */
extern Bool FocusSetByUser;     /* True if f.focus called */
 
extern char PText[];		/* Pop-up window dummy text. */
extern int PTextSize;		/* Pop-up window dummy text size. */
 
extern int Lineno;		/* Line count for parser. */
extern Bool Startup_File_Error;	/* Startup file error flag. */
extern char Startup_File[];	/* Startup file name. */
extern char IFontName[];	/* Icon font name. */
extern char PFontName[];	/* Pop-up font name. */
extern char MFontName[];	/* Menu font name. */
extern char PBackgroundName[];	/* name of color to use for popup */
extern char PForegroundName[];	/* name of color to use for popup */
extern char PBorderColor[];	/* name of color to use for popup */
extern char **Argv;		/* Pointer to command line parameters. */
extern char **Environ;		/* Pointer to environment. */
 
extern char *DefaultBindings[];	/* Default bindings string array. */
extern Keyword KeywordTable[];	/* Keyword lookup table. */
extern Binding *Blist;		/* Button/key bindings list. */
extern KeyExpr KeyExprTbl[];	/* Key expression table. */
extern ContExpr ContExprTbl[];	/* Context expression table. */
extern ButtonModifier ButtModTbl[];/* Button modifier table. */

extern int scr;
extern Display *dpy;		/* Display info pointer. */
 
#ifdef PROFIL
int ptrap();
#endif
 
/*
 * External routine typing.
 */
extern Bool Beep();
extern Bool CircleDown();
extern Bool CircleUp();
extern Bool Continue();
extern Bool Focus();
extern Bool GetButton();
extern Bool Iconify();
extern Bool Kill();
extern Bool Lower();
extern Bool Menu();
extern Bool Move();
extern Bool MoveOpaque();
extern Bool NewIconify();
extern Bool Pause();
extern Bool PushDown();
extern Bool PushLeft();
extern Bool PushRight();
extern Bool PushUp();
extern Bool Quit();
extern Bool Raise();
extern Bool Redraw();
extern Bool Refresh();
extern Bool ResetBindings();
extern Bool ResetMenus();
extern Bool ResetVariables();
extern Bool Resize();
extern Bool Restart();
extern int StoreCursors();
extern int StoreBox();
extern int StoreGridBox();
extern int StoreZap();
extern int Error();
extern int XError();
extern int CreateMenus();
 
extern char *stash();
extern char *GetIconName();
extern Bool x_error_occurred;
extern XErrorEvent last_error_event;

#define clear_x_errors() (x_error_occurred = FALSE)
