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

#include "TreeItem.h"

class TreeNavigationHistoryLock final
{
public:
    TreeNavigationHistoryLock() {}
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TreeNavigationHistoryLock)
    JUCE_DECLARE_WEAK_REFERENCEABLE(TreeNavigationHistoryLock)
};

class TreeNavigationHistory final : public ChangeBroadcaster
{
public:
    
    TreeNavigationHistory();

    ScopedPointer<TreeNavigationHistoryLock> lock();
        
    bool canGoForward() const;
    bool canGoBackward() const;
    
    WeakReference<TreeItem> goBack();
    WeakReference<TreeItem> goForward();
    
    WeakReference<TreeItem> getCurrentItem() const;
    bool addItemIfNeeded(TreeItem *item);
    
private:
    
    Array<WeakReference<TreeItem>> list;
    
    // A way to prevent new items from being added when navigating back/forward
    WeakReference<TreeNavigationHistoryLock> historyLock;
    
    int currentPageIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TreeNavigationHistory)
};
