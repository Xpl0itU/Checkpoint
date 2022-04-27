/*
 *   This file is part of Checkpoint
 *   Copyright (C) 2017-2019 Bernardo Giordano, FlagBrew
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
 *       * Requiring preservation of specified reasonable legal notices or
 *         author attributions in that material or in the Appropriate Legal
 *         Notices displayed by works containing it.
 *       * Prohibiting misrepresentation of the origin of that material,
 *         or requiring that modified versions of such material be marked in
 *         reasonable ways as different from the original version.
 */

#include "account.hpp"

static std::map<AccountUid, User> mUsers;

bool Account::init(void)
{
    return nn::act::Initialize().IsSuccess();
}

void Account::exit(void)
{
    for (auto& value : mUsers) {
        if (value.second.icon) {
            SDL_DestroyTexture(value.second.icon);
        }
    }
    nn::act::Finalize();
}

std::vector<AccountUid> Account::ids(void)
{
    std::vector<AccountUid> v;
    for (auto& value : mUsers) {
        v.push_back(value.second.id);
    }
    return v;
}

User Account::getUser(AccountUid id)
{
    User user{id, "", "", NULL};

    nn::act::SlotNo slotNo = accountIdToSlotNo(id);
    if (slotNo != 0) {
        return getUserFromSlot(slotNo);
    }

    return user;
}

User Account::getUserFromSlot(nn::act::SlotNo slot)
{
    User user{0, "", "", NULL};

    if (nn::act::IsSlotOccupied(slot)) {
        user.id = nn::act::GetPersistentIdEx(slot);

        // Get the mii name because not every account has a NNID linked
        int16_t miiName[nn::act::MiiNameSize];
        nn::act::GetMiiNameEx(miiName, slot);

        // convert to string
        std::u16string miiNameString((char16_t*) miiName, nn::act::MiiNameSize);
        user.name = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(miiNameString);

        user.shortName = trimToFit(user.name, 96 - g_username_dotsize * 2, 13);

        // load icon
        uint8_t* buffer = (uint8_t*) malloc(MAX_IMAGE_SIZE);
        size_t image_size = 0;
        nn::act::GetMiiImageEx(&image_size, buffer, MAX_IMAGE_SIZE, 0, slot);
        if (image_size > 0) {
            SDLH_LoadImage(&user.icon, buffer, image_size, true);
        }
        free(buffer);
    }

    return user;
}

std::string Account::username(AccountUid id)
{
    std::map<AccountUid, User>::const_iterator got = mUsers.find(id);
    if (got == mUsers.end()) {
        User user = getUser(id);
        mUsers.insert({id, user});
        return user.name;
    }

    return got->second.name;
}

std::string Account::shortName(AccountUid id)
{
    std::map<AccountUid, User>::const_iterator got = mUsers.find(id);
    if (got == mUsers.end()) {
        User user = getUser(id);
        mUsers.insert({id, user});
        return user.shortName;
    }

    return got->second.shortName;
}

SDL_Texture* Account::icon(AccountUid id)
{
    std::map<AccountUid, User>::const_iterator got = mUsers.find(id);
    if (got == mUsers.end()) {
        User user = getUser(id);
        mUsers.insert({id, user});
        return user.icon;
    }
    return got->second.icon;
}