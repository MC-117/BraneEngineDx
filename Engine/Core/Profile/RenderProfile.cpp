#include "RenderProfile.h"

#include "../GUI/GUIUtility.h"

RenderProfileScope::RenderProfileScope(const string& name, const char* text, ...)
{
    va_list ap;
    va_start(ap, text);
    const int textLen = strlen(text);
    int n = textLen * 2; /* Reserve two times as much as the length of the fmt_str */
    std::unique_ptr<char[]> formatted;
    while (true) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        memcpy_s(&formatted[0], textLen, text, textLen);
        const int final_n = vsnprintf(&formatted[0], n, text, ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    va_end(ap);
    RenderDurationProfiler::GInstance().beginScope(name, formatted.get());
}

RenderProfileScope::~RenderProfileScope()
{
    RenderDurationProfiler::GInstance().endScope();
}

int secondFormatter(double value, char* buff, int size, void* data)
{
    static double v[]      = {3600000,60000,1000,1,0.001,0.000001};
    static const char* p[] = {"h","m","s","ms","us","ns"};
    if (value == 0) {
        return snprintf(buff,size,"0s");
    }
    for (int i = 0; i < 6; ++i) {
        if (fabs(value) >= v[i]) {
            return snprintf(buff,size,"%g%s",value/v[i],p[i]);
        }
    }
    return snprintf(buff,size,"%g%s",value/v[5],p[5]);
}

bool ImPlot::BeginPlotRenderFrameProfile(ImPlotRenderFrameProfileContext& context, const char* name, int maxDepth, const ImVec2& size, ImPlotFlags flags)
{
    if (ImPlot::BeginPlot(name, size, flags))
    {
        ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_Opposite);
        ImPlot::SetupAxisFormat(ImAxis_X1, secondFormatter);
        //ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Invert);
        //ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
        //ImPlot::SetupAxisTicks(ImAxis_Y1, 0, maxDepth, maxDepth);
        context.barHeight = ImGui::GetFontSize() + 6;
        float plotHeight = ImPlot::GetPlotSize().y;
        ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, context.barHeight, plotHeight);
        return true;
    }
    return false;
}

void ImPlot::PlotRenderFrameProfile(ImPlotRenderFrameProfileContext& context, const char* name, const RenderDurationFrame& frame)
{
    const ImU32 hoveredCol = ImGui::GetColorU32(ImGuiCol_TabHovered);
    const ImU32 selectedCol = ImGui::GetColorU32(ImGuiCol_TabActive);
    if (ImPlot::BeginItem(name))
    {
        ImDrawList* drawList = ImPlot::GetPlotDrawList();
        int maxLines = frame.getMaxDepth();
        double startTime = frame.getStartTime().toMillisecond();
        float barHeight = context.barHeight;
        float plotLeftLimit = ImPlot::GetPlotPos().x;
        float plotRightLimit = ImPlot::GetPlotPos().x + ImPlot::GetPlotSize().x;

        auto DrawScope = [&](const string& scopeName, const string& desc, const Time& stime, const Time& etime, int depth)
        {
            double sMs = stime.toMillisecond();
            double eMs = etime.toMillisecond();
            ImU32 col = (ImU32)std::hash<string>()(scopeName) | 0xFF000000U;
            ImPlotPoint pp0 = { sMs - startTime, 0 };
            ImPlotPoint pp1 = { eMs - startTime, 0 };
            ImVec2 p0 = ImPlot::PlotToPixels(pp0);
            ImVec2 p1 = ImPlot::PlotToPixels(pp1);
            p0.y += depth * barHeight + 1;
            p1.y = p0.y + barHeight - 1;
            
            const float pWidth = p1.x - p0.x;
            if (pWidth > 1)
            {
                const float expectedRound = 3;
                
                drawList->AddRectFilled(p0, p1, col, expectedRound);

                ImVec2 textP0 = p0, textP1 = p1;
                textP0.x = std::max(p0.x, plotLeftLimit);
                textP1.x = std::min(p1.x, plotRightLimit);
                if (textP1.x - textP0.x >= 6)
                    ImGui::AddTextCentered(drawList, textP0, textP1, scopeName.c_str());
            
                if (ImGui::IsMouseHoveringRect(p0, p1))
                {
                    drawList->AddRect(p0, p1, hoveredCol, expectedRound, 0, 3);
                    ImGui::BeginTooltip();
                    ImGui::Text("Name: %s", scopeName.c_str());
                    char durLabel[IMPLOT_LABEL_MAX_SIZE];
                    secondFormatter(eMs - sMs, durLabel, IMPLOT_LABEL_MAX_SIZE, NULL);
                    ImGui::Text("Duration: %s", durLabel);
                    ImGui::Text("Desc: %s", desc.c_str());
                    ImGui::EndTooltip();
                }
            }
        };
        
        for (const RenderDurationScope* scope : frame.getScopes())
        {
            const string& scopeName = scope->getName();
            const string& desc = scope->getDesc();
            int depth = scope->getDepth() + context.lineOffset;
            DrawScope(scopeName, desc, scope->getStartCPUTime(), scope->getEndCPUTime(), depth);
            depth += maxLines + 1;
            DrawScope(scopeName, desc, scope->getStartGPUTime(), scope->getEndGPUTime(), depth);
        }
        context.lineOffset += (maxLines + 1) * 2;
        ImPlot::EndItem();
    }
}

void ImPlot::EndPlotRenderFrameProfile()
{
    ImPlot::EndPlot();
}
