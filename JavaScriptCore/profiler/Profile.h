/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef Profile_h
#define Profile_h

#include "ProfileNode.h"
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace KJS {

    class ExecState;
    typedef void (*SortFunction)(ProfileNode*);

    class Profile : public RefCounted<Profile> {
    public:
        static PassRefPtr<Profile> create(const UString& title, ExecState* originatingGlobalExec, unsigned pageGroupIdentifier) { return adoptRef(new Profile(title, originatingGlobalExec, pageGroupIdentifier)); }

        void willExecute(const CallIdentifier&);
        void didExecute(const CallIdentifier&);
        void stopProfiling();

        const UString& title() const { return m_title; };
        ProfileNode* callTree() const { return m_headNode.get(); };

        double totalTime() const { return m_headNode->totalTime(); }

        ExecState* originatingGlobalExec() const { return m_originatingGlobalExec; }
        unsigned pageGroupIdentifier() const { return m_pageGroupIdentifier; }

        void sort(SortFunction);
        void sortTotalTimeDescending() { sort(ProfileNode::sortTotalTimeDescending); }
        void sortTotalTimeAscending() { sort(ProfileNode::sortTotalTimeAscending); }
        void sortSelfTimeDescending() { sort(ProfileNode::sortSelfTimeDescending); }
        void sortSelfTimeAscending() { sort(ProfileNode::sortSelfTimeAscending); }
        void sortCallsDescending() { sort(ProfileNode::sortCallsDescending); }
        void sortCallsAscending() { sort(ProfileNode::sortCallsAscending); }
        void sortFunctionNameDescending() { sort(ProfileNode::sortFunctionNameDescending); }
        void sortFunctionNameAscending() { sort(ProfileNode::sortFunctionNameAscending); }

        void focus(const ProfileNode* profileNode) { if (!profileNode) return; m_headNode->focus(profileNode->callIdentifier()); }
        void exclude(const ProfileNode* profileNode) { if (!profileNode) return; m_headNode->exclude(profileNode->callIdentifier()); }
        void restoreAll() { m_headNode->restoreAll(); }

#ifndef NDEBUG
        void debugPrintData() const;
        void debugPrintDataSampleStyle() const;
#endif

        typedef void (Profile::*ProfileFunction)(const CallIdentifier& callIdentifier);

    private:
        Profile(const UString& title, ExecState* originatingGlobalExec, unsigned pageGroupIdentifier);

        UString m_title;
        ExecState* m_originatingGlobalExec;
        unsigned m_pageGroupIdentifier;
        RefPtr<ProfileNode> m_headNode;
        RefPtr<ProfileNode> m_currentNode;
        
        unsigned m_depth;
    };

} // namespace KJS

#endif // Profiler_h
