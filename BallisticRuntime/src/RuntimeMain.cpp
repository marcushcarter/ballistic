#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "RuntimeLayer.h"

namespace Ballistic {

	class BallisticRuntime : public Application {
	public:
		BallisticRuntime() {}

		void Shutdown() override {
			Application::Shutdown();
		}
		
		~BallisticRuntime() {
		}

	};

	Application* CreateApplication() {
		return new BallisticRuntime();
	}

};	