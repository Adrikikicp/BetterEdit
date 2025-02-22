#pragma once

#include "GroupSummaryPopup.hpp"
#include "MoreTriggersPopup.hpp"

static constexpr const BoomListType kBoomListType_Group
    = static_cast<BoomListType>(0x429);

class GroupListView;
class MoreTriggersPopup;
class ContextPopup;

class GroupCell :
    public TableViewCell,
    public ContextPopupDelegate
{
    protected:
        int m_nGroup;
        GroupSummaryPopup* m_pPopup;
        CCLabelBMFont* m_pLabel;
        MoreTriggersPopup* m_pContextPopup = nullptr;

		GroupCell(const char* name, CCSize size);
        virtual ~GroupCell();

        void draw() override;
        void onShowRestOfTheTriggers(CCObject*);
        void onSelectGroup(CCObject*);

        void contextPopupWillHide(ContextPopup*) override;

        friend class GroupListView;
	
	public:
        void loadFromGroup(int);

		static GroupCell* create(const char* key, CCSize size);
};

class GroupListView : public CustomListView {
    protected:
        GroupSummaryPopup* m_pPopup;

        void setupList() override;
        TableViewCell* getListCell(const char* key) override;
        void loadCell(TableViewCell* cell, unsigned int index) override;
    
    public:
        static GroupListView* create(
            GroupSummaryPopup* popup,
            CCArray* actions,
            float width,
            float height
        );
};
