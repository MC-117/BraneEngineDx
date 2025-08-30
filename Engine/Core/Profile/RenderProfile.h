#pragma once
#include "implot.h"
#include "RenderDurationProfiler.h"

class IRenderContext;

class RenderProfileScope
{
public:
    RenderProfileScope(const string& name, const char* text, ...);
    RenderProfileScope(const RenderProfileScope& scope) = delete;
    ~RenderProfileScope();
};

class RenderProfileScopeWithContext
{
public:
    IRenderContext& context;
    
    RenderProfileScopeWithContext(IRenderContext& context, const string& name, const char* text, ...);
    RenderProfileScopeWithContext(const RenderProfileScopeWithContext& scope) = delete;
    ~RenderProfileScopeWithContext();
};

#define RENDER_SCOPE_NO_CONTEXT(name) RenderProfileScope name##_RenderScope(#name, "");
#define RENDER_DESC_SCOPE_NO_CONTEXT(name, text, ...) RenderProfileScope name##_RenderScope(#name, text, ##__VA_ARGS__);

#define RENDER_SCOPE(context, name) RenderProfileScopeWithContext name##_RenderScope(context, #name, "");
#define RENDER_NAME_SCOPE(context, name) RenderProfileScopeWithContext __FILE__##__LINE__##_RenderScope(context, name, "");
#define RENDER_DESC_SCOPE(context, name, text, ...) RenderProfileScopeWithContext name##_RenderScope(context, #name, text, ##__VA_ARGS__);

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
