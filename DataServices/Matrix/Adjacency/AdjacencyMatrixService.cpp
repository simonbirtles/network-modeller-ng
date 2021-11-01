#include "stdafx.h"
#include "AdjacencyMatrixService.h"
#include "AdjacencyMatrix.h"
#include "IAdjacencyMatrix.h"

namespace NM
{
	namespace DataServices
	{
		/**
		*
		*
		*
		*/
		AdjacencyMatrixService::AdjacencyMatrixService()
		{
		}
		/**
		*
		*
		*
		*/
		AdjacencyMatrixService::~AdjacencyMatrixService()
		{
		}
		/**
		*
		*
		*
		*/
		bool AdjacencyMatrixService::StartRegService()
		{
			_pMatrix.reset(new CAdjacencyMatrix);
			return true;
		}
		/**
		*
		*
		*
		*/
		bool AdjacencyMatrixService::StopRegService()
		{
			_pMatrix.reset();
			return true;
		}
		/**
		*
		*
		*
		*/
		void* AdjacencyMatrixService::GetClientInterface()
		{
			return new IAdjacencyMatrix(_pMatrix);
		}

		// ns
	}
}
