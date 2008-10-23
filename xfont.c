/*
 * xfont.c - old style X fonts for cwm
 *
 * Copyright (c) 2008 Marius Eriksen <marius@monkey.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "hash.h"
#include "headers.h"
#include "calmwm.h"

static XFontStruct *_make_font(struct screen_ctx *sc, struct fontdesc *fdp);

HASH_GENERATE(fonthash, fontdesc, node, fontdesc_cmp);

int
fontdesc_cmp(struct fontdesc *a, struct fontdesc *b)
{
	return (strcmp(a->name, b->name));
}

/*
 * Fowler/Noll/Vo hash
 *    http://www.isthe.com/chongo/tech/comp/fnv/
 */

#define	FNV_P_32	((unsigned int)0x01000193)	/* 16777619 */
#define	FNV_1_32	((unsigned int)0x811c9dc5)	/* 2166136261 */

unsigned int
fontdesc_hash(struct fontdesc *fdp)
{
	const unsigned char *p, *end, *start;
	unsigned int hash = FNV_1_32;

	start = fdp->name;
	end = (const unsigned char *)fdp->name + strlen(fdp->name);

	for (p = start; p < end; p++) {
		hash *= FNV_P_32;
		hash ^= (unsigned int)*p;
	}

	return (hash);
}

void
font_init(struct screen_ctx *sc)
{
	HASH_INIT(&sc->fonthash, fontdesc_hash);
}

struct fontdesc *
font_getx(struct screen_ctx *sc, const char *name)
{
	struct fontdesc *fdp;

	if ((fdp = font_get(sc, name)) == NULL)
		errx(1, "font_get(%s)", name);

	return (fdp);
}

struct fontdesc *
font_get(struct screen_ctx *sc, const char *name)
{
	struct fontdesc fd, *fdp;
	XFontStruct *xfont;

	fd.name = name;

	if ((fdp = HASH_FIND(fonthash, &sc->fonthash, &fd)) == NULL
	    && (xfont = _make_font(sc, &fd)) != NULL) {
		fdp = xmalloc(sizeof(*fdp));
		fdp->name = xstrdup(fd.name);
		fdp->xfont = xfont;
		fdp->sc = sc;
		HASH_INSERT(fonthash, &sc->fonthash, fdp);
	}

	return (fdp);
}

int
xfont_width(struct fontdesc *fdp, const char *text, int len)
{
	return (XTextWidth(fdp->xfont, text, len));
}

void
xfont_draw(struct fontdesc *fdp, const char *text, int len,
    Drawable d, int x, int y)
{
	GC gc = XCreateGC(X_Dpy, fdp->sc->rootwin, 0, 0);
        XSetFont(X_Dpy, gc, fdp->xfont->fid);
        XDrawString(X_Dpy, d, gc, x, y, text, len);
}

int
xfont_ascent(struct fontdesc *fdp)
{
	return (fdp->xfont->ascent);
}

int
xfont_descent(struct fontdesc *fdp)
{
	return (fdp->xfont->descent);
}

static XFontStruct *
_make_font(struct screen_ctx *sc, struct fontdesc *fdp)
{
        return (XLoadQueryFont(X_Dpy, fdp->name));
}
