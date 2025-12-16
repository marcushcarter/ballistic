#include "ProjectManager.h"
#include "Scene/SceneManager.h"

namespace Ballistic {

    ProjectManager::ProjectManager() {
        m_SceneManager = std::make_shared<SceneManager>();
        m_SceneManager->createScene("New Scene");
    }
    
}