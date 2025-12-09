#pragma once

#include "lrpch.h"
#include "Core/IWindow.h"

namespace Ballistic {

	class Application {
	public:
		Application();
		virtual ~Application() = default;
		void run();

	private:

		std::shared_ptr<IWindow> _IWindow;

		// void Shutdown();
	};

};	