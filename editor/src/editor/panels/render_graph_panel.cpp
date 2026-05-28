#include "render_graph_panel.h"
#include "../editor_context.h"

void RenderGraphPanel::OnProjectOpened(const std::filesystem::path& path)
{
    // TODO: ImNodes::EditorContextCreate + LoadCurrentEditorStateFromIniFile
    (void)path;
}

void RenderGraphPanel::OnProjectClosed(const std::filesystem::path& path)
{
    // TODO: Save then ImNodes::EditorContextFree
    (void)path;
}

void RenderGraphPanel::Save(const std::filesystem::path& path)
{
    // TODO: ImNodes::SaveCurrentEditorStateToIniFile
    (void)path;
}

void RenderGraphPanel::Draw(EditorContext& ctx)
{
    // TODO: move from Editor::DrawRenderGraphPanel
    (void)ctx;
}
