#include "Panels/IPanel.h"

namespace ballistic
{
    void IPanel::Attach() {
        LogDebug(m_name, " panel attached");

        OnAttach();
    }
    
    void IPanel::Detach() {
        LogDebug(m_name, " panel dettached");

        OnDetach();
    }
    
} // namespace ballistic
