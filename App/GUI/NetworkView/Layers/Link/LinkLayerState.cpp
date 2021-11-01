#include "stdafx.h"
#include "LinkLayerState.h"


namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		LinkLayerState::LinkLayerState()
		{
		}
		/**
		*
		*
		*
		*/
		LinkLayerState::~LinkLayerState()
		{
		}
		/**
		*
		*
		*
		*/
		bool LinkLayerState::IsControlKeyPressed()
		{
			return (GetKeyState(VK_CONTROL) < 0);
		}
		/**
		*
		*
		*
		*/
		bool LinkLayerState::IsShiftKeyPressed()
		{
			return (GetKeyState(VK_SHIFT) < 0);
		}
		/**
		*
		*
		*
		*/
		bool LinkLayerState::IsLeftButtonPressed()
		{
			return (GetKeyState(VK_LBUTTON) < 0);
		}

	}
}
