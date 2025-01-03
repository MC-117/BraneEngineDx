#pragma once
#include "../RenderCore/RenderInterface.h"
#include "../GPUBuffer.h"

struct SSRBinding : public IRenderData
{
	struct SSRInfo
	{
		Vector4f hiZUVScale;
		int hiZStartLevel;
		int hiZStopLevel;
		int hiZMaxStep;
		int ssrResolveSamples;
		float ssrResolveRadius;
		float ssrPad[3];

		SSRInfo();
	} ssrInfo;

	GPUBuffer ssrInfoBuffer;

	SSRBinding();

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};