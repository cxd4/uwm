/* $XConsortium: colormap.c,v 1.4 88/10/20 12:14:19 jim Exp $ */

# include <X11/copyright.h>

# include "uwm.h"

Bool	focusPointerRoot;

Window
getMouse ()
{
    Window	root, child;
    int		root_x, root_y, x, y;
    unsigned int	mask;

    XQueryPointer (dpy, RootWindow (dpy, scr), &root, &child, &root_x, &root_y, &x, &y, &mask);
    return child ? child : root;
}

initColormap ()
{
    Colormap	*colormaps;
    int		ncolormaps;
    int		i;
    extern int	initColormapTrack (), colormapTrack ();
	
    colormaps = XListInstalledColormaps (dpy, RootWindow (dpy, scr), &ncolormaps);
    for (i = 0; i < ncolormaps; i++)
	noteInstalledColormap (colormaps[i]);
    expressWindowInterest (initColormapTrack, colormapTrack);
    focusPointerRoot = TRUE;
}

initColormapTrack (wi, wa)
    WindowInfo		*wi;
    XWindowAttributes	*wa;
{
    if (wa) {
	wi->colormap = wa->colormap;
	wi->event_mask |= EnterWindowMask|LeaveWindowMask|ColormapChangeMask|FocusChangeMask;
	wi->in_window = FALSE;
	noteWindowAttached (wi->window, wi->colormap);
	if (getMouse () == wi->window) {
	    wi->in_window = TRUE;
	    installColormap (wi->colormap);
	}
    } else {
	wi->event_mask &= ~EnterWindowMask|LeaveWindowMask|ColormapChangeMask|FocusChangeMask;
	if (wi->in_window)
	    uninstallColormap (wi->colormap);
	noteWindowUnattached (wi->window, wi->colormap);
    }
}

colormapTrack (wi, ev)
   WindowInfo	*wi;
   XAnyEvent	*ev;
{
   XColormapEvent	*cev;
   XCrossingEvent	*eev;
   XFocusChangeEvent	*fev;

   switch (ev->type) {
   case ColormapNotify:
	cev = (XColormapEvent *) ev;
	switch (cev->state) {
	case ColormapInstalled:
	    noteInstalledColormap (cev->colormap);
	    break;
	case ColormapUninstalled:
	    noteUninstalledColormap (cev->colormap);
	    break;
	}
	if (cev->new) {
	    if (wi->in_window) {
#ifdef DEBUG
	    	printf ("reset colormap ");
#endif /* DEBUG */
		uninstallColormap (wi->colormap);
		wi->colormap = cev->colormap;
#ifdef DEBUG
	    	printf ("reset colormap ");
#endif /* DEBUG */
		installColormap (wi->colormap);
	    } else
	        wi->colormap = cev->colormap;
	}
	break;
    case FocusIn:
	fev = (XFocusChangeEvent *) ev;
	switch (fev->detail) {
	case NotifyInferior:	/* might be root */
	    if (fev->window != RootWindow (dpy, scr))
		break;
	    goto focusIn;
	case NotifyNonlinearVirtual:	/* don't want root */
	    if (fev->window == RootWindow (dpy, scr))
		break;
	case NotifyNonlinear:	/* might be root */
	case NotifyVirtual:	/* can't be root */
	case NotifyAncestor:
	case NotifyPointer:
	focusIn:
	    if (fev->window == RootWindow (dpy, scr))
		focusPointerRoot = TRUE;
#ifdef DEBUG
	    printf ("FocusIn ");
#endif /* DEBUG */
	    wi->in_window = TRUE;
	    installColormap (wi->colormap);
	    break;
	case NotifyPointerRoot:
	    focusPointerRoot = TRUE;
	    break;
	case NotifyDetailNone:
	    XSetInputFocus (dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	    break;
	}
	break;
    case FocusOut:
    	fev = (XFocusChangeEvent *) ev;
	switch (fev->detail) {
	case NotifyInferior:	/* might be root */
	    if (fev->window != RootWindow (dpy, scr))
		break;
	case NotifyNonlinearVirtual:	/* don't want root */
	case NotifyNonlinear:	/* might be root */
	case NotifyVirtual:	/* can't be root */
	case NotifyAncestor:
	case NotifyPointer:
	    if (fev->window == RootWindow (dpy, scr))
		focusPointerRoot = FALSE;
#ifdef DEBUG
	    printf ("FocusOut %x %d ", fev->window, fev->detail);
#endif /* DEBUG */
	    wi->in_window = FALSE;
	    uninstallColormap (wi->colormap);
	    break;
	case NotifyPointerRoot:
	    focusPointerRoot = FALSE;
	    break;
	case NotifyDetailNone:
	    break;
	}
	break;
	/*
	 * simulate focusin/focusout events in pointer root mode
	 */
    case EnterNotify:
	eev = (XCrossingEvent *) ev;
	if (eev->mode != NotifyGrab && focusPointerRoot) {
	    switch (eev->detail) {
	    case NotifyInferior:
	        if (eev->window != RootWindow (dpy, scr))
		    break;
		goto enter;
	    case NotifyNonlinearVirtual:
	    	if (eev->window == RootWindow (dpy, scr))
		    break;
	    case NotifyVirtual:
	    case NotifyAncestor:
	    case NotifyNonlinear:
	    enter:
		wi->in_window = TRUE;
#ifdef DEBUG
		printf ("EnterNotify %6x %2d ", eev->window, eev->detail);
#endif /* DEBUG */
		installColormap (wi->colormap);
	    }
	}
	break;
    case LeaveNotify:
	eev = (XCrossingEvent *) ev;
	if (eev->mode != NotifyGrab && focusPointerRoot) {
	    switch (eev->detail) {
	    case NotifyInferior:
	        if (eev->window != RootWindow (dpy, scr))
		    break;
	    case NotifyNonlinearVirtual:
	    case NotifyVirtual:
	    case NotifyAncestor:
	    case NotifyNonlinear:
		wi->in_window = FALSE;
#ifdef DEBUG
		printf ("LeaveNotify %6x %2d ", eev->window, eev->detail);
#endif /* DEBUG */
		uninstallColormap (wi->colormap);
		break;
	    }
	}
	break;
   }
}

struct colormapInfo {
	struct colormapInfo	*next;
	Colormap		colormap;
	int			window_count;	/* number of windows using this colormap */
	Bool			actual;
	Bool			desired;
};

static struct colormapInfo	*colormaps;

static struct colormapInfo *
lookupColormap (colormap)
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    for (ci = colormaps; ci; ci=ci->next)
	if (ci->colormap == colormap)
	    return ci;
    ci = (struct colormapInfo *) malloc (sizeof (*ci));
    ci->actual = FALSE;
    ci->desired = FALSE;
    ci->window_count = 0;
    ci->next = colormaps;
    ci->colormap = colormap;
    colormaps = ci;
    return ci;
}

deleteColormap (colormap)
    Colormap	colormap;
{
    struct colormapInfo	*ci, *prev;

    prev = 0;
    for (ci = colormaps; ci; ci=ci->next) {
        if (ci->colormap == colormap) {
	    if (prev)
	    	prev->next = ci->next;
	    else
	    	colormaps = ci->next;
	    free ((char *) ci);
	    break;
	}
	prev = ci;
    }
}

noteWindowAttached (window, colormap)
    Window	window;
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    ci = lookupColormap (colormap);
    ci->window_count++;
}

noteWindowUnattached (window, colormap)
    Window	window;
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    ci = lookupColormap (colormap);
    if (--ci->window_count == 0)
	deleteColormap (colormap);
}
    
noteInstalledColormap (colormap)
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    ci = lookupColormap (colormap);
    if (!ci->actual)
#ifdef DEBUG
	printf ("note installed %x %d %s %s\n", colormap, ci->window_count,
		ci->actual  ? "installed" : "uninstalled",
		ci->desired ? "desired" : "not desired");
#endif /* DEBUG */
    ci->actual = TRUE;
    /* any random colormaps don't get installed  */
    if (ci->window_count == 0) {
	for (ci = colormaps; ci; ci=ci->next)
	    if (ci->desired)
	        XInstallColormap (dpy, ci->colormap);
    }
}

noteUninstalledColormap (colormap)
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    ci = lookupColormap (colormap);
    if (ci->actual)
#ifdef DEBUG
	printf ("note uninstalled %x %d %s %s\n", colormap, ci->window_count,
		ci->actual  ? "installed" : "uninstalled",
		ci->desired ? "desired" : "not desired");
#endif /* DEBUG */
    ci->actual = FALSE;
    if (ci->desired)
	XInstallColormap (dpy, colormap);
}

installColormap (colormap)
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    for (ci = colormaps; ci; ci=ci->next)
	if (ci->desired) {
#ifdef DEBUG
	    printf ("apparently lost uninstall event for colormap %x\n", ci->colormap);
#endif /* DEBUG */
	    return;
	}

    ci = lookupColormap (colormap);
#ifdef DEBUG
    printf ("install %x %d %s %s\n", ci->colormap, ci->window_count,
    	ci->actual  ? "installed" : "uninstalled",
	ci->desired ? "desired" : "not desired");
#endif /* DEBUG */
    if (ci->desired)
	return;
    ci->desired = TRUE;
    if (!ci->actual)
	XInstallColormap (dpy, colormap);
}

uninstallColormap (colormap)
    Colormap	colormap;
{
    struct colormapInfo	*ci;

    ci = lookupColormap (colormap);
    if (ci->desired)
#ifdef DEBUG
	printf ("uninstall %x %d %s %s\n", ci->colormap, ci->window_count,
    	ci->actual  ? "installed" : "uninstalled",
	ci->desired ? "desired" : "not desired");
#endif /* DEBUG */
    ci->desired = FALSE;
}
