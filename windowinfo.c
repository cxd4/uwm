/* $XConsortium: windowinfo.c,v 1.7 88/10/23 12:08:25 jim Exp $ */

#include <X11/copyright.h>

# include "uwm.h"

XContext	windowInfo;

typedef struct _windowInterest {
    struct _windowInterest	*next;
    int				(*initAndDone)();
    int				(*eventHandler)();
} WindowInterest;

static WindowInterest		*interests;

void
expressWindowInterest (initAndDone, eventHandler)
    int (*initAndDone)();
    int (*eventHandler)();
{
    WindowInterest	*wi;

    for (wi = interests; wi; wi=wi->next)
	if (wi->eventHandler == eventHandler)
	    return;
    wi = (WindowInterest *) malloc (sizeof (*wi));
    wi->next = interests;
    interests = wi;
    wi->initAndDone = initAndDone;
    wi->eventHandler = eventHandler;
}

void
expressWindowDisinterest (eventHandler)
    int (*eventHandler)();
{
    WindowInterest	*wi, *prev;

    prev = 0;
    for (wi = interests; wi; wi=wi->next) {
	if (wi->eventHandler == eventHandler) {
	    if (prev)
		prev->next = wi->next;
	    else
		interests = wi->next;
	    free ((char *) wi);
	    return;
	}
	prev = wi;
    }
}

processInterestEvent (ev)
    XAnyEvent	*ev;
{
    WindowInfo	*i;
    WindowInterest	*wi;
    long eventTypeToMask();

    i = getWindowInfo (ev->window, 0);
    if (i) {
	for (wi = interests; wi; wi=wi->next)
	    if (wi->eventHandler)
		(*wi->eventHandler) (i, ev);
	/*
	 * nuke this event so uwm doesn't get confused
	 */
	if ((eventTypeToMask (ev->type) & i->uwm_event_mask) == 0)
	    ev->type = 0;
    }
}

static WindowInfo *
initWindowInfo (w)
    Window	w;
{
    WindowInfo	*i;
    XWindowAttributes	wa;
    WindowInterest	*wi;

    if (XGetWindowAttributes (dpy, w, &wa)) {
	i = (WindowInfo *) malloc (sizeof (*i));
	i->window = w;
	i->event_mask = 0;
	i->uwm_event_mask = 0;
	XSaveContext (dpy, w, windowInfo, (caddr_t) i);
	for (wi = interests; wi; wi=wi->next)
	    (*wi->initAndDone) (i, &wa);
	if (i->event_mask)
	    XSelectInput (dpy, w, i->event_mask);
	i->been_mapped = False;
	bzero ((char *)&(i->stashedhints), sizeof (XSizeHints));
	return i;
    }
    return 0;
}

deleteWindowInfo (w)
    Window	w;
{
    WindowInfo	*i;
    WindowInterest	*wi;

    if (XFindContext (dpy, w, windowInfo, &i) == XCSUCCESS) {
	for (wi = interests; wi; wi = wi->next)
	    (*wi->initAndDone) (i, (XWindowAttributes *) 0);
	XDeleteContext (dpy,  w, windowInfo);
	free ((char *) i);
    }
}

WindowInfo *
getWindowInfo (w)
    Window	w;
{
    WindowInfo	*i = NULL;

    if (!windowInfo) {
	windowInfo = XUniqueContext ();
    }
    if (XFindContext (dpy, w, windowInfo, (caddr_t *) &i) == XCNOENT)
	i = initWindowInfo (w);
    return i;
}

uwmExpressInterest (w, eventMask)
    Window	w;
    long	eventMask;
{
    WindowInfo	*i;

    i = getWindowInfo (w);
    if (i) {
	if (i->uwm_event_mask | eventMask != i->uwm_event_mask) {
	    i->uwm_event_mask |= eventMask;
	    if (i->event_mask | i->uwm_event_mask != i->event_mask) {
		i->event_mask |= i->uwm_event_mask;
		XSelectInput (dpy, w, i->event_mask);
	    }
	}
    }
}

long foo_masks[] = {
	0, 0,
	KeyPressMask,					/* KeyPress */
	KeyPressMask,					/* KeyRelease */
	ButtonPressMask,				/* ButtonPress */
	ButtonReleaseMask,				/* ButtonRelease */
	PointerMotionMask|ButtonMotionMask|Button1MotionMask|Button2MotionMask|Button3MotionMask|Button4MotionMask|Button5MotionMask,
							/* MotionNotify */
	EnterWindowMask,				/* EnterNotify */
	LeaveWindowMask,				/* LeaveNotify */
	FocusChangeMask,				/* FocusIn */
	FocusChangeMask,				/* FocusOut */
	KeymapStateMask,				/* KeymapNotify */
	ExposureMask,					/* Expose */
	ExposureMask,					/* GraphicsExpose */
	ExposureMask,					/* NoExpose */
	VisibilityChangeMask,				/* VisibilityNotify */
	SubstructureNotifyMask,				/* CreateNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* DestroyNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* UnmapNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* MapNotify */
	SubstructureRedirectMask,			/* MapRequest */
	SubstructureNotifyMask,				/* ReparentNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* ConfigureNotify */
	SubstructureRedirectMask,			/* ConfigureRequest */
	StructureNotifyMask|SubstructureNotifyMask,	/* GravityNotify */
	SubstructureRedirectMask,			/* ResizeRequest */
	StructureNotifyMask|SubstructureNotifyMask,	/* CirculateNotify */
	SubstructureRedirectMask,			/* CirculateRequest */
	PropertyChangeMask,				/* PropertyNotify */
	~0,						/* SelectionClear */
	~0,						/* SelectionRequest */
	~0,						/* SelectionNotify */
	ColormapChangeMask,				/* ColormapNotify */
	SubstructureRedirectMask,			/* ClientMessage */
	~0,						/* MappingNotify */
    };

long
eventTypeToMask (t)
    int	t;
{
    if (0 <= t && t < sizeof foo_masks / sizeof foo_masks[0])
	return foo_masks[t];
    return 0;
}
