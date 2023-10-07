#pragma once
#ifndef _IRENDEREXECUTION_H_
#define _IRENDEREXECUTION_H_

#include "Unit.h"

class ENGINE_API IRenderExecution
{
public:
	IRenderExecution();
	virtual ~IRenderExecution();

	virtual void executeParticle(const vector<DrawArraysIndirectCommand>& cmds) = 0;
	virtual void executeMesh(const vector<DrawElementsIndirectCommand>& cmds) = 0;
};

#endif // !_IRENDEREXECUTION_H_
