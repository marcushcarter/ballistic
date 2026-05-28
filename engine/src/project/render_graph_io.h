#pragma once
#include "pch.h"
#include "render_graph.h"

std::string RGFormatToString(VkFormat format);
VkFormat RGStringToFormat(const std::string& s);

bool LoadRenderGraph(const std::filesystem::path& blstPath, std::vector<RGImage>& outImages);
bool SaveRenderGraph(const std::filesystem::path& blstPath, const std::vector<RGImage>& images);