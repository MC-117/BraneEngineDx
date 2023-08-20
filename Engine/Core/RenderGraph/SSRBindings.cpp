#include "SSRBindings.h"

SSRBinding::SSRInfo::SSRInfo()
	: hiZStartLevel(5)
	, hiZStopLevel(-1)
	, hiZMaxStep(48)
	, ssrResolveSamples(9)
	, ssrResolveRadius(0.001f)
{
}

SSRBinding::SSRBinding()
	: ssrInfoBuffer(GB_Constant, GBF_Struct, sizeof(SSRInfo))
{
}

void SSRBinding::create()
{
}

void SSRBinding::release()
{
	ssrInfoBuffer.resize(0);
}

void SSRBinding::upload()
{
	ssrInfoBuffer.uploadData(1, &ssrInfo, true);
}

void SSRBinding::bind(IRenderContext& context)
{
	static const ShaderPropertyName SSRInfoName = "SSRInfo";

	context.bindBufferBase(ssrInfoBuffer.getVendorGPUBuffer(), SSRInfoName);
}
