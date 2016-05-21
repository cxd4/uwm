/* $XConsortium: globals.c,v 1.12 88/09/22 10:53:29 jim Exp $ */
 
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
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group 
 *  Western Software Lab. April 17, 1987
 *  Convert to X11
 */
 
#ifndef lint
static char *sccsid = "%W%	%G%";
#endif
/*
 * globals.c - window manager global data
 *
 */
 
#include "uwm.h"
 
Window Pop;			/* Pop up dimension display window. */
Window Frozen;			/* Contains window id of "gridded" window. */
XFontStruct *IFontInfo;		/* Icon text font information. */
XFontStruct *PFontInfo;		/* Pop-up text font information. */
XFontStruct *MFontInfo;		/* Menu text font information. */
Pixmap GrayPixmap;		/* Gray pixmap. */
Pixel IBorder;			/* Icon window border pixmap. */
Pixmap IBackground;		/* Icon window background pixmap. */
Pixel PBorder;			/* Pop-Up Window border pixmap. */
Pixel PBackground;		/* Pop-up Window background pixmap. */
Pixel MBorder;			/* Menu Window border pixmap. */
Pixel MBackground;		/* Menu Window background pixmap. */
Cursor ArrowCrossCursor;	/* Arrow cross cursor. */
Cursor TextCursor;		/* Text cursor used in icon windows. */
Cursor IconCursor;		/* Icon Cursor. */
Cursor MenuCursor;		/* Menu cursor. */
Cursor MenuMaskCursor;		/* Menu mask cursor. */
Cursor LeftButtonCursor;	/* Left button main cursor. */
Cursor MiddleButtonCursor;	/* Middle button main cursor. */
Cursor RightButtonCursor;	/* Right button main cursor. */
Cursor TargetCursor;		/* Target (select-a-window) cursor. */
int ScreenWidth;		/* Display screen width. */
int ScreenHeight;		/* Display screen height. */
int ITextForeground;		/* Icon window text forground color. */
int ITextBackground;		/* Icon window text background color. */
int IBorderWidth;		/* Icon window border width. */
int PTextForeground;		/* Pop-up window text forground color. */
int PTextBackground;		/* Pop-up window text background color. */
int MTextForeground;		/* Menu window text forground color. */
int MTextBackground;		/* Menu window text background color. */
int PWidth;			/* Pop-up window width. */
int PHeight;			/* Pop-up window height. */
int PBorderWidth;		/* Pop-up window border width. */
int PPadding;			/* Pop-up window padding. */
int MBorderWidth;		/* Menu window border width. */
int MPadding;			/* Menu window padding. */
int Delay;			/* Delay between clicks of a double click. */
int Delta;			/* Mouse movement slop. */
int HIconPad;			/* Icon horizontal padding. */
int VIconPad;			/* Icon vertical padding. */
int HMenuPad;			/* Menu horizontal padding. */
int VMenuPad;			/* Menu vertical padding. */
int MaxColors;			/* Maximum number of colors to use. */
int Pushval = 5;		/* Number of pixels to push window by. */
int Volume;			/* Audible alarm volume. */
int status;			/* Routine return status. */
int Maxfd;			/* Maximum file descriptors for select(2). */
MenuLink *Menus;		/* Linked list of menus. */
GC  IconGC;             	/* graphics context for icon */
GC  PopGC;  	           	/* graphics context for pop */
GC  MenuGC;             	/* graphics context for menu */
GC  MenuInvGC;             	/* graphics context for menu background */
GC  DrawGC;             	/* graphics context for drawing */
Binding *Blist;			/* Button/key binding list. */
 
Bool Autoselect;		/* Warp mouse to default menu selection? */
Bool Freeze;			/* Freeze server during move/resize? */
Bool Grid;			/* Should the m/r box contain a 9 seg. grid. */
Bool NWindow;           	 /* Normalize windows? */
Bool NIcon;             	 /* Normalize icons? */
Bool Push;			/* Relative=TRUE, Absolute=FALSE. */
Bool ResizeRelative;		/* resizes should be relative to window edge */
Bool Reverse;			/* Reverse video? */
Bool Zap;			/* Should the the zap effect be used. */
Bool WarpOnRaise;               /* Warp to upper right corner on raise. */
Bool WarpOnIconify;             /* Warp to icon center on iconify. */
Bool WarpOnDeIconify;           /* Warp to upper right corner on de-iconify. */
Bool FocusSetByUser;            /* True if f.focus called */
 
char PText[PTEXT_LENGTH] = INIT_PTEXT;	/* Pop-up window dummy text. */
int PTextSize = PTEXT_LENGTH;	/* Pop-up window dummy text size. */
 
int Lineno = 1;			/* Line count for parser. */
Bool Startup_File_Error = FALSE;/* Startup file error flag. */
char Startup_File[NAME_LEN] = "";/* Startup file name. */
char IFontName[NAME_LEN];	/* Icon font name. */
char PFontName[NAME_LEN];	/* Pop-up font name. */
char MFontName[NAME_LEN];	/* Menu font name. */
char PBackgroundName[NAME_LEN];	/* name of color to use for popup */
char PForegroundName[NAME_LEN];	/* name of color to use for popup */
char PBorderColor[NAME_LEN];	/* name of color to use for popup */
char **Argv;			/* Pointer to command line parameters. */
char **Environ;			/* Pointer to environment. */

int scr;
Display *dpy;
 
/*
 * Keyword lookup table for parser.
 */
Keyword KeywordTable[] = {
    { "autoselect",	IsBoolTrue,	&Autoselect,0,0,0 },
    { "background",	IsString,	0,0,PBackgroundName, 0},
    { "bordercolor",	IsString,	0,0,PBorderColor, 0},
    { "borderwidth",	IsNumeric,	0,&PBorderWidth,0,0},
    { "delay",		IsNumeric,	0,&Delay,0,0 },
    { "delta",		IsNumeric,	0,&Delta,0,0 },
    { "foreground",	IsString,	0,0,PForegroundName, 0},
    { "freeze",		IsBoolTrue,	&Freeze,0,0,0 },
    { "iconfont",	IsString,	0,0,IFontName,0 },
    { "f.beep",		IsQuitFunction,	0,0,0,Beep },
    { "f.circledown",	IsQuitFunction,	0,0,0,CircleDown },
    { "f.circleup",	IsQuitFunction,	0,0,0,CircleUp },
    { "f.continue",	IsQuitFunction,	0,0,0,Continue },
    { "f.exit",		IsQuitFunction,	0,0,0,Quit },
    { "f.focus",	IsFunction,	0,0,0,Focus },
    { "f.iconify",	IsFunction,	0,0,0,Iconify },
    { "f.kill",		IsFunction,	0,0,0,Kill },
    { "f.lower",	IsFunction,	0,0,0,Lower },
    { "f.menu",		IsMenuMap,	0,0,0,Menu },
    { "f.move",		IsDownFunction,	0,0,0,Move },
    { "f.moveopaque",	IsDownFunction,	0,0,0,MoveOpaque },
    { "f.newiconify",	IsDownFunction,	0,0,0,NewIconify },
    { "f.pause",	IsQuitFunction,	0,0,0,Pause },
    { "f.pushdown",	IsFunction,	0,0,0,PushDown },
    { "f.pushleft",	IsFunction,	0,0,0,PushLeft },
    { "f.pushright",	IsFunction,	0,0,0,PushRight },
    { "f.pushup",	IsFunction,	0,0,0,PushUp },
    { "f.raise",	IsFunction,	0,0,0,Raise },
    { "f.redraw",	IsFunction,	0,0,0,Redraw },
    { "f.refresh",	IsQuitFunction,	0,0,0,Refresh },
    { "f.resize",	IsDownFunction,	0,0,0,Resize },
    { "f.restart",	IsQuitFunction,	0,0,0,Restart },
    { "grid",		IsBoolTrue,	&Grid,0,0,0 },
    { "hiconpad",	IsNumeric,	0,&HIconPad,0,0 },
    { "hmenupad",	IsNumeric,	0,&HMenuPad,0,0 },
    { "iborderwidth",	IsNumeric,	0,&IBorderWidth,0,0 },
    { "maxcolors",	IsNumeric,	0,&MaxColors,0,0 },
    { "mborderwidth",	IsNumeric,	0,&MBorderWidth,0,0 },
    { "menu",		IsMenu,		0,0,0,0 },
    { "menufont",	IsString,	0,0,MFontName,0 },
    { "noautoselect",	IsBoolFalse,	&Autoselect,0,0,0 },
    { "nofreeze",	IsBoolFalse,	&Freeze,0,0,0 },
    { "nogrid",		IsBoolFalse,	&Grid,0,0,0 },
    { "nonormali",	IsBoolFalse,	&NIcon,0,0,0 },
    { "nonormalw",	IsBoolFalse,	&NWindow,0,0,0 },
    { "noresizerelative",	IsBoolFalse,	&ResizeRelative,0,0,0 },
    { "noreverse",	IsBoolFalse,	&Reverse,0,0,0 },
    { "normali",	IsBoolTrue,	&NIcon,0,0,0 },
    { "normalw",	IsBoolTrue,	&NWindow,0,0,0 },
    { "nowarpdi",	IsBoolFalse,	&WarpOnDeIconify,0,0,0 },
    { "nowarpi",	IsBoolFalse,	&WarpOnIconify,0,0,0 },
    { "nowarpr",	IsBoolFalse,	&WarpOnRaise,0,0,0 },
    { "nozap",		IsBoolFalse,	&Zap,0,0,0 },
    { "push",		IsNumeric,	0,&Pushval,0,0 },
    { "pushabsolute",	IsBoolFalse,	&Push,0,0,0 },
    { "pushrelative",	IsBoolTrue,	&Push,0,0,0 },
    { "resetbindings",	IsParser,	0,0,0,ResetBindings },
    { "resetmenus",	IsParser,	0,0,0,ResetMenus },
    { "resetvariables",	IsParser,	0,0,0,ResetVariables },
    { "resizefont",	IsString,	0,0,PFontName,0 },
    { "resizerelative",	IsBoolTrue,	&ResizeRelative,0,0,0 },
    { "reverse",	IsBoolTrue,	&Reverse,0,0,0 },
    { "viconpad",	IsNumeric,	0,&VIconPad,0,0 },
    { "vmenupad",	IsNumeric,	0,&VMenuPad,0,0 },
    { "volume",		IsNumeric,	0,&Volume,0,0 },
    { "warpdi",		IsBoolTrue,	&WarpOnDeIconify,0,0,0 },
    { "warpi",		IsBoolTrue,	&WarpOnIconify,0,0,0 },
    { "warpr",		IsBoolTrue,	&WarpOnRaise,0,0,0 },
    { "zap",		IsBoolTrue,	&Zap,0,0,0 },
    { NULL,		NULL,		NULL,NULL,NULL,NULL }
};
 
/*
 * Key expression table for parser.
 */
KeyExpr KeyExprTbl[] = {
    { "ctrl", ControlMask },
    { "c", ControlMask },
    { "lock", LockMask },
    { "l", LockMask },
    { "shift", ShiftMask },
    { "s", ShiftMask },
    { "meta", Mod1Mask },
    { "m", Mod1Mask },
    { "mod1", Mod1Mask },
    { "1", Mod1Mask },
    { "mod2", Mod2Mask },
    { "2", Mod2Mask },
    { "mod3", Mod3Mask },
    { "3", Mod3Mask },
    { "mod4", Mod4Mask },
    { "4", Mod4Mask },
    { "mod5", Mod5Mask },
    { "5", Mod5Mask },
    { NULL, NULL }
};
 
/*
 * Context expression table for parser.
 */
ContExpr ContExprTbl[] = {
    { "icon", ICON },
    { "i", ICON },
    { "root", ROOT },
    { "r", ROOT },
    { "window", WINDOW },
    { "w", WINDOW },
    { NULL, NULL }
};
 
/*
 * Button expression table for parser.
 */
ButtonModifier ButtModTbl[] = {
    { "left", LeftMask },
    { "leftbutton", LeftMask },
    { "l", LeftMask },
    { "middle", MiddleMask },
    { "middlebutton", MiddleMask },
    { "m", MiddleMask },
    { "right", RightMask },
    { "rightbutton", RightMask },
    { "r", RightMask },
    { "move", DeltaMotion },
    { "motion", DeltaMotion },
    { "delta", DeltaMotion },
    { "down", ButtonDown },
    { "d", ButtonDown },
    { "up", ButtonUp },
    { "u", ButtonUp },
    { NULL, NULL }
};
 
Bool x_error_occurred = FALSE;
XErrorEvent last_error_event;
