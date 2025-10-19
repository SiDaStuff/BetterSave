#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <functional>

using namespace geode::prelude;

class ProgressPopup : public Popup<> {
protected:
    CCLabelBMFont* m_titleLabel;
    CCLabelBMFont* m_statusLabel;
    CCLabelBMFont* m_progressLabel;
    CCLabelBMFont* m_warningLabel;
    Slider* m_progressBar = nullptr;
    bool m_canClose = false;
    bool m_fullscreenMode = false;
    CCLayerColor* m_blackBackground = nullptr;
    std::vector<CCNode*> m_hiddenLayers;
    
    bool setup() override;
    void onClose(CCObject* sender) override;
    
    std::string m_title;
    
    void hideAllLayers();
    void restoreAllLayers();
    
public:
    static ProgressPopup* create(const std::string& title, bool fullscreen = false);
    void setTitle(const std::string& title);
    
    void setStatus(const std::string& message, ccColor3B color = {255, 255, 255});
    void setProgress(int current, int total);
    void allowClose(bool allow);
    void enableCloseButton();
    void setFullscreenMode(bool fullscreen);
    void closePopup();
};

