#pragma once

#include "lrpch.h"

namespace Ballistic {

	struct Parent { entt::entity entity = entt::null; };
	struct Children { std::vector<entt::entity> entities; };

	struct TagComponent {
		TagComponent() = default;
		TagComponent(const std::string& tag) : tag(tag) {}

		std::string tag;
	};

	struct TransformComponent {
	public:
		TransformComponent();
		operator glm::mat4() const;

		inline glm::vec3 GetRotation() { return glm::degrees(m_Rotation); }
		inline glm::vec3 GetTranslation() { return m_Translation; }
		inline glm::vec3 GetScale() { return m_Scale; }

		glm::mat4 GetMatrix() const;

		void SetRotation(const glm::vec3& angles);
		void SetTranslation(const glm::vec3& translation);
		void SetScale(const glm::vec3& scale);

		void IncrementRotation(const glm::vec3& delta);
		void IncrementTranslation(const glm::vec3& delta);
		void IncrementScale(const glm::vec3& delta);

	private:
		mutable bool m_MatrixDirty;
		mutable glm::mat4 m_ModelMatrix;

		glm::vec3 m_Translation;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;
	};

	struct SphereComponent {
		SphereComponent() = default;
		SphereComponent(float radius) : radius(radius) {}
		
		float radius;
	};

}