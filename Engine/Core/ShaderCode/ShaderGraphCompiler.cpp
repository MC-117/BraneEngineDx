#include "ShaderGraphCompiler.h"
#include <fstream>
#include <string>
#include "../Console.h"
#include "../Material.h"
#include "ShaderCompiler.h"
#include "Core/Utility/EngineUtility.h"
#include "ShaderGraph/TextureNode.h"

HLSLWriterShaderReader::HLSLWriterShaderReader(HLSLWriter& hlslWriter)
    : hlslWriter(hlslWriter)
{
}

bool HLSLWriterShaderReader::open(const char* path)
{
    filesystem::path u8path = filesystem::u8path(path);
    this->path = u8path.generic_u8string();
    this->name = getFileNameWithoutExt(this->path);
    this->envPath = u8path.parent_path().generic_u8string();
    
    stream.clear();
    hlslWriter.writeTo(stream);
    stream.seekg(0, std::ios::beg);
    opened = true;
    return true;
}

void HLSLWriterShaderReader::close()
{
    stream.clear();
    opened = false;
}

bool HLSLWriterShaderReader::isOpen() const
{
    return opened;
}

const char* HLSLWriterShaderReader::getPath() const
{
    return path.c_str();
}

const char* HLSLWriterShaderReader::getEnvPath() const
{
    return envPath.c_str();
}

const char* HLSLWriterShaderReader::getName() const
{
    return name.c_str();
}

bool HLSLWriterShaderReader::readLine(string& line)
{
    return (bool)getline(stream, line);
}

bool HLSLWriterShaderReader::readChar(char& out)
{
    return (bool)stream.get(out);
}

void HLSLWriterShaderReader::unreadChar()
{
    stream.unget();
}

size_t HLSLWriterShaderReader::getPos()
{
    return stream.tellg();
}

void HLSLWriterShaderReader::setPos(size_t pos)
{
    stream.seekg(pos);
}

ShaderGraph* ShaderGraphCompiler::compile(const string& path)
{
    ifstream wf = ifstream(path);
    SerializationInfoParser sip = SerializationInfoParser(wf, path);
    if (!sip.parse()) {
        Console::error("%s: load failed '%s'", path.c_str(), sip.parseError.c_str());
        wf.close();
        return NULL;
    }
    SerializationInfo& info = sip.infos[0];
    wf.close();

    return compile(info);
}

ShaderGraph* ShaderGraphCompiler::compile(const SerializationInfo& info)
{
    Serializable* serializable = ShaderGraph::ShaderGraphSerialization::serialization.deserialize(info);
    ShaderGraph* graph = castTo<ShaderGraph>(serializable);
    bool success = false;
    if (graph) {
        graph->setPath(info.path);
        success = compile(*graph);
    }
    if (!success) {
        delete serializable;
        serializable = NULL;
        graph = NULL;
    }
    return graph;
}

bool ShaderGraphCompiler::compile(ShaderGraph& graph)
{
    HLSLWriter writer;
    ShaderGraph::generateCode(writer, graph);
    HLSLWriterShaderReader reader(writer);
    reader.open(graph.getPath().c_str());
    ShaderCompiler compiler;
    compiler.init(reader);
    while (compiler.compile()) {}
    
    if (!compiler.isSuccessed()) {
        return false;
    }

    bool successed = true;
    
    graph.setName(compiler.getName());
    auto adapters = compiler.getAdapters();
    for (auto b = adapters.begin(), e = adapters.end(); b != e; b++) {
        successed &= graph.addShaderAdapter(*b->second);
    }

    return successed;
}

void ShaderGraphCompiler::gatherReferencedTextures(const SerializationInfo& info, vector<string>& texPaths)
{
    const SerializationInfo* variables = info.get("variables");
    for (const auto& variable : variables->sublists) {
        if (variable.serialization && variable.serialization->isChildOf(TextureParameterVariable::serialization())) {
            string texPath;
            if (variable.get("defaultValue", texPath)) {
                if (!texPath.empty()) {
                    texPaths.emplace_back(std::move(texPath));
                }
            }
        }
    }
}
