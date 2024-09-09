#include "ImGUIPass.h"
#include "../IRenderContext.h"

bool ImGuiPass::loadDefaultResource()
{
	return true;
}

void ImGuiPass::setDrawData(ImDrawData* pDrawData)
{
	drawData = pDrawData;
	/*if (pDrawData == NULL)
		return;
	if (drawData.CmdLists != nullptr) {
		for (int i = 0; i < drawData.CmdListsCount; i++) {
			delete drawData.CmdLists[i];
		}
		delete drawData.CmdLists;
		drawData.CmdLists = nullptr;
	}
	drawData.Clear();
	drawData = *pDrawData;
	drawData.CmdLists = new ImDrawList * [pDrawData->CmdListsCount];
	for (int i = 0; i < pDrawData->CmdListsCount; i++) {
		drawData.CmdLists[i] = pDrawData->CmdLists[i]->CloneOutput();
	}*/
}

void ImGuiPass::prepare()
{
}

void ImGuiPass::execute(IRenderContext& context)
{
	context.execteImGuiDraw(drawData);
}

void ImGuiPass::reset()
{
}
