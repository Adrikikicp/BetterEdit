#include "KeybindManager.hpp"
#include <algorithm>
#include <functional>

KeybindManager* g_manager;

bool Keybind::operator==(Keybind const& other) const {
    return
        other.key == this->key &&
        other.modifiers == this->modifiers;
}

bool Keybind::operator<(Keybind const& other) const {
    return this->key < other.key;
}

std::string Keybind::toString() const {
    std::string res = "";

    if (this->modifiers & kmControl)  res += "Ctrl + ";
    if (this->modifiers & kmAlt)   res += "Alt + ";
    if (this->modifiers & kmShift) res += "Shift + ";

    switch (this->key) {
        case KEY_None:
            res = res.substr(0, res.size() - 3);
            break;
        
        case KEY_C:
            // because for some reason keyToString thinks C is a V
            res += "C";
            break;
        
        case static_cast<enumKeyCodes>(-1):
            res += "Unk";
            break;
        
        default:
            res += cocos2d::CCDirector::sharedDirector()
                ->getKeyboardDispatcher()
                ->keyToString(this->key);
    }

    return res;
}

Keybind::Keybind() {
    this->key = KEY_None;
    this->modifiers = 0;
}

Keybind::Keybind(enumKeyCodes pressed) {
    this->key = pressed;

    auto kb = CCDirector::sharedDirector()->getKeyboardDispatcher();

    this->modifiers = 0;
    if (kb->getControlKeyPressed())
        this->modifiers |= this->kmControl;
    if (kb->getShiftKeyPressed())
        this->modifiers |= this->kmShift;
    if (kb->getAltKeyPressed())
        this->modifiers |= this->kmAlt;
}

Keybind::Keybind(enumKeyCodes key, Modifiers mods) {
    this->key = key;
    this->modifiers = mods;
}

Keybind::Keybind(enumKeyCodes key, int mods) {
    this->key = key;
    this->modifiers = static_cast<Modifiers>(mods);
}

std::size_t std::hash<Keybind>::operator()(Keybind const& key) const {
    return (key.key << 8) + (key.modifiers << 4) + (key.click);
}

bool KeybindCallback::operator==(KeybindCallback const& other) const {
    return this->id == other.id;
}

void KeybindManager::encodeDataTo(DS_Dictionary*) {
}

void KeybindManager::dataLoaded(DS_Dictionary*) {
}

void KeybindManager::firstLoad() {
}

bool KeybindManager::init() {
    if (!CCNode::init())
        return false;
    
    this->m_sFileName = "BEKeybindManager.dat";
    this->loadDefaultKeybinds();

    this->setup();
    
    return true;
}

bool KeybindManager::initGlobal() {
    g_manager = new KeybindManager;

    if (g_manager && g_manager->init())
        return true;

    CC_SAFE_DELETE(g_manager);
    return false;
}

void KeybindManager::loadDefaultKeybinds() {
    this->addPlayKeybind({ "Pause", [](PlayLayer* pl, EditorUI* ui, bool push) -> bool {
        if (!push) return false;
        if (ui) ui->onPause(nullptr);
        if (pl) pl->m_uiLayer->onPause(nullptr);
        return false;
    }}, {{ KEY_Escape, 0 }});
    
    this->addPlayKeybind({ "Jump P1", [](PlayLayer* pl, EditorUI* ui, bool push) -> bool {
        if (push) {
            if (ui) ui->m_pEditorLayer->pushButton(0, true);
            else pl->pushButton(0, true);
        } else {
            if (ui) ui->m_pEditorLayer->releaseButton(0, true);
            else pl->releaseButton(0, true);
        }
        return false;
    }}, {{ KEY_Space, 0 }});
    
    this->addPlayKeybind({ "Jump P2", [](PlayLayer* pl, EditorUI* ui, bool push) -> bool {
        if (push) {
            if (ui) ui->m_pEditorLayer->pushButton(0, false);
            else pl->pushButton(0, false);
        } else {
            if (ui) ui->m_pEditorLayer->releaseButton(0, false);
            else pl->releaseButton(0, false);
        }
        return false;
    }}, {{ KEY_Up, 0 }});

    this->addPlayKeybind({ "Place Checkpoint", [](PlayLayer* pl, bool push) -> bool {
        if (push) {
            pl->m_uiLayer->onCheck(nullptr);
        }
        return false;
    }}, {{ KEY_Z, 0 }});

    this->addPlayKeybind({ "Delete Checkpoint", [](PlayLayer* pl, bool push) -> bool {
        if (push) {
            pl->m_uiLayer->onDeleteCheck(nullptr);
        }
        return false;
    }}, {{ KEY_X, 0 }});

    this->addEditorKeybind({ "Build Mode", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleMode(ui->m_pBuildModeBtn);
        return false;
    }}, {{ KEY_One, 0 }});

    this->addEditorKeybind({ "Edit Mode", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleMode(ui->m_pEditModeBtn);
        return false;
    }}, {{ KEY_Two, 0 }});

    this->addEditorKeybind({ "Delete Mode", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleMode(ui->m_pDeleteModeBtn);
        return false;
    }}, {{ KEY_Three, 0 }});

    this->addEditorKeybind({ "Swipe modifier", [](EditorUI* ui) -> bool {
        return false;
    }}, {{ KEY_None, Keybind::kmShift }});

    this->addEditorKeybind({ "Move modifier", [](EditorUI* ui, bool push) -> bool {
        ui->m_bSpaceKeyPressed = push;
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->m_bMoveModifier = push;
        return false;
    }}, {{ KEY_Space, 0 }});

    this->addEditorKeybind({ "Rotate CCW", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->transformObjectCall(kEditCommandRotateCCW);
        return false;
    }}, {{ KEY_Q, 0 }});

    this->addEditorKeybind({ "Rotate CW", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->transformObjectCall(kEditCommandRotateCW);
        return false;
    }}, {{ KEY_E, 0 }});

    this->addEditorKeybind({ "Flip X", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->transformObjectCall(kEditCommandFlipX);
        return false;
    }}, {{ KEY_Q, Keybind::kmAlt }});

    this->addEditorKeybind({ "Flip Y", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->transformObjectCall(kEditCommandFlipY);
        return false;
    }}, {{ KEY_E, Keybind::kmAlt }});

    this->addEditorKeybind({ "Delete Selected", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onDeleteSelected(nullptr);
        return false;
    }}, {{ KEY_E, Keybind::kmAlt }});

    this->addEditorKeybind({ "Undo", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->undoLastAction(nullptr);
        return false;
    }}, {{ KEY_Z, Keybind::kmControl }});

    this->addEditorKeybind({ "Redo", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->redoLastAction(nullptr);
        return false;
    }}, {{ KEY_Z, Keybind::kmControl | Keybind::kmShift }});

    this->addEditorKeybind({ "Deselect", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->deselectAll();
        return false;
    }}, {{ KEY_D, Keybind::kmAlt }});

    this->addEditorKeybind({ "Copy", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onCopy(nullptr);
        return false;
    }}, {{ KEY_C, Keybind::kmControl }});

    // this->addEditorKeybind({ "Cut", [](EditorUI* ui) -> bool {
    //     return false;
    // }}, {{ KEY_X, Keybind::kmControl }});

    this->addEditorKeybind({ "Paste", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onPaste(nullptr);
        return false;
    }}, {{ KEY_V, Keybind::kmControl }});

    this->addEditorKeybind({ "Duplicate", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onDuplicate(nullptr);
        return false;
    }}, {{ KEY_D, Keybind::kmControl }});

    this->addEditorKeybind({ "Rotate", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleEnableRotate(nullptr);
        return false;
    }}, {{ KEY_R, 0 }});

    this->addEditorKeybind({ "Free Move", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleFreeMove(nullptr);
        return false;
    }}, {{ KEY_F, 0 }});

    this->addEditorKeybind({ "Swipe", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleSwipe(nullptr);
        return false;
    }}, {{ KEY_T, 0 }});

    this->addEditorKeybind({ "Snap", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->toggleSnap(nullptr);
        return false;
    }}, {{ KEY_G, 0 }});

    this->addEditorKeybind({ "Playtest", [](EditorUI* ui) -> bool {
        if (ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onStopPlaytest(nullptr);
        else
            ui->onPlaytest(nullptr);

        return false;
    }}, {{ KEY_Enter, 0 }});

    this->addEditorKeybind({ "Playback Music", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onPlayback(nullptr);
        return false;
    }}, {{ KEY_Enter, Keybind::kmControl }});

    this->addEditorKeybind({ "Previous Build Tab", [](EditorUI* ui) -> bool {
        if (ui->m_pEditorLayer->m_bIsPlaybackMode)
            return false;
        
        auto t = ui->m_nSelectedTab - 1;
        if (t < 0)
            t = ui->m_pTabsArray->count() - 1;
        ui->selectBuildTab(t);
        return false;
    }}, {{ KEY_F1, 0 }});

    this->addEditorKeybind({ "Next Build Tab", [](EditorUI* ui) -> bool {
        if (ui->m_pEditorLayer->m_bIsPlaybackMode)
            return false;

        auto t = ui->m_nSelectedTab + 1;
        if (t > static_cast<int>(ui->m_pTabsArray->count() - 1))
            t = 0;
        ui->selectBuildTab(t);
        return false;
    }}, {{ KEY_F2, 0 }});

    this->addEditorKeybind({ "Next Group", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onGroupUp(nullptr);
        return false;
    }}, {{ KEY_Right, 0 }});

    this->addEditorKeybind({ "Previous Group", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->onGroupDown(nullptr);
        return false;
    }}, {{ KEY_Left, 0 }});

    this->addEditorKeybind({ "Object Left", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandLeft);
        return false;
    }}, {{ KEY_A, 0 }});

    this->addEditorKeybind({ "Object Right", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandRight);
        return false;
    }}, {{ KEY_D, 0 }});

    this->addEditorKeybind({ "Object Up", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandUp);
        return false;
    }}, {{ KEY_W, 0 }});

    this->addEditorKeybind({ "Object Down", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandDown);
        return false;
    }}, {{ KEY_S, 0 }});

    this->addEditorKeybind({ "Object Left Small", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandSmallLeft);
        return false;
    }}, {{ KEY_A, Keybind::kmShift }});

    this->addEditorKeybind({ "Object Right Small", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandSmallRight);
        return false;
    }}, {{ KEY_D, Keybind::kmShift }});

    this->addEditorKeybind({ "Object Up Small", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandSmallUp);
        return false;
    }}, {{ KEY_W, Keybind::kmShift }});

    this->addEditorKeybind({ "Object Down Small", [](EditorUI* ui) -> bool {
        if (!ui->m_pEditorLayer->m_bIsPlaybackMode)
            ui->moveObjectCall(kEditCommandSmallDown);
        return false;
    }}, {{ KEY_S, Keybind::kmShift }});
}

std::vector<KeybindCallback*> const& KeybindManager::getCallbacks(KeybindType type) {
    return m_mCallbacks[type];
}

KeybindList KeybindManager::getKeybindsForCallback(
    KeybindType type, KeybindCallback* cb
) {
    std::set<Keybind> res;

    for (auto & [key, vals] : m_mKeybinds) {
        for (auto & val : vals)
            if (val.type == type && val.bind == cb)
                res.insert(key);
    }

    return res;
}

size_t KeybindManager::getIndexOfCallback(KeybindType type, KeybindCallback* cb) {
    size_t ix = 0;
    for (auto const& cb_ : this->m_mCallbacks[type]) {
        if (cb_ == cb)
            return ix;
        
        ix++;
    }
    return 0;
}

void KeybindManager::addCallback(
    KeybindCallback* cb,
    KeybindType type,
    KeybindList const& binds
) {
    auto index = m_mCallbacks[type].size();

    cb->id = m_mCallbacks[type].size();
    cb->defaults = binds;

    m_mCallbacks[type].push_back(cb);

    for (auto bind : binds) {
        this->addKeybind(type, cb, bind);
    }
}

void KeybindManager::addEditorKeybind(
    KeybindEditor cb,
    KeybindList const& binds
) {
    this->addCallback(new KeybindEditor(cb), kKBEditor, binds);
}

void KeybindManager::addPlayKeybind(
    KeybindPlayLayer cb,
    KeybindList const& binds
) {
    this->addCallback(new KeybindPlayLayer(cb), kKBPlayLayer, binds);
}

void KeybindManager::addGlobalKeybind(
    KeybindGlobal cb,
    KeybindList const& binds
) {
    this->addCallback(new KeybindGlobal(cb), kKBGlobal, binds);
}

void KeybindManager::addKeybind(KeybindType type, KeybindCallback* cb, Keybind const& bind) {
    if (m_mKeybinds.count(bind) && m_mKeybinds[bind].size())
        m_mKeybinds[bind].push_back({ type, cb });
    else
        m_mKeybinds[bind] = {{ type, cb }};
}

void KeybindManager::removeKeybind(KeybindType type, KeybindCallback* cb, Keybind const& bind) {
    if (m_mKeybinds.count(bind) && m_mKeybinds[bind].size()) {
        std::vector<KeybindManager::Target>::iterator iter;
        for (iter = m_mKeybinds[bind].begin(); iter != m_mKeybinds[bind].end(); ) {
            if (iter->bind == cb)
                iter = m_mKeybinds[bind].erase(iter);
            else
                iter++;
        }
    }
}

void KeybindManager::editKeybind(KeybindType type, KeybindCallback* cb, Keybind const& old, Keybind const& bind) {
    this->removeKeybind(type, cb, old);
    this->addKeybind(type, cb, bind);
}

void KeybindManager::clearKeybinds(KeybindType type, KeybindCallback* cb) {
    for (auto & [key, vals] : m_mKeybinds)
        for (auto & val : vals)
            if (val.type == type && val.bind == cb)
                m_mKeybinds.erase(key);
}

void KeybindManager::executeEditorCallbacks(Keybind const& bind, EditorUI* ui, bool keydown) {
    if (!m_mKeybinds.count(bind))
        return;

    for (auto & target : m_mKeybinds[bind]) {
        switch (target.type) {
            case kKBEditor: {
                auto c = as<KeybindEditor*>(target.bind);
                if (c->call_b)
                    c->call_b(ui, keydown);
                else if (keydown)
                    c->call(ui);
            } break;
            
            case kKBPlayLayer: {
                if (as<KeybindPlayLayer*>(target.bind)->editor)
                    as<KeybindPlayLayer*>(target.bind)->call_e(nullptr, ui, keydown);
            } break;
        }
    }
}

void KeybindManager::executePlayCallbacks(Keybind const& bind, PlayLayer* pl, bool keydown) {
    if (!m_mKeybinds.count(bind))
        return;

    for (auto & target : m_mKeybinds[bind]) {
        switch (target.type) {
            case kKBPlayLayer: {
                auto c = as<KeybindPlayLayer*>(target.bind);
                if (c->call_e)
                    c->call_e(pl, nullptr, keydown);
                else
                    c->call(pl, keydown);
            } break;
        }
    }
}

void KeybindManager::resetToDefault(KeybindType type, KeybindCallback* cb) {
    this->clearKeybinds(type, cb);
    for (auto const& bind : cb->defaults)
        this->addKeybind(type, cb, bind);
}

void KeybindManager::resetAllToDefaults() {
    for (auto const& [type, cbs] : m_mCallbacks)
        for (auto const& cb : cbs)
            this->resetToDefault(type, cb);
}

KeybindManager* KeybindManager::get() {
    return g_manager;
}

