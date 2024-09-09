#include "MeshPass.h"

bool MeshPass::loadDefaultResource()
{
	return true;
}

void MeshPass::prepare()
{
	outputTextures.clear();
}

void MeshPass::execute(IRenderContext& context)
{
	RenderCommandExecutionInfo executionInfo(context);
	executionInfo.plusClearFlags = plusClearFlags;
	executionInfo.minusClearFlags = minusClearFlags;
	executionInfo.outputTextures = &outputTextures;
	executionInfo.timer = &timer;
	commandList.excuteCommand(executionInfo);
}

void MeshPass::reset()
{
	commandList.resetCommand();
}

void MeshPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	textures = outputTextures;
}
