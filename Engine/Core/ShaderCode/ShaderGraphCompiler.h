#pragma once

#include "ShaderCompiler.h"
#include "../ShaderStage.h"
#include "ShaderGraph/HLSLGeneration.h"
#include "ShaderGraph/ShaderGraph.h"

class ENGINE_API HLSLWriterShaderReader : public IShaderCodeReader
{
public:
    HLSLWriterShaderReader(HLSLWriter& hlslWriter);

    virtual bool open(const char* path);
    virtual void close();

    virtual bool isOpen() const;
	
    virtual const char* getPath() const;
    virtual const char* getEnvPath() const;
    virtual const char* getName() const;

    virtual bool readLine(string& line);
    virtual bool readChar(char& out);
    virtual void unreadChar();

    virtual size_t getPos();
    virtual void setPos(size_t pos);
protected:
    HLSLWriter& hlslWriter;
    stringstream stream;
    bool opened;
    string path;
    string envPath;
    string name;
};

class ENGINE_API ShaderGraphCompiler
{
public:
    static ShaderGraph* compile(const string& path);
    static bool compile(ShaderGraph& graph);
};