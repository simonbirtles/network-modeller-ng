#include "stdafx.h"
#include "ISelectedObjectsService.h"
#include "SelectedObjects.h"
#include "ISelectedObjects.h"

namespace NM
{
	/**
	*
	*
	*/
	ISelectedObjectsService::ISelectedObjectsService() :
		_selected(nullptr),
		_serviceStarted(false)
	{
	}
	/**
	*
	*
	*/
	ISelectedObjectsService::~ISelectedObjectsService()
	{
		StopRegService();
	}
	/**
	*
	*
	*/
	bool ISelectedObjectsService::StartRegService()
	{
		if (!_serviceStarted && (_selected == nullptr))
		{
			_selected = new SelectedObjects();
			_serviceStarted = true;
		}
		return true;
	}
	/**
	*
	*
	*/
	bool ISelectedObjectsService::StopRegService()
	{
		if (_serviceStarted)
		{
			delete _selected;
			_selected = nullptr;
			_serviceStarted = false;
		}
		return true;
	}
	/**
	*
	*
	*/
	void* ISelectedObjectsService::GetClientInterface()
	{
		if (_serviceStarted && _selected)
		{
			return new ISelectedObjects(&_selected);
		}
		return nullptr;
	}

}