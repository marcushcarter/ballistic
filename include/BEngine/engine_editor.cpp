#include "BEngine/engine_editor.hpp"

#include <iostream>

namespace BE {
    
Editor::Editor(Engine* enginePtr) 
    : engine(enginePtr), meshPreview(1, 1, {{ GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, true }} ) {
    
    IMGUI_CHECKVERSION();
    
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx);

    ImPlotContext* ctxpl = ImPlot::CreateContext();
    ImPlot::SetCurrentContext(ctxpl);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.IniFilename = nullptr;
    // ImGui::LoadIniSettingsFromMemory(BE::Default::ini.c_str(), BE::Default::ini.size());
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(engine->getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");

    currentGizmoOperation = ImGuizmo::TRANSLATE;
    currentGizmoMode = ImGuizmo::WORLD;

    std::vector<AttachmentDesc> descriptors = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, true }, 
        { GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, false }
    };
    meshPreview.recreate(descriptors);
    meshPreview.resize(128, 128);

    // defaults

    BE::Anchor cube = engine->activeScene->createAnchor();
    engine->activeScene->registry.tags[cube] = BE::NameComponent{"cube", BE::AnchorType::None};
    engine->activeScene->registry.transforms[cube] = BE::TransformComponent{{0,0,0}, {0,0,0}, {1,1,1}};
    engine->activeScene->registry.meshes[cube] = BE::MeshComponent{engine->resources().meshes["default_cube"], nullptr, nullptr};
    
    // BE::Anchor skybox = engine->activeScene->createAnchor();
    // engine->activeScene->registry.tags[skybox] = BE::NameComponent{"skybox", BE::AnchorType::None};
    // engine->activeScene->registry.transforms[skybox] = BE::TransformComponent{{0,0,0}, {0,0,0}, {10,10,10}};
    // engine->activeScene->registry.meshes[skybox] = BE::MeshComponent{engine->resources().meshes["default_cube"], nullptr, nullptr};

    BE::Anchor light = engine->activeScene->createAnchor();
    engine->activeScene->registry.tags[light] = BE::NameComponent{"light", BE::AnchorType::None};
    engine->activeScene->registry.transforms[light] = BE::TransformComponent{{0,1.3f,0}, {0,0,0}, {0.1,0.1,0.1}};
    engine->activeScene->registry.meshes[light] = BE::MeshComponent{engine->resources().meshes["default_cube"], nullptr, nullptr};
    engine->activeScene->registry.lights[light] = BE::LightComponent{glm::vec3(1,1,1), 1.0f, 1};

    BE::Anchor camera = engine->activeScene->createAnchor();
    engine->activeScene->registry.tags[camera] = BE::NameComponent{"camera", BE::AnchorType::None};
    engine->activeScene->registry.transforms[camera] = BE::TransformComponent{{-3,3,3}, {-0.6f,-0.8f,0}, {1,1,1}};
    engine->activeScene->registry.cameras[camera] = BE::CameraComponent{ 45.0f, 0.1f, 100.0f, 1.0f, false, true };

    selectedAnchor = -1;
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

    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin(
        "Dockspace", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
    );
    ImGui::PopStyleVar(2);

    ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f));
    ImGui::End();

    if (glfwGetKey(engine->getWindow(), GLFW_KEY_W)) currentGizmoOperation = ImGuizmo::TRANSLATE;
    if (glfwGetKey(engine->getWindow(), GLFW_KEY_E)) currentGizmoOperation = ImGuizmo::ROTATE;
    if (glfwGetKey(engine->getWindow(), GLFW_KEY_R)) currentGizmoOperation = ImGuizmo::SCALE;
    if (glfwGetKey(engine->getWindow(), GLFW_KEY_T)) currentGizmoOperation = ImGuizmo::UNIVERSAL;
}

void Editor::showPanels() {
    
    RenderStats();

    Menu();
    Viewport();
    Heirarchy();
    Resources();
    Inspector();
    // Settings();
    // Popups();
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

// === Custom ImGui === //

namespace UI {

bool DragFloat3(const char *label, float *v, float v_speed = (1.0f), float v_min = (0.0f), float v_max = (0.0f)) {

    bool changed = false;
    
    ImGui::Text(label);
    ImGui::SameLine();
    changed = ImGui::DragFloat("##x", &v[0], v_speed, v_min, v_max);
    ImGui::SameLine();
    changed = ImGui::DragFloat("##xx", &v[1], v_speed, v_min, v_max);
    ImGui::SameLine();
    changed = ImGui::DragFloat("##xxx", &v[2], v_speed, v_min, v_max);

    return changed;

}

void Mat4(const char* label, glm::mat4& mat) {
    if (ImGui::CollapsingHeader(label)) {
        for (int row = 0; row < 4; row++) {
            float tmp[4] = { mat[row][0], mat[row][1], mat[row][2], mat[row][3] };
            if (ImGui::InputFloat4(("Row " + std::to_string(row) + "##" + *label).c_str(), tmp)) {
                mat[row][0] = tmp[0];
                mat[row][1] = tmp[1];
                mat[row][2] = tmp[2];
                mat[row][3] = tmp[3];
            }
        }
    }
}


} // UI namespace

// === Panels === //

void Editor::Menu() {

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { engine->closeWindow(); }
            ImGui::EndMenu(); 
        };
        if (ImGui::BeginMenu("Edit")) {
            // if (ImGui::MenuItem("Add Scene")) { std::string label = "Scene" + std::to_string(engine->scenes.size()+1); engine->addScene(label); }
            if (ImGui::MenuItem("Create Anchor")) { selectedAnchor = engine->activeScene->createAnchor(); }
            if (ImGui::MenuItem("Import")) { FileFolders(); }
            ImGui::EndMenu(); 
        };
        if (ImGui::BeginMenu("View")) { ImGui::EndMenu(); };
        if (ImGui::BeginMenu("Window")) { ImGui::EndMenu(); };
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Recompile Shaders")) { engine->resources().recompileShaders(); }
            ImGui::EndMenu(); 
        };
        ImGui::EndMainMenuBar();
    }

}

void Editor::Viewport() {

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoMove;

    ImGui::Begin("Hello, ImGui!", nullptr, window_flags);

    if (ImGui::IsWindowHovered()) { 
        engine->editorCamera.inputs();
    } else {
        engine->editorCamera.scrollDelta = glm::vec2(0,0);
    }

    ImVec2 viewportPos = ImGui::GetCursorScreenPos();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    ImVec2 mousePos = ImGui::GetMousePos();

    bool resizing = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    if (!resizing) engine->viewport->resize(viewportSize.x, viewportSize.y);
    
    // if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) engine->viewport->camera->handleInputs(engine->getWindow(), engine->ts.dt);
    ImGui::Image((void*)(intptr_t) engine->viewport->getColorTexture(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));

    ImVec2 relativePos = ImVec2(mousePos.x - viewportPos.x, mousePos.y - viewportPos.y);

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

    bool picking = false;

    if (selectedAnchor != -1) {
        if (engine->activeScene->registry.transforms.find(selectedAnchor) != engine->activeScene->registry.transforms.end()) {
            TransformComponent& t = engine->activeScene->registry.transforms[selectedAnchor];
            
            glm::mat4 translation = glm::translate(glm::mat4(1), t.position);
            glm::mat4 rotationX = glm::rotate(glm::mat4(1), glm::radians(t.rotationEuler.x), glm::vec3(1,0,0));
            glm::mat4 rotationY = glm::rotate(glm::mat4(1), glm::radians(t.rotationEuler.y), glm::vec3(0,1,0));
            glm::mat4 rotationZ = glm::rotate(glm::mat4(1), glm::radians(t.rotationEuler.z), glm::vec3(0,0,1));
            glm::mat4 rotation = rotationX * rotationY * rotationZ;
            glm::mat4 scale = glm::scale(glm::mat4(1), t.scale);

            t.model = translation * rotation * scale;

            ImGuizmo::Manipulate(
                glm::value_ptr(engine->viewport->camera2.viewMatrix),
                glm::value_ptr(engine->viewport->camera2.projectionMatrix),
                currentGizmoOperation,
                currentGizmoMode,
                glm::value_ptr(t.model)
            );

            if (ImGuizmo::IsUsing()) {
                glm::vec3 translation, rotation, scale;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(t.model),
                    glm::value_ptr(translation),
                    glm::value_ptr(rotation),
                    glm::value_ptr(scale)
                );

                t.position = translation;
                t.rotationEuler = rotation;
                t.scale = scale;

                picking = true;
            }
        }

    }

    if (!picking && ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
        int texX = int((relativePos.x / viewportSize.x) * engine->viewport->width);
        int texY = int((1.0f - (relativePos.y / viewportSize.y)) * engine->viewport->height);

        GLuint id = 0;
        engine->viewport->fbo.bind();
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glReadPixels(texX, texY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &id);
        engine->viewport->fbo.unbind();

        selectedAnchor = id - 1;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::Heirarchy() {
    
    ImGui::Begin("Heirarchy");
    for (Anchor a : engine->activeScene->anchors) {

        std::string name = "Anchor" + std::to_string(a) + "##" + std::to_string(a);
        if (engine->activeScene->registry.tags.find(a) != engine->activeScene->registry.tags.end()) {
            if (!engine->activeScene->registry.tags[a].name.empty()) {
                name = engine->activeScene->registry.tags[a].name + "##" + std::to_string(a);
            }
        }

        if (ImGui::Button(name.c_str())) selectedAnchor = a;
        ImGui::SameLine();
        if (ImGui::Button(std::string("x##" + name).c_str())) {
            engine->activeScene->removeAnchor(a);
            selectedAnchor = -1;
        }
    }
    ImGui::End();
}

void Editor::Popups() {

    ImGui::Begin("Create Shader");

        static CreatedShader shader;
        
        ImGui::InputText("Name", shader.name, sizeof(shader.name));

        const char* typeLabels[] = { "Scene", "Post" };
        int currentType = static_cast<int>(shader.type);
        if (ImGui::Combo("Base Shader", &currentType, typeLabels, IM_ARRAYSIZE(typeLabels))) {
            shader.type = static_cast<CreatedShader::BaseType>(currentType);
        }

        ImGui::InputTextMultiline("##functionSOurce", shader.globalSource, sizeof(shader.globalSource), ImVec2(-FLT_MIN, 400), ImGuiInputTextFlags_AllowTabInput);
        ImGui::InputTextMultiline("##mianSOurce", shader.mainSource, sizeof(shader.mainSource), ImVec2(-FLT_MIN, 400), ImGuiInputTextFlags_AllowTabInput);

        if (ImGui::Button("Reset")) {
            shader.type = BE::CreatedShader::BaseType::Scene;
        }

        ImGui::SameLine();

        if (ImGui::Button("Submit")) {

            std::string vertexSource = "";
            if (shader.type == CreatedShader::BaseType::Scene) vertexSource = BE::Default::baseSceneVertex;

            std::string fragmentSource = BE::Default::baseSceneFragmentGlobal + shader.globalSource + BE::Default::baseSceneFragmentMain + shader.mainSource + "\n}";

            engine->resources().loadShader(shader.name, &vertexSource, &fragmentSource);

            shader.type = BE::CreatedShader::BaseType::Scene;

            std::cout << fragmentSource << std::endl;
        
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

        if (ImGui::Selectable(std::string(key + "##Mesh").c_str())) {}
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("MESH", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    ImGui::SeparatorText("Materials");
    for (auto& [key, material] : engine->resources().materials) {
        if (!MatchesSearch(key) && !MatchesSearch("Materials")) continue;

        if (ImGui::Selectable(std::string(key + "##Material").c_str())) {}
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("MATERIAL", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    ImGui::SeparatorText("Shaders");
    for (auto& [key, shader] : engine->resources().shaders) {
        if (!MatchesSearch(key) && !MatchesSearch("Shaders")) continue;

        if (ImGui::Selectable(std::string(key + "##Shader").c_str())) {}
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("SHADER", key.c_str(), key.size() + 1);
            ImGui::Text("Dragging %s", key.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    ImGui::SeparatorText("Textures");
    for (auto& [key, texture] : engine->resources().textures) {
        if (!MatchesSearch(key) && !MatchesSearch("Textures")) continue;

        if (ImGui::Selectable(std::string(key + "##Texture").c_str())) {}
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

        float availWidth = ImGui::GetContentRegionAvail().x;

        // NAME COMPONENT

        bool hasName = engine->activeScene->registry.tags.find(selectedAnchor) != engine->activeScene->registry.tags.end();
        if (hasName) {
            NameComponent& t = engine->activeScene->registry.tags[selectedAnchor];

            ImGui::BeginGroup();

            char buffer[256];
            std::strncpy(buffer, t.name.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
                t.name = buffer;
            }
        
            // const char* typeLabels[] = { "None", "Player" };
            // int currentType = static_cast<int>(t.type);
            // if (ImGui::Combo("Anchor Type", &currentType, typeLabels, IM_ARRAYSIZE(typeLabels))) {
            //     t.type = static_cast<AnchorType>(currentType);
            // }

            ImGui::Separator();
            
            ImGui::EndGroup();
        }

        // TRANSFORM COMPONENT

        bool openTransform = true;
        bool hasTransform = engine->activeScene->registry.transforms.find(selectedAnchor) != engine->activeScene->registry.transforms.end();
        if (hasTransform) {
            TransformComponent& t = engine->activeScene->registry.transforms[selectedAnchor];

            if (ImGui::CollapsingHeader("Transform", &openTransform)) {

                ImGui::BeginGroup();

                ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
                ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotationEuler), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f);
                
                ImGui::Separator();

                ImGui::EndGroup();
            }
        }

        if (!openTransform) engine->activeScene->registry.transforms.erase(selectedAnchor);

        // MESH COMPONENT

        bool openMesh = true;
        bool hasMesh = engine->activeScene->registry.meshes.find(selectedAnchor) != engine->activeScene->registry.meshes.end();
        if (hasMesh) {
            MeshComponent& m = engine->activeScene->registry.meshes[selectedAnchor];
            if (ImGui::CollapsingHeader("Material", &openMesh)) {
                
                static glm::vec2 rotation(0.0f);
                static bool cullPreview = false;
                
                if (m.mesh != nullptr) {
                    meshPreview.resize(previewResolution, previewResolution);
                    m.mesh->makePreview(meshPreview, *engine->resources().shaders["default_uv"].get(), rotation, cullPreview);
                }

                ImGui::BeginGroup();

                const char* meshName = m.mesh ? "Unknown" : "None";
                for (auto& [key, mesh] : engine->resources().meshes) {
                    if (m.mesh == mesh) {
                        meshName = key.c_str();
                        break;
                    }
                }

                if (ImGui::BeginCombo("Mesh##Dropdown", meshName)) {
                    for (auto& [key, mesh] : engine->resources().meshes) {
                        bool isSelected = (m.mesh == mesh);
                        if (ImGui::Selectable(key.c_str(), isSelected)) {
                            m.mesh = mesh;
                            rotation = glm::vec2(0, 0);
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (m.mesh) {
                    ImGui::Text("Mesh Preview:");
                    ImGui::Image((void*)(intptr_t) meshPreview.attachments[0].ID, ImVec2(availWidth-10, availWidth-10), ImVec2(0, 1), ImVec2(1, 0));
                    
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

                    ImGui::Checkbox("View with Culling", &cullPreview);
                    ImGui::Text("Vertices: %d", m.mesh->vertices.size());
                    ImGui::Text("Indices: %d", m.mesh->indices.size());
                    ImGui::Text("Triangles: %d", m.mesh->indices.size()/3);
                    if (ImGui::Button("Remove Mesh")) { m.mesh = nullptr; }
            
                    ImGui::Separator();

                }
                
                const char* materialName = m.material ? "Unknown" : "None";
                for (auto& [key, material] : engine->resources().materials) {
                    if (m.material == material) {
                        materialName = key.c_str();
                        break;
                    }
                }

                if (ImGui::BeginCombo("Material##Dropdown", materialName)) {
                    for (auto& [key, material] : engine->resources().materials) {
                        bool isSelected = (m.material == material);
                        if (ImGui::Selectable(key.c_str(), isSelected)) {
                            m.material = material;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (m.material) {
                    
                    ImGui::BeginDisabled(false);

                    if (m.material->diffuseMap) ImGui::Image((void*)(intptr_t)m.material->diffuseMap->ID, ImVec2(availWidth/4, availWidth/4), ImVec2(0, 1), ImVec2(1, 0));
                    else ImGui::Dummy(ImVec2(availWidth/4, availWidth/4));
                    ImGui::SameLine();

                    if (m.material->normalMap) ImGui::Image((void*)(intptr_t)m.material->normalMap->ID, ImVec2(availWidth/4, availWidth/4), ImVec2(0, 1), ImVec2(1, 0));
                    else ImGui::Dummy(ImVec2(availWidth/4, availWidth/4));
                    ImGui::SameLine();
                    
                    if (m.material->roughnessMap) ImGui::Image((void*)(intptr_t)m.material->roughnessMap->ID, ImVec2(availWidth/4, availWidth/4), ImVec2(0, 1), ImVec2(1, 0));
                    else ImGui::Dummy(ImVec2(availWidth/4, availWidth/4));

                    ImGui::Text("Material Properties:");
                    if (ImGui::SliderFloat("Metallic", &m.material->metallic, 0.0f, 1.0f)) {}
                    if (ImGui::SliderFloat("Roughness", &m.material->roughness, 0.0f, 1.0f)) {}
                    if (ImGui::ColorEdit4("Diffuse Color", &m.material->diffuseColor.x)) {}
                    if (ImGui::Checkbox("Cull Faces?", &m.material->cull)) {}
                    if (ImGui::Checkbox("Transparent?", &m.material->transparent)) {}
                    ImGui::EndDisabled();
                    
                    if (ImGui::Button("Remove Material")) { m.material = nullptr; }
            
                    ImGui::Separator();
                
                }
                
                const char* shaderName = m.shader ? "Unknown" : "None";
                for (auto& [key, shader] : engine->resources().shaders) {
                    if (m.shader == shader) {
                        shaderName = key.c_str();
                        break;
                    }
                }
                
                if (ImGui::BeginCombo("Shader##Dropdown", shaderName)) {
                    for (auto& [key, shader] : engine->resources().shaders) {
                        bool isSelected = (m.shader == shader);
                        if (ImGui::Selectable(key.c_str(), isSelected)) {
                            m.shader = shader;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (m.shader) {
                    if (ImGui::Button("Remove Shader")) { m.shader = nullptr; }
            
                    ImGui::Separator();
                }
        
                ImGui::EndGroup();

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
        }

        if (!openMesh) engine->activeScene->registry.meshes.erase(selectedAnchor);

        // LIGHT COMPONENT

        bool openLight = true;
        bool hasLight = engine->activeScene->registry.lights.find(selectedAnchor) != engine->activeScene->registry.lights.end();
        if (hasLight) {
            if (ImGui::CollapsingHeader("Light", &openLight)) {
                ImGui::BeginGroup();

                LightComponent& l = engine->activeScene->registry.lights[selectedAnchor];

                const char* typeLabels[] = { "Directlight", "Pointlight", "Spotlight" };
                if (ImGui::Combo("Light Type", &l.type, typeLabels, IM_ARRAYSIZE(typeLabels))) {}

                if (ImGui::ColorEdit3("Color", &l.color.x)) {}
                if (ImGui::DragFloat("Intensity", &l.intensity)) {}

                ImGui::Separator();

                ImGui::EndGroup();
            }
        }

        // CAMERA COMPONENT

        bool openCamera = true;
        bool hasCamera = engine->activeScene->registry.cameras.find(selectedAnchor) != engine->activeScene->registry.cameras.end();
        if (hasCamera) {
            if (ImGui::CollapsingHeader("Camera", &openCamera)) {
                ImGui::BeginGroup();

                CameraComponent& c = engine->activeScene->registry.cameras[selectedAnchor];

                ImGui::DragFloat("FOV", &c.fov);
                // ImGui::DragFloat("nearPlane", &c.nearPlane);
                // ImGui::DragFloat("farPlane", &c.farPlane);
                ImGui::DragFloat("Zoom", &c.zoom, 0.01f);
                ImGui::Checkbox("IsMain", &c.isMain);
                ImGui::Checkbox("Perspective", &c.isPerspective);

                ImGui::Separator();

                ImGui::EndGroup();
            }
        }

        if (!openCamera) engine->activeScene->registry.cameras.erase(selectedAnchor);

        if (!hasName || !hasTransform || !hasMesh || !hasLight || !hasCamera) {

            if (ImGui::Button("Add Component", ImVec2(availWidth, 0))) { ImGui::OpenPopup("AddComponentPopup"); }
            if (ImGui::BeginPopup("AddComponentPopup")) {

                if (!hasName) { if (ImGui::MenuItem("Add Name")) { engine->activeScene->registry.tags[selectedAnchor] = NameComponent{"", AnchorType::None}; } }
                if (!hasTransform) { if (ImGui::MenuItem("Add Transform")) { engine->activeScene->registry.transforms[selectedAnchor] = TransformComponent{{0,0,0}, {0,0,0}, {1,1,1}}; } }
                if (!hasMesh) { if (ImGui::MenuItem("Add Mesh")) { engine->activeScene->registry.meshes[selectedAnchor] = MeshComponent{nullptr, nullptr, nullptr}; } }
                if (!hasLight) { if (ImGui::MenuItem("Add Light")) { engine->activeScene->registry.lights[selectedAnchor] = LightComponent{ glm::vec3(1,1,1), 1.0f, 1 }; } }
                if (!hasCamera) { if (ImGui::MenuItem("Add Camera")) { engine->activeScene->registry.cameras[selectedAnchor] = CameraComponent{ 45.0f, 0.1f, 100.0f, 1.0f, false, true }; } }

                ImGui::EndPopup();
            }

        }

    }
    ImGui::End();
}

void Editor::Settings() {
    ImGui::Begin("Settings");

    ImGui::SliderInt("Preview Resolution", &previewResolution, 1.0f, 256.0f);
    
    ImGui::End();
}

void Editor::RenderStats() {
    ImGui::Begin("Render Stats");
    ImGui::Text("draw calls: %d", Stats::g_renderStats.drawCalls);
    ImGui::Text("triangles: %d", Stats::g_renderStats.triangles);
    ImGui::Text("vertices: %d", Stats::g_renderStats.vertices);
    ImGui::Text("indices: %d", Stats::g_renderStats.indices);

    ImGui::Separator();
    
    ImGui::Text("shader binds: %d", Stats::g_renderStats.shaderBinds);
    ImGui::Text("texture binds: %d", Stats::g_renderStats.textureBinds);
    ImGui::Text("framebuffer binds: %d", Stats::g_renderStats.framebufferBinds);

    ImGui::Separator();
    
    ImGui::Text("vao binds: %d", Stats::g_renderStats.vaoBinds);
    ImGui::Text("vbo binds: %d", Stats::g_renderStats.vboBinds);
    ImGui::Text("ebo binds: %d", Stats::g_renderStats.eboBinds);

    ImGui::Separator();
    
    ImGui::Text("Engine FPS: %.1f MS: %.2f", engine->ts.fps, engine->ts.ms);
    ImGui::Text("ImGui FPS: %.1f MS: %.2f", ImGui::GetIO().Framerate, 1000.0f/ImGui::GetIO().Framerate);
    ImGui::End();
}

void Editor::FileFolders() {
    const char* filters[] = { "*.png", "*.cpp" };
    const char* filePaths = tinyfd_openFileDialog("Open a File", "", 0, nullptr, nullptr, true);
    
    if (filePaths) {

        std::vector<std::string> files;
        std::stringstream ss(filePaths);
        std::string item;

        while (std::getline(ss, item, '|')) {
            files.push_back(item);
        }

        for (auto& f : files) {
            std::string n = f.c_str() ? f.substr(std::string(f).find_last_of("/\\") + 1) : "";
            std::string ext = n.find_last_of('.') != std::string::npos ? n.substr(n.find_last_of('.') + 1) : "";

            if (ext == "obj") {
                engine->resources().loadMesh(n, f);
            } else if (ext == "dsl" || ext == "besl") {
                engine->resources().loadShaderDSL(f);
            } else if (ext == "png" || ext == "jpg") {
                engine->resources().loadTexture(n, f, "diffuse");
            } else if (ext == "mtl") {
                engine->resources().loadMaterial(n, f);
            }

        }
    }
}

}; // BE namespace