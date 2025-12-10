#pragma once

#include "lrpch.h"
#include "Components.h"

namespace Ballistic {

	glm::mat4 WorldTransform(entt::registry& registry, entt::entity entity);

	class Scene {
	public:
		entt::entity create(const std::string& tag, entt::entity parent = entt::null);
	    void destroy(entt::entity entity);

	    void reparent(entt::entity entity, entt::entity newParent = entt::null);
	    
	    void duplicate(entt::entity original);
	    void duplicate(entt::entity original, entt::entity targetParent);

	    void clear();

	    entt::registry registry;
	};

}