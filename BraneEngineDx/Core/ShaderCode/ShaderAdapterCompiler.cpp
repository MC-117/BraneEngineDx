#include "ShaderAdapterCompiler.h"

bool ShaderAdapterCompiler::compile(const string& path)
{
    ShaderCompiler compiler;
    compiler.init(path);
    while (compiler.compile()) {}
    return compiler.isSuccessed();
}
