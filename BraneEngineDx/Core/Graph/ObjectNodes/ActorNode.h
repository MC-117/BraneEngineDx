#pragma once

#include "TransformNode.h"

class Actor;
class AudioSource;

DEC_REF_PIN(AudioSource, AudioSourceRefPin);
DEC_REF_VAR_CLASS(AudioSource, AudioSourceRefVariable, AudioSourceRefPin);

DEC_REF_PIN(Actor, ActorRefPin);
DEC_REF_VAR_CLASS(Actor, ActorRefVariable, ActorRefPin);

class AudioSourcePlayNode : public InOutFlowNode
{
public:
	Serialize(AudioSourcePlayNode, InOutFlowNode);

	AudioSourcePlayNode();
	virtual ~AudioSourcePlayNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
};

class AudioSourcePauseNode : public InOutFlowNode
{
public:
	Serialize(AudioSourcePauseNode, InOutFlowNode);

	AudioSourcePauseNode();
	virtual ~AudioSourcePauseNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
};

class AudioSourceStopNode : public InOutFlowNode
{
public:
	Serialize(AudioSourceStopNode, InOutFlowNode);

	AudioSourceStopNode();
	virtual ~AudioSourceStopNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
};

class AudioSourceIsPlayingNode : public InOutFlowNode
{
public:
	Serialize(AudioSourceIsPlayingNode, InOutFlowNode);

	AudioSourceIsPlayingNode();
	virtual ~AudioSourceIsPlayingNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
	BoolPin* returnPin = NULL;
};

class AudioSourceGetTimeNode : public InOutFlowNode
{
public:
	Serialize(AudioSourceGetTimeNode, InOutFlowNode);

	AudioSourceGetTimeNode();
	virtual ~AudioSourceGetTimeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
	FloatPin* timePin = NULL;
};

class AudioSourceSetTimeNode : public InOutFlowNode
{
public:
	Serialize(AudioSourceSetTimeNode, InOutFlowNode);

	AudioSourceSetTimeNode();
	virtual ~AudioSourceSetTimeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
	FloatPin* timePin = NULL;
};

class AudioSourceGetVolumeNode : public InOutFlowNode
{
public:
	Serialize(AudioSourceGetVolumeNode, InOutFlowNode);

	AudioSourceGetVolumeNode();
	virtual ~AudioSourceGetVolumeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
	FloatPin* volumePin = NULL;
};

class AudioSourceSetVolumeNode : public InOutFlowNode
{
public:
	Serialize(AudioSourceSetVolumeNode, InOutFlowNode);

	AudioSourceSetVolumeNode();
	virtual ~AudioSourceSetVolumeNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	AudioSourceRefPin* audioPin = NULL;
	FloatPin* volumePin = NULL;
};

class ActorGetAudioSourceCountNode : public InOutFlowNode
{
public:
	Serialize(ActorGetAudioSourceCountNode, InOutFlowNode);

	ActorGetAudioSourceCountNode();
	virtual ~ActorGetAudioSourceCountNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ActorRefPin* actorPin = NULL;
	IntPin* countPin = NULL;
};

class ActorGetAudioSourceNode : public InOutFlowNode
{
public:
	Serialize(ActorGetAudioSourceNode, InOutFlowNode);

	ActorGetAudioSourceNode();
	virtual ~ActorGetAudioSourceNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ActorRefPin* actorPin = NULL;
	IntPin* indexPin = NULL;
	AudioSourceRefPin* audioPin = NULL;
};