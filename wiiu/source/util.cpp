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

#include "util.hpp"

static int fsaFd = -1;
static int mcp_hook_fd = -1;
static void some_func(IOSError err, void *arg) { (void)arg; }

void servicesExit(void)
{
    Input::finalize();
    freeIcons();

    unmount_fs("storage_mlc");
    unmount_fs("storage_usb");
    unmount_fs("slccmpt");

    IOSUHAX_FSA_Close(fsaFd);

    try_shutdown_iosuhax();

    KeyboardManager::get().shutdown();

    nn::pdm::Finalize();

    Account::exit();

    SDLH_Exit();

    romfsExit();

    Logger::getInstance().flush();
}

int32_t servicesInit(void)
{
    io::createDirectory("wiiu");
    io::createDirectory("wiiu/Checkpoint");
    io::createDirectory("wiiu/Checkpoint/saves");

    Logger::getInstance().log(Logger::INFO, "Starting Checkpoint loading...");

    romfsInit();

    Configuration::getInstance();

    if (!SDLH_Init()) {
        Logger::getInstance().log(Logger::ERROR, "SDLH_Init failed");
        return -1;
    }

    if (!Account::init()) {
        Logger::getInstance().log(Logger::ERROR, "Account::init failed");
        return -1;
    }

    if (nn::pdm::Initialize() != 0) {
        Logger::getInstance().log(Logger::ERROR, "pdm::Initialize() failed");
        return -1;
    }

    if (!KeyboardManager::get().init()) {
        Logger::getInstance().log(Logger::ERROR, "Error initializing keyboard");
    }

    if (!try_init_iosuhax()) {
        Logger::getInstance().log(Logger::ERROR, "Cannot init IOSUHAX. Make sure CFW is running");
        return -1;
    }

    fsaFd = IOSUHAX_FSA_Open();

    // Wii U saves
    mount_fs("storage_mlc", fsaFd, NULL, "/vol/storage_mlc01");
    mount_fs("storage_usb", fsaFd, NULL, "/vol/storage_usb01");

    // vWii saves
    mount_fs("slccmpt", fsaFd, "/dev/slccmpt01", "/vol/storage_slccmpt");

    Logger::getInstance().log(Logger::INFO, "Checkpoint loading completed!");

    Input::initialize();

    return 0;
}

std::u16string StringUtils::UTF8toUTF16(const char* src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> converter;
    return converter.from_bytes(src);
}

// https://stackoverflow.com/questions/14094621/change-all-accented-letters-to-normal-letters-in-c
std::string StringUtils::removeAccents(std::string str)
{
    std::u16string src = UTF8toUTF16(str.c_str());
    const std::u16string illegal = UTF8toUTF16("ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüūýþÿ");
    const std::u16string fixed = UTF8toUTF16("AAAAAAECEEEEIIIIDNOOOOOx0UUUUYPsaaaaaaeceeeeiiiiOnooooo/0uuuuuypy");

    for (size_t i = 0, sz = src.length(); i < sz; i++) {
        size_t index = illegal.find(src[i]);
        if (index != std::string::npos) {
            src[i] = fixed[index];
        }
    }

    return UTF16toUTF8(src);
}

std::string StringUtils::removeNotAscii(std::string str)
{
    for (size_t i = 0, sz = str.length(); i < sz; i++) {
        if (!isascii(str[i])) {
            str[i] = ' ';
        }
    }
    return str;
}

extern "C" void ACPTurnOnDrcLed(uint32_t unk, uint32_t pattern);
#define ACP_DRC_LED_PATTERN_2 2

void blinkLed(uint8_t times)
{
    ACPTurnOnDrcLed(0, ACP_DRC_LED_PATTERN_2);
}

int MCPHookOpen(void)
{
	//take over mcp thread
	mcp_hook_fd = MCP_Open();
	if(mcp_hook_fd < 0)
		return -1;
	IOS_IoctlAsync(mcp_hook_fd, 0x62, (void*)0, 0, (void*)0, 0, some_func, (void*)0);
	//let wupserver start up
	OSSleepTicks(OSMillisecondsToTicks(500));
	if(IOSUHAX_Open("/dev/mcp") < 0)
		return -1;
	return 0;
}

void MCPHookClose(void)
{
	if(mcp_hook_fd < 0)
		return;
	//close down wupserver, return control to mcp
	IOSUHAX_Close();
	//wait for mcp to return
	OSSleepTicks(OSMillisecondsToTicks(500));
	MCP_Close(mcp_hook_fd);
	mcp_hook_fd = -1;
}

bool try_init_iosuhax()
{
	int res = IOSUHAX_Open(NULL);
	if (res < 0)
		res = MCPHookOpen();

    if (res < 0)
        return false;

    return true;
}

void try_shutdown_iosuhax()
{
    if(mcp_hook_fd >= 0)
        MCPHookClose();
    else
        IOSUHAX_Close();
}

int flushVolume(const char* volumePath)
{
    return IOSUHAX_FSA_FlushVolume(fsaFd, volumePath);
}

nn::act::SlotNo accountIdToSlotNo(uint32_t accountId)
{
    for (uint8_t i = 1; i <= 12; i++) {
        if (nn::act::GetPersistentIdEx(i) == accountId) {
            return i;
        }
    }

    return 0;
}