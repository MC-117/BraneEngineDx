#pragma once
#include "Live2DConfig.h"

void CubismPrintMessage_Impl(const char* msg);
void CubismLogVerbose_Impl(const char* fmt, ...);
void CubismLogDebug_Impl(const char* fmt, ...);
void CubismLogInfo_Impl(const char* fmt, ...);
void CubismLogWarning_Impl(const char* fmt, ...);
void CubismLogError_Impl(const char* fmt, ...);