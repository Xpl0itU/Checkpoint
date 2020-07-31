#ifndef ACCOUNTSELECTOVERLAY_HPP
#define ACCOUNTSELECTOVERLAY_HPP

#include "Overlay.hpp"
#include "SDLHelper.hpp"
#include "clickable.hpp"
#include "colors.hpp"
#include "hid.hpp"
#include "input.hpp"
#include "account.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Clickable;

class AccountSelectOverlay : public Overlay {
public:
    AccountSelectOverlay(Screen& screen, const std::function<void(AccountUid)>& callbackSelectedUser, const std::function<void()>& callbackCancel);
    ~AccountSelectOverlay(void) {}
    void draw(void) const override;
    void update(touchPosition* touch) override;

private:
    Hid<HidDirection::HORIZONTAL, HidDirection::HORIZONTAL> hid;
    std::vector<User> users;
    std::function<void(AccountUid)> select;
    std::function<void()> cancel;
};

#endif
