#include "stdafx.h"
#include "NodeLayerState.h"

namespace NM
{
	namespace NetGraph
	{

		NodeLayerState::NodeLayerState()
		{
		}


		NodeLayerState::~NodeLayerState()
		{
		}
		/**
		*
		*
		*
		*/
		bool NodeLayerState::IsControlKeyPressed()
		{
			return (GetKeyState(VK_CONTROL) < 0);

			/*return ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 1);*/
		}
		/**
		*
		*
		*
		*/
		bool NodeLayerState::IsShiftKeyPressed()
		{
			return (GetKeyState(VK_SHIFT) < 0);

	/*		return ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 1);*/
		}
		/**
		*
		*
		*
		*/
		bool NodeLayerState::IsLeftButtonPressed()
		{
			return (GetKeyState(VK_LBUTTON) < 0);
		}

	}
}
