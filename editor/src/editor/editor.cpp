#include "editor.h"
#include "workspace.h"
#include "project/project.h"
#include "graphics/renderer.h"
#include "utils/file_dialog.h"

void Editor::OpenProject(const std::filesystem::path& path)
{
    std::filesystem::create_directories(path / ".ballistic" / "editor");
    
    activeIniPath = (path / ".ballistic" / "editor" / "editor.ini").string();
    ImGui::GetIO().IniFilename = activeIniPath.c_str();
    if (std::filesystem::exists(activeIniPath)) {
        ImGui::LoadIniSettingsFromDisk(activeIniPath.c_str());
    } else {
        // ImGui::LoadIniSettingsFromMemory(DEFAULT_EDITOR_INI, strlen(DEFAULT_EDITOR_INI));
    }

    renderGraphContext = ImNodes::EditorContextCreate();
    ImNodes::EditorContextSet(renderGraphContext);
    std::string nodesPath = (path / ".ballistic/editor/nodes_rendergraph.ini").string();
    if (std::filesystem::exists(nodesPath)) {
        ImNodes::LoadCurrentEditorStateFromIniFile(nodesPath.c_str());
    } else  {

    }
}

void Editor::CloseProject(const std::filesystem::path& path)
{
    if (!activeIniPath.empty()) {
        ImGui::SaveIniSettingsToDisk(activeIniPath.c_str());
        ImGui::GetIO().IniFilename = nullptr;
        activeIniPath.clear();
    }

    if (renderGraphContext) {
        ImNodes::EditorContextSet(renderGraphContext);
        ImNodes::SaveCurrentEditorStateToIniFile((path / ".ballistic" / "editor" / "nodes_rendergraph.ini").string().c_str());
        ImNodes::EditorContextFree(renderGraphContext);
        renderGraphContext = nullptr;
    }
}

void Editor::SaveLayout(const std::filesystem::path& path)
{
    if (!activeIniPath.empty()) {
        ImGui::SaveIniSettingsToDisk(activeIniPath.c_str());
    }

    if (renderGraphContext) {
        ImNodes::EditorContextSet(renderGraphContext);
        ImNodes::SaveCurrentEditorStateToIniFile((path / ".ballistic" / "editor" / "nodes_rendergraph.ini").string().c_str());
    }
}

void Editor::SaveProjectAndLayout(EditorContext& ctx)
{
    ctx.project.Save();
    SaveLayout(ctx.project.path);
    ctx.workspace.TouchProject(ctx.project.path);
}

void Editor::Update(EditorContext& ctx)
{
    if (!ctx.project.IsOpen()) return;
 
    autosaveTimer += ImGui::GetIO().DeltaTime;
    if (ctx.workspace.config.autosaveEnabled && autosaveTimer >= ctx.workspace.config.autosaveInterval) {
        autosaveTimer = 0.0f;
        SaveProjectAndLayout(ctx);
        LOG_DEBUG("Autosaved project");
    }
}

void Editor::Draw(EditorContext& ctx)
{
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imguiViewport->Pos);
    ImGui::SetNextWindowSize(imguiViewport->Size);
    ImGui::SetNextWindowViewport(imguiViewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr,
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground
    );
    ImGui::PopStyleVar();
    ImGui::DockSpace(ImGui::GetID("MainDockSpace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
    
    viewport.Draw(ctx);
    DrawProjectPanel(ctx);
    DrawRenderGraphPanel();
}

void Editor::DrawProjectPanel(EditorContext& ctx)
{
    Project& project = ctx.project;

    if (ImGui::Begin("Project")) {
        ImGui::TextDisabled("%s", project.path.string().c_str());
        ImGui::Separator();
        if (ImGui::Button(ICON_FA_HOUSE " Project Manager")) {
            SaveProjectAndLayout(ctx);
            ctx.requestCloseProject = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save")) {
            SaveProjectAndLayout(ctx);
        }
    }
    ImGui::End();
}

void Editor::DrawRenderGraphPanel()
{
    if (ImGui::Begin("Render Graph", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImNodes::EditorContextSet(renderGraphContext);
        ImNodes::BeginNodeEditor();

        ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = nullptr;
        ImNodes::GetStyle().Flags = ImNodesStyleFlags_NodeOutline | ImNodesStyleFlags_GridLines;

        ImNodesIO& io = ImNodes::GetIO();
        io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

        ImNodes::BeginNode(1);
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("GBufferPass");
            ImNodes::EndNodeTitleBar();
            ImNodes::BeginOutputAttribute(2);
            ImGui::Text("GBuffer_Albedo");
            ImNodes::EndOutputAttribute();
            ImNodes::BeginOutputAttribute(3);
            ImGui::Text("GBuffer_Depth");
            ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        ImNodes::BeginNode(4);
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("LightingPass");
            ImNodes::EndNodeTitleBar();
            ImNodes::BeginInputAttribute(5);
            ImGui::Text("GBuffer_Albedo");
            ImNodes::EndInputAttribute();
            ImNodes::BeginInputAttribute(6);
            ImGui::Text("GBuffer_Depth");
            ImNodes::EndInputAttribute();
            ImNodes::BeginOutputAttribute(7);
            ImGui::Text("HdrLight");
            ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        ImNodes::Link(1, 2, 5);
        ImNodes::Link(2, 3, 6);

        ImNodes::MiniMap();
        // ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();
    }
    ImGui::End();
}
