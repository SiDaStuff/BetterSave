/**
 * BetterSave - Backup History Popup
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "BackupHistoryManager.hpp"

using namespace geode::prelude;

class BackupHistoryPopup : public Popup<> {
protected:
    ScrollLayer* m_scrollLayer = nullptr;
    CCMenu* m_contentMenu = nullptr;
    
    bool setup() override;
    void loadHistory();
    void onSlotSelect(CCObject*);
    void onClearHistory(CCObject*);
    void onRefresh(CCObject*);
    
public:
    static BackupHistoryPopup* create();
    
    std::string m_selectedSlot;
};

