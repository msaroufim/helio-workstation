/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Revision.h"
#include "Pack.h"

namespace VCS
{
    class HeadState;
    class TrackedItem;
    class TrackedItemsSource;
    class DeltaDataSource;

    class Head :
        private Thread,
        public ChangeListener, // listens to project changes to set diff outdated
        public ChangeBroadcaster, // broadcasts the diff rebuild has started/ended
        public Serializable
    {
    public:

        Head(const Head &other);
        explicit Head(Pack::Ptr packPtr,
             WeakReference<TrackedItemsSource> targetProject = nullptr);

        ValueTree getHeadingRevision() const;
        
        ValueTree getDiff() const;
        bool isDiffOutdated() const;
        void setDiffOutdated(bool isOutdated);

        bool isRebuildingDiff() const; // mainly for change-listeners
        void setRebuildingDiffMode(bool isBuildingNow);
        
        bool hasAnythingOnTheStage() const;
        bool hasTrackedItemsOnTheStage() const;

        void mergeStateWith(ValueTree changes);
        bool moveTo(const ValueTree revision); // rebuilds state index
        void pointTo(const ValueTree revision); // does not rebuild index

        void checkout();
        void cherryPick(const Array<Uuid> uuids);
        void cherryPickAll();
        bool resetChanges(const Array<RevisionItem::Ptr> &changes);

        void rebuildDiffIfNeeded(); // called from the editor when it gets visible
        void rebuildDiffNow(); // called from the visible editor, when it receives vcs change message 
        void rebuildDiffSynchronously(); // a hack foe quick-stash
        
        //===--------------------------------------------------------------===//
        // Serializable
        //===--------------------------------------------------------------===//

        ValueTree serialize() const override;
        void deserialize(const ValueTree &tree) override;
        void reset() override;
        
        //===--------------------------------------------------------------===//
        // ChangeListener
        //===--------------------------------------------------------------===//

        void changeListenerCallback(ChangeBroadcaster *source) override;

    private:

        //===--------------------------------------------------------------===//
        // Thread
        //===--------------------------------------------------------------===//

        void run() override;
        void checkoutItem(VCS::RevisionItem::Ptr stateItem);
        bool resetChangedItemToState(const VCS::RevisionItem::Ptr diffItem);

        ReadWriteLock outdatedMarkerLock;
        bool diffOutdated;

        ReadWriteLock diffLock;
        ValueTree diff;
        
        ReadWriteLock rebuildingDiffLock;
        bool rebuildingDiffMode;

    private:

        ValueTree headingAt;
        ReadWriteLock stateLock;
        ScopedPointer<HeadState> state;

    private:

        WeakReference<TrackedItemsSource> targetVcsItemsSource; // ProjectTreeItem

        Pack::Ptr pack;

        JUCE_LEAK_DETECTOR(Head)

    };
} // namespace VCS
