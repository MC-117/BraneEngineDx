#pragma once
#include "implot.h"
#include "RenderDurationProfiler.h"

class RenderProfileScope
{
public:
    RenderProfileScope(const string& name, const char* text, ...);
    RenderProfileScope(const RenderProfileScope& scope) = delete;
    ~RenderProfileScope();
};

#define RENDER_SCOPE(name) RenderProfileScope name##_RenderScope(#name, "");
#define RENDER_DESC_SCOPE(name, text, ...) RenderProfileScope name##_RenderScope(#name, text, ##__VA_ARGS__);

namespace ImPlot
{
    struct ImPlotRenderFrameProfileContext
    {
        float barHeight = 0;
        int lineOffset = 0;
    };
    bool BeginPlotRenderFrameProfile(ImPlotRenderFrameProfileContext& context, const char* name, int maxDepth, const ImVec2& size=ImVec2(-1,0), ImPlotFlags flags=0);
    void PlotRenderFrameProfile(ImPlotRenderFrameProfileContext& context, const char* name, const RenderDurationFrame& frame);
    void EndPlotRenderFrameProfile();
}
