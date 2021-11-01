#pragma once

namespace NM
{
	class SelectedObjectsClientObs
	{
	public:
		virtual ~SelectedObjectsClientObs() {};
		virtual void SelectedObjectsUpdate() = 0;
		virtual void MultiSelectChange(bool bMultiSelect) = 0;

	protected:
		SelectedObjectsClientObs() {};

	};
}