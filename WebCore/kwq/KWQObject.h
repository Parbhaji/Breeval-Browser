/*
 * Copyright (C) 2001 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef QOBJECT_H_
#define QOBJECT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <KWQDef.h>

#include "qnamespace.h"
#include "qstring.h"
#include "qevent.h"
#include "qstringlist.h"

// FIXME: should these macros be in "kwq.h" or other header file?
#define slots
#define SLOT(x) """## x ##"""
#define signals protected
#define SIGNAL(x) """## x ##"""
#define emit
#define Q_OBJECT
#define Q_PROPERTY(text)

class QEvent;
class QPaintDevice;
class QPaintDeviceMetrics;
class QWidget;
class QColor;
class QColorGroup;
class QPalette;
class QPainter;
class QRegion;
class QSize;
class QSizePolicy;
class QRect;
class QFont;
class QFontMetrics;
class QBrush;
class QBitmap;
class QMovie;
class QTimer;
class QImage;
class QVariant;

// class QObject ===============================================================

class QObject : public Qt {
public:

#ifdef _KWQ_
    enum Actions {
        ACTION_BUTTON_CLICKED = 1,
        ACTION_TEXT_FIELD = 2,  // corresponds to [NSTextField action]
        ACTION_TEXT_FIELD_END_EDITING = 3 // corresponds to NSTextField's delegate textDidEndEditing:
    };
#endif

    // typedefs ----------------------------------------------------------------
    // enums -------------------------------------------------------------------
    // constants ---------------------------------------------------------------

    // static member functions -------------------------------------------------

    static bool connect(const QObject *, const char *, const QObject *, 
        const char *);
    
    static bool disconnect( const QObject *, const char *, const QObject *, 
        const char *);

    // constructors, copy constructors, and destructors ------------------------

    QObject(QObject *parent=0, const char *name=0);
    virtual ~QObject();

    // member functions --------------------------------------------------------

    const char *name() const;
    virtual void setName(const char *);

    QVariant property(const char *name) const;
    bool inherits(const char *) const;
    bool connect(const QObject *src, const char *signal, const char *slot) const;

    int startTimer(int);
    void killTimer(int);
    void killTimers();

    void installEventFilter(const QObject *);
    void removeEventFilter(const QObject *);

    void blockSignals(bool);

#ifdef _KWQ_
    virtual void performAction(QObject::Actions action);
    void emitAction(QObject::Actions action);
    void setTarget (QObject *obj);
#endif    
    
    // operators ---------------------------------------------------------------

// protected -------------------------------------------------------------------
// private ---------------------------------------------------------------------

private:
    // no copying or assignment
    // note that these are "standard" (no pendantic stuff needed)
    QObject(const QObject &);
    QObject &operator=(const QObject &);

    QObject *target;
}; // class QObject ============================================================

#endif
