// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * Copyright (C) 2006 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef Page_h
#define Page_h

#include "PlatformString.h"
#include <wtf/HashSet.h>

#if PLATFORM(WIN)
typedef struct HWND__* HWND;
typedef struct HINSTANCE__* HINSTANCE;
#endif

namespace WebCore {

    class Chrome;
    class ChromeClient;
    class Frame;
    class FrameNamespace;
    class FloatRect;
    class Screen;
    class ScreenClient;
    class Settings;
    class SelectionController;
    class Widget;

    class Page : Noncopyable {
    public:
        Page(PassRefPtr<ChromeClient>, PassRefPtr<ScreenClient>);
        ~Page();

        void setMainFrame(PassRefPtr<Frame>);
        Frame* mainFrame() const { return m_mainFrame.get(); }

        void setGroupName(const String&);
        String groupName() const { return m_groupName; }

        const HashSet<Page*>* frameNamespace() const;
        static const HashSet<Page*>* frameNamespace(const String&);

        void incrementFrameCount() { ++m_frameCount; }
        void decrementFrameCount() { --m_frameCount; }
        int frameCount() const { return m_frameCount; }

        static void setNeedsReapplyStyles();
        static void setNeedsReapplyStylesForSettingsChange(Settings*);

        SelectionController* dragCaretController() { return m_dragCaretController; }
        Chrome* chrome() { return m_chrome; }
        Screen* screen() { return m_screen; }

        void setDefersLoading(bool);
        bool defersLoading() const { return m_defersLoading; }

#if PLATFORM(WIN)
        // The global DLL or application instance used for all windows.
        static void setInstanceHandle(HINSTANCE instanceHandle) { s_instanceHandle = instanceHandle; }
        static HINSTANCE instanceHandle() { return s_instanceHandle; }
#endif

    private:
        SelectionController* m_dragCaretController;
        Chrome* m_chrome;
        Screen* m_screen;

        RefPtr<Frame> m_mainFrame;
        int m_frameCount;
        String m_groupName;

        bool m_defersLoading;

#if PLATFORM(WIN)
        static HINSTANCE s_instanceHandle;
#endif
    };

} // namespace WebCore
    
#endif // Page_h
