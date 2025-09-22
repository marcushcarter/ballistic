#include "BEngine/engine_editor.hpp"

namespace BE {
    
Editor::Editor(Engine* enginePtr) : engine(enginePtr) {
    IMGUI_CHECKVERSION();
    
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx);

    ImPlotContext* ctxpl = ImPlot::CreateContext();
    ImPlot::SetCurrentContext(ctxpl);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(engine->getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");

    meshPreviewFB.resize(128, 128);
}

Editor::~Editor() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::beginFrame() {
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // ImGui::DockSpaceOverViewport(
    //     ImGui::GetMainViewport(),
    //     ImGuiDockNodeFlags_PassthruCentralNode
    // );

    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.0f, 0.2f, 1.0f));

    window_flags |= 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Dockspace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
    ImGui::End();
}

void Editor::showPanels() {

    Menu();
    Viewport();
    Heirarchy();
    Resources();
    Inspector();

    // === PERFORMANCE == //

    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Performance Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking );
    ImGui::Text("ENGINE FPS %.1f MS %.2f", engine->frameTime.fps, engine->frameTime.ms);
    ImGui::Text("IMGUI FPS %.1f MS %.2f", ImGui::GetIO().Framerate, 1000.0f/ImGui::GetIO().Framerate);
    ImGui::Text("SELECTED ANCHOR %d", selectedAnchor);
    ImGui::End();
}

void Editor::endFrame() {

    ImGui::Render();
    glViewport(0, 0, engine->width, engine->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void Editor::Frame() {
    
    beginFrame();
    showPanels();
    endFrame();
}

// === Panels === //

void Editor::Menu() {

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { engine->closeWindow(); }
            ImGui::EndMenu(); 
        };
        if (ImGui::BeginMenu("Edit")) {
            // if (ImGui::MenuItem("Add Scene")) { std::string label = "Scene" + std::to_string(engine->scenes.size()+1); engine->addScene(label); }
            if (ImGui::MenuItem("Add Camera")) { std::string label = "Camera" + std::to_string(engine->activeScene->cameras.size()+1); engine->activeScene->addCamera(label); }
            if (ImGui::MenuItem("Add Light")) { std::string label = "Light" + std::to_string(engine->activeScene->lights().lights.size()+1);  engine->activeScene->lights().addLight(label, 1); }
            if (ImGui::MenuItem("Create Anchor")) { selectedAnchor = engine->activeScene->createAnchor(); }
            ImGui::EndMenu(); 
        };
        if (ImGui::BeginMenu("View")) { ImGui::EndMenu(); };
        if (ImGui::BeginMenu("Window")) { ImGui::EndMenu(); };
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Recompile Shaders")) { engine->resources().recompileShaders(); }
            if (ImGui::MenuItem("Update GPU")) { engine->viewport->scene->lights().updateGPU(); }
            ImGui::EndMenu(); 
        };
        ImGui::EndMainMenuBar();
    }

}

