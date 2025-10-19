#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "BetterSaveLogger.hpp"

using namespace geode::prelude;

class LogsViewerPopup : public Popup<> {
protected:
    TextArea* m_logsArea;
    CCMenu* m_buttonMenu;

    bool setup() override;
    void onRefresh(CCObject*);
    void onClear(CCObject*);
    void loadLogs();

public:
    static LogsViewerPopup* create();
};

