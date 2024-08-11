#include "ShaderAdapterCompiler.h"

bool ShaderAdapterCompiler::compile(const string& path)
{
    ShaderCodeFileReader reader;
    reader.open(path.c_str());
    ShaderCompiler compiler;
    compiler.init(reader);
    while (compiler.compile()) {}
    return compiler.isSuccessed();
}
