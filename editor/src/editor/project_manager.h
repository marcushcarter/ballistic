#pragma once
#include "pch.h"
#include "project_registry.h"

struct ProjectManager
{
    ProjectRegistry registry;

    VkDescriptorSet logoLongTextureID = VK_NULL_HANDLE;
    VkExtent2D logoLongExtent = {};

    char filterBuffer[256] = {};
    int selectedIndex = -1;
    int sortIndex = 0;
    int removeConfirmIndex = -1;

    char createNameBuffer[256] = {};
    char createPathBuffer[512] = {};
    bool createEditNow = true;
    int createVersionControlIndex = 0;

    int renameIndex = -1;
    char renameBuffer[256] = {};

    std::filesystem::path pendingOpenPath;
    bool openRequested = false;

    void Start(const std::filesystem::path& roamingRoot, VkDescriptorSet logoLongTex, VkExtent2D logoLongExtent);

    std::filesystem::path Draw();

    void DrawList();
    void DrawCreatePopup();
    void DrawRemovePopup();
    void DrawRenamePopup();
    void DrawSettingsPopup();
    void RequestOpen(int index);
};
