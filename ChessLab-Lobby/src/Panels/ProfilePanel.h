#pragma once

#include <memory>

#include "Walnut/Image.h"

namespace Panels
{
	class ProfilePanel
	{
	public:
		void OnAttach();

		void OnImGuiRender();

	private:
		std::shared_ptr<Walnut::Image> m_ProfileIcon;

	};
}