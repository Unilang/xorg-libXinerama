/* $XdotOrg$ */
/*************************************************************************
*
* Copyright (c) 1999,2002 Hewlett-Packard Company
* Copyright (c) 1999,2002 IBM Corporation
* Copyright (c) 1999,2002 Hummingbird Communications Ltd.
* Copyright (c) 1999,2002 Silicon Graphics, Inc.
* Copyright (c) 1999,2002 Sun Microsystems, Inc.
* Copyright (c) 1999,2002 The Open Group
*
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a 
* copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation 
* the rights to use, copy, modify, merge, publish, distribute, and/or sell 
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, provided that the above copyright notice(s) and this 
* permission notice appear in all copies of the Software and that both the 
* above copyright notice(s) and this permission notice appear in supporting 
* documentation.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR
* ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR
* ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
* WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
* ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
* OF THIS SOFTWARE.
*
* Except as contained in this notice, the name of a copyright holder shall 
* not be used in advertising or otherwise to promote the sale, use or other 
* dealings in this Software without prior written authorization of the 
* copyright holder.
*
*************************************************************************/

#define NEED_EVENTS
#define NEED_REPLIES
#include <stdio.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/xineramaProto.h>
#include <X11/extensions/Xinerama.h>


static XExtensionInfo _xinerama_ext_info_data;
static XExtensionInfo *xinerama_ext_info = &_xinerama_ext_info_data;
static char *xinerama_extension_name = XINERAMA_PROTOCOL_NAME;

#define XineramaCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xinerama_extension_name, val)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, xinerama_ext_info)

static XExtensionHooks xinerama_extension_hooks = {
    NULL,                               /* create_gc */
    NULL,                               /* copy_gc */
    NULL,                               /* flush_gc */
    NULL,                               /* free_gc */
    NULL,                               /* create_font */
    NULL,                               /* free_font */
    close_display,                      /* close_display */
    NULL,                               /* wire_to_event */
    NULL,                               /* event_to_wire */
    NULL,                               /* error */
    NULL,                               /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, xinerama_ext_info,
                                   xinerama_extension_name, 
                                   &xinerama_extension_hooks,
                                   0, NULL)

Bool XineramaQueryExtension (
    Display *dpy,
    int *event_basep,
    int *error_basep
)
{
    XExtDisplayInfo *info = find_display (dpy);

    if (XextHasExtension(info)) {
        *event_basep = info->codes->first_event;
        *error_basep = info->codes->first_error;
        return True;
    } else {
        return False;
    }
}

Status XineramaQueryVersion(
    Display *dpy,
    int *major_version, 
    int *minor_version
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXineramaQueryVersionReq *req;
    xXineramaQueryVersionReply rep;

    XineramaCheckExtension (dpy, info, 0);

    LockDisplay (dpy);
    GetReq (XineramaQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->XineramaReqType = X_XineramaQueryVersion;
    req->client_major_version = XINERAMA_MAJOR_VERSION;
    req->client_minor_version = XINERAMA_MINOR_VERSION;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return 0;
    }
    *major_version = rep.server_major_version;
    *minor_version = rep.server_minor_version;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}

Bool XineramaActive (
    Display *dpy,
    Window window 
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXineramaActiveReply rep;
    xXineramaActiveReq *req;
    Bool active;

    XineramaCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (XineramaActive, req);
    req->reqType = info->codes->major_opcode;
    req->XineramaReqType = X_XineramaActive;
    req->window = window;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    active = rep.active;
    UnlockDisplay (dpy);
    SyncHandle ();
    return active;
}



Status XineramaGetData(
    Display *dpy,
    Window window,
    XRectangle **framebuffer_rects,
    int *number_framebuffers
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXineramaGetDataReq *req;
    xXineramaGetDataReply rep;
    XRectangle *rects;
    int result = 0;

    *number_framebuffers = 0;
    *framebuffer_rects = NULL;

    XineramaCheckExtension (dpy, info, 0);

    LockDisplay (dpy);
    GetReq (XineramaGetData, req);
    req->reqType = info->codes->major_opcode;
    req->XineramaReqType = X_XineramaGetData;
    req->window = window;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return 0;
    }

    if(rep.num_rects) {
        if((rects = Xmalloc(sizeof(XRectangle) * rep.num_rects))) {
            xXineramaRectangle scratch;
            int i;

            for(i = 0; i < rep.num_rects; i++) {
                _XRead(dpy, (char*)&scratch, sz_xXineramaRectangle);
                rects[i].x = scratch.x;
                rects[i].y = scratch.y;
                rects[i].width = scratch.width;
                rects[i].height = scratch.height;
            }
            *framebuffer_rects = rects;
            *number_framebuffers = rep.num_rects;
            result = 1;
        } else {
            _XEatData(dpy, rep.length << 2);
        }
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return result;
}

#define HINT_NAME "XINERAMA_CENTER_HINT"
#define HINT_TYPE "INTEGER"

Status XineramaGetCenterHint (
    Display *dpy,
    Window root,
    int *x,
    int *y
)
{
    XExtDisplayInfo *info = find_display (dpy);
    Atom atom, type, actual_type;
    unsigned long nitems, bytes_after;
    int actual_format, status;
    short *prop;

    XineramaCheckExtension (dpy, info, 0);

    type = XInternAtom(dpy, HINT_TYPE, False);
    atom = XInternAtom(dpy, HINT_NAME, True);

    if(atom == None) return 0;  /* no such atom */

    if( (status = XGetWindowProperty(dpy, root, atom, 0, 1, False, type, 
				     &actual_type, &actual_format, &nitems, 
				     &bytes_after, (unsigned char**)(&prop))) 
	!= Success)
	return 0;

    if(actual_type == None) return 0;  /* no such property */

    *x = prop[0];
    *y = prop[1];

    if (prop) XFree(prop);

    return 1;
}


Status XineramaSetCenterHint (
    Display *dpy,
    Window root,
    int x,
    int y
)
{
    XExtDisplayInfo *info = find_display (dpy);
    Atom atom, type;
    short data[2];

    XineramaCheckExtension (dpy, info, 0);

    atom = XInternAtom(dpy, HINT_NAME, False);
    type = XInternAtom(dpy, HINT_TYPE, False);

    data[0] = x;
    data[1] = y;

    return XChangeProperty(dpy, root, atom, type, 16, PropModeReplace, 
                           (unsigned char*)data, 2);
}


