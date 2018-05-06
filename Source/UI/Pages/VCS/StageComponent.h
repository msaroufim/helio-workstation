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

//[Headers]
class VersionControl;
class ProgressIndicator;

#include "HeadlineItemDataSource.h"
#include "ComponentFader.h"
#include "Revision.h"
//[/Headers]

#include "../../Themes/FramePanel.h"

class StageComponent final : public Component,
                             public ListBoxModel,
                             public ChangeListener,
                             public HeadlineItemDataSource
{
public:

    StageComponent(VersionControl &versionControl);
    ~StageComponent();

    //[UserMethods]

    void selectAll(NotificationType notificationType);
    void clearSelection();

    //===------------------------------------------------------------------===//
    // ChangeListener - listens VCS head to refresh diff info
    //===------------------------------------------------------------------===//

    void changeListenerCallback(ChangeBroadcaster *source) override;

    //===------------------------------------------------------------------===//
    // ListBoxModel
    //===------------------------------------------------------------------===//

    int getNumRows() override;
    Component *refreshComponentForRow(int rowNumber,
        bool isRowSelected, Component *existingComponentToUpdate) override;
    void paintListBoxItem(int rowNumber, Graphics &g,
        int width, int height, bool rowIsSelected) override {}
    void selectedRowsChanged(int lastRowSelected) override;

    //===------------------------------------------------------------------===//
    // HeadlineItemDataSource
    //===------------------------------------------------------------------===//

    bool hasMenu() const noexcept override;
    ScopedPointer<Component> createMenu() override;
    Image getIcon() const override;
    String getName() const override;
    bool canBeSelectedAsMenuItem() const override;

    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void handleCommandMessage (int commandId) override;


private:

    //[UserVariables]

    VersionControl &vcs;

    ReadWriteLock diffLock;
    ValueTree lastDiff;
    String lastCommitMessage;

    ComponentFader fader;
    void startProgressAnimation();
    void stopProgressAnimation();

    void updateList();
    void clearList();

    void toggleQuickStash();
    void commitSelected();
    void resetSelected();

    //[/UserVariables]

    ScopedPointer<Component> horizontalCenter;
    ScopedPointer<Label> titleLabel;
    ScopedPointer<ProgressIndicator> indicator;
    ScopedPointer<FramePanel> panel;
    ScopedPointer<ListBox> changesList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StageComponent)
};
