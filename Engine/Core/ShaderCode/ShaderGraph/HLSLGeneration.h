#pragma once
#include "../../CodeGeneration/ClangGeneration.h"

namespace HLSL
{
    ENGINE_API extern const Name x_op;
    ENGINE_API extern const Name y_op;
    ENGINE_API extern const Name z_op;
    ENGINE_API extern const Name w_op;
    ENGINE_API extern const Name r_op;
    ENGINE_API extern const Name g_op;
    ENGINE_API extern const Name b_op;
    ENGINE_API extern const Name a_op;
}

class ENGINE_API HLSLWriter : public ClangWriter
{
public:
    virtual Name convertKeyword(const Name& keyword);
    virtual int getOperatorParamNum(const Name& op);
    virtual const char* getOperatorFormatter(const Name& op);
protected:
    virtual ClangWriter* newWriter();
};