void Editor::Viewport() {

    ImGui::Begin("Hello, ImGui!");
    static ImVec2 lastSize = ImGui::GetWindowSize();
    ImVec2 size = ImGui::GetContentRegionAvail();
    bool resizing = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    if (!resizing) engine->viewport.get()->resize(size.x/2, size.y/2);
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) engine->activeScene->activeCamera->handleInputs(engine->getWindow(), engine->frameTime.dt);
    ImGui::Image((void*)(intptr_t) engine->viewport.get()->framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void Editor::Heirarchy() {
    
    ImGui::Begin("Heirarchy");
    for (Anchor a : engine->activeScene->anchors) {
        std::string name = "Anchor" + std::to_string(a);
        if (ImGui::Button(name.c_str())) selectedAnchor = a;
    }
    ImGui::End();
}

void Editor::Resources() {
    
    ImGui::Begin("Resources");

    static char searchBuffer[128] = "";

    float buttonSize = ImGui::GetFrameHeight();
    float padding = ImGui::GetStyle().ItemSpacing.x * 2;
    float searchWidth = ImGui::GetContentRegionAvail().x - (buttonSize + padding);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushItemWidth(searchWidth);
    ImGui::InputTextWithHint("##SearchResources", "Search...", searchBuffer, sizeof(searchBuffer));
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    if (ImGui::Button("+", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()))) {
        ImGui::OpenPopup("AddResourcePopup");
    }
    ImGui::PopStyleVar();

    ImGui::Separator();

    if (ImGui::BeginPopup("AddResourcePopup")) {
        if (ImGui::MenuItem("Add Mesh")) {}
        if (ImGui::MenuItem("Add Shader")) {}
        if (ImGui::MenuItem("Add Texture")) {}
        if (ImGui::MenuItem("Add Material")) {}
        ImGui::EndPopup();
    }

    auto MatchesSearch = [&](const std::string& name) {
        return strlen(searchBuffer) == 0 || name.find(searchBuffer) != std::string::npos;
    };

    ImGui::SeparatorText("Meshes");
    for (auto& [key, mesh] : engine->resources().meshes) {
        if (!MatchesSearch(key) && !MatchesSearch("Meshes")) continue;

        if (ImGui::Selectable(key.c_str())) {}
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("MESH", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    ImGui::SeparatorText("Materials");
    for (auto& [key, material] : engine->resources().materials) {
        if (!MatchesSearch(key) && !MatchesSearch("Materials")) continue;

        if (ImGui::Selectable(key.c_str())) {}
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("MATERIAL", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    ImGui::SeparatorText("Shaders");
    for (auto& [key, shader] : engine->resources().shaders) {
        if (!MatchesSearch(key) && !MatchesSearch("Shaders")) continue;

        if (ImGui::Selectable(key.c_str())) {}
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("SHADER", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    ImGui::SeparatorText("Textures");
    for (auto& [key, texture] : engine->resources().textures) {
        if (!MatchesSearch(key) && !MatchesSearch("Textures")) continue;

        if (ImGui::Selectable(key.c_str())) {}
        // ImGui::Image((void*)(intptr_t) texture->ID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("TEXTURE", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }

    ImGui::End();
}

void Editor::Inspector() {
    
    ImGui::Begin("Inspector");
    if (selectedAnchor != -1) {

        float avialWidth = ImGui::GetContentRegionAvail().x;

        // TRANSFORM COMPONENT

        if (engine->activeScene->registry.transforms.find(selectedAnchor) != engine->activeScene->registry.transforms.end()) {
            TransformComponent& t = engine->activeScene->registry.transforms[selectedAnchor];
            if (ImGui::CollapsingHeader("Transform")) {
                ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
                ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotation), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f);
                ImGui::Separator();
            }
        } else {
            if (ImGui::Button("Add Transform Component")) {
                engine->activeScene->registry.transforms[selectedAnchor] = TransformComponent{{0,0,0}, {0,0,0}, {1,1,1}};
            }
        }

        // MESH COMPONENT

        if (engine->activeScene->registry.meshes.find(selectedAnchor) != engine->activeScene->registry.meshes.end()) {
            MeshComponent& m = engine->activeScene->registry.meshes[selectedAnchor];
            if (ImGui::CollapsingHeader("Material")) {
                
                static glm::vec2 rotation(0.0f);
                
                if (m.mesh != nullptr) m.mesh->makePreview(meshPreviewFB, *engine->resources().shaders["__mesh_preview"].get(), rotation);

                if (ImGui::BeginChild("Section", ImVec2(0, 0), true)) {

                    if (m.mesh) {
                        ImGui::Text("Mesh Preview:");
                        ImGui::Image((void*)(intptr_t)meshPreviewFB.texture, ImVec2(avialWidth-10, avialWidth-10), ImVec2(0, 1), ImVec2(1, 0));
                        
                        static bool draggingMesh = false;
                        static ImVec2 lastMousePos;

                        ImGuiIO& io = ImGui::GetIO();
                        ImVec2 imageMin = ImGui::GetItemRectMin();
                        ImVec2 imageMax = ImGui::GetItemRectMax(); 

                        bool isHovering = io.MousePos.x >= imageMin.x && io.MousePos.x <= imageMax.x && io.MousePos.y >= imageMin.y && io.MousePos.y <= imageMax.y;

                        if (!draggingMesh && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            if (isHovering) {
                                draggingMesh = true;
                                lastMousePos = io.MousePos;
                            }
                        }

                        if (draggingMesh) {
                            glm::vec2 delta(io.MousePos.x - lastMousePos.x, io.MousePos.y - lastMousePos.y);

                            float sensitivity = 0.5f;
                            rotation.x += delta.x * sensitivity;
                            rotation.y += delta.y * sensitivity;

                            if (rotation.y > 89.0f) rotation.y = 89.0f;
                            if (rotation.y < -89.0f) rotation.y = -89.0f;
                            
                            lastMousePos = io.MousePos;

                            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                                draggingMesh = false;
                            }
                        }

                        ImGui::Text("Vertices: %d", m.mesh->vertices.size());
                        ImGui::Text("Indices: %d", m.mesh->indices.size());
                        ImGui::Text("Triangles: %d", m.mesh->indices.size()/3);
                        if (ImGui::Button("Remove Mesh")) { m.mesh = nullptr; }
                    } else {
                        ImGui::Text("Drag in a Mesh");
                    }
                
                    ImGui::Separator();

                    if (m.material) {

                        if (m.material->diffuseMap) ImGui::Image((void*)(intptr_t)m.material->diffuseMap->ID, ImVec2(avialWidth/4, avialWidth/4), ImVec2(0, 1), ImVec2(1, 0));
                        else ImGui::Dummy(ImVec2(avialWidth/4, avialWidth/4));
                        ImGui::SameLine();

                        if (m.material->normalMap) ImGui::Image((void*)(intptr_t)m.material->normalMap->ID, ImVec2(avialWidth/4, avialWidth/4), ImVec2(0, 1), ImVec2(1, 0));
                        else ImGui::Dummy(ImVec2(avialWidth/4, avialWidth/4));
                        ImGui::SameLine();
                        
                        if (m.material->roughnessMap) ImGui::Image((void*)(intptr_t)m.material->roughnessMap->ID, ImVec2(avialWidth/4, avialWidth/4), ImVec2(0, 1), ImVec2(1, 0));
                        else ImGui::Dummy(ImVec2(avialWidth/4, avialWidth/4));

                        ImGui::Text("Material Properties:");
                        ImGui::BeginDisabled(true);
                        if (ImGui::SliderFloat("Metallic", &m.material->metallic, 0.0f, 1.0f)) {}
                        if (ImGui::SliderFloat("Roughness", &m.material->roughness, 0.0f, 1.0f)) {}
                        if (ImGui::DragFloat3("Diffuse Color",&m.material->diffuseColor.x, 0.01f, 0.0f, 1.0f));
                        if (ImGui::Checkbox("Cull Faces?", &m.material->cull)) {}
                        if (ImGui::Checkbox("Transparent?", &m.material->transparent)) {}
                        ImGui::EndDisabled();
                        
                        if (ImGui::Button("Remove Material")) { m.material = nullptr; }

                    } else {
                        ImGui::Text("Drag in a Material");
                    }

                    ImGui::Separator();

                    if (m.shader) {
                        if (ImGui::Button("Remove Shader")) { m.shader = nullptr; }
                    } else {
                        ImGui::Text("Drag in a Shader");
                    }
            
                    ImGui::EndChild();
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* meshPayload = ImGui::AcceptDragDropPayload("MESH")) {
                        const char* meshName = (const char*)meshPayload->Data;
                        auto it = engine->resources().meshes.find(meshName);
                        if (it != engine->resources().meshes.end()) {
                            m.mesh = it->second;
                            rotation = glm::vec2(0, 0);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* materialPayload = ImGui::AcceptDragDropPayload("MATERIAL")) {
                        const char* materialName = (const char*)materialPayload->Data;
                        auto it = engine->resources().materials.find(materialName);
                        if (it != engine->resources().materials.end()) {
                            m.material = it->second;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* shaderPayload = ImGui::AcceptDragDropPayload("SHADER")) {
                        const char* shaderName = (const char*)shaderPayload->Data;
                        auto it = engine->resources().shaders.find(shaderName);
                        if (it != engine->resources().shaders.end()) {
                            m.shader = it->second;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

            }
        } else {
            if (ImGui::Button("Add Mesh Component")) {
                engine->activeScene->registry.meshes[selectedAnchor] = MeshComponent{engine->resources().meshes["__cube"], engine->resources().materials["__default_material"], engine->resources().shaders["__scene"]};
            }
        }
    
    }
    ImGui::End();
}

}; // BE namespace