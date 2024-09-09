#include "AnimationClip.h"
#include "../Skeleton/Skeleton.h"
#include <fstream>
#include "../Asset.h"
#include "../Transform.h"

string readString(istream& in)
{
	uint32_t len = 0;
	in.read((char*)&len, sizeof(uint32_t));
	string str;
	str.resize(len);
	for (int i = 0; i < len; i++) {
		uint8_t c;
		in.read((char*)&c, sizeof(uint8_t));
		str[i] = c;
	}
	return str;
}

void writeString(const string& str, ostream& out, bool haveLen = true)
{
	uint32_t len = str.length();
	if (haveLen) {
		out.write((char*)&len, sizeof(uint32_t));
	}
	for (int i = 0; i < len; i++) {
		uint8_t c = str[i];
		out.write((char*)&c, sizeof(uint8_t));
	}
}

TransformAnimationPlayer::TransformAnimationPlayer()
{
}

TransformAnimationPlayer::TransformAnimationPlayer(Transform & target)
	: targetTransform(&target)
{
}

TransformAnimationPlayer::TransformAnimationPlayer(Transform & target, TransformAnimationData & animationData)
	: targetTransform(&target), transformAnimationData(&animationData)
{
}

void TransformAnimationPlayer::setTargetTransform(Transform & target)
{
	targetTransform = &target;
	reset();
}

void TransformAnimationPlayer::setAnimationData(TransformAnimationData & animationData)
{
	transformAnimationData = &animationData;
	reset();
}

TransformData & TransformAnimationPlayer::getTransformData()
{
	return transformData;
}

TransformData TransformAnimationPlayer::getTransformData(float timeRate)
{
	TransformData data;
	if (isLoop()) {
		timeRate = fmod(timeRate, 1);
	}
	else {
		timeRate = timeRate > 1 ? 1 : timeRate;
	}
	data.position = positionCurvePlayer.curve->get(timeRate * positionCurvePlayer.curve->duration);
	data.rotation = rotationCurvePlayer.curve->get(timeRate * rotationCurvePlayer.curve->duration);
	data.scale = scaleCurvePlayer.curve->get(timeRate * scaleCurvePlayer.curve->duration);
	return data;
}

Enum<AnimationUpdateFlags> TransformAnimationPlayer::getUpdateFlags() const
{
	Enum<AnimationUpdateFlags> flags;
	if (positionCurvePlayer.curve != NULL && !positionCurvePlayer.curve->empty())
		flags |= AnimationUpdateFlags::Pos;
	if (rotationCurvePlayer.curve != NULL && !rotationCurvePlayer.curve->empty())
		flags |= AnimationUpdateFlags::Rot;
	if (scaleCurvePlayer.curve != NULL && !scaleCurvePlayer.curve->empty())
		flags |= AnimationUpdateFlags::Sca;
	return flags;
}

Enum<AnimationUpdateFlags> TransformAnimationPlayer::update(float deltaTime)
{
	Enum<AnimationUpdateFlags> updateFlags = AnimationUpdateFlags::None;
	if (transformAnimationData != NULL) {
		Vector3f pos = positionCurvePlayer.update(deltaTime);
		if (positionCurvePlayer.playing()) {
			transformData.position = pos;
			if (targetTransform != NULL)
				targetTransform->setPosition(pos);
			updateFlags |= AnimationUpdateFlags::Pos;
		}
		Quaternionf rot = rotationCurvePlayer.update(deltaTime);
		if (rotationCurvePlayer.playing()) {
			transformData.rotation = rot;
			if (targetTransform != NULL)
				targetTransform->setRotation(rot);
			updateFlags |= AnimationUpdateFlags::Rot;
		}
		Vector3f sca = scaleCurvePlayer.update(deltaTime);
		if (scaleCurvePlayer.playing()) {
			transformData.scale = sca;
			if (targetTransform != NULL)
				targetTransform->setScale(sca);
			updateFlags |= AnimationUpdateFlags::Sca;
		}
	}
	return updateFlags;
}

bool TransformAnimationPlayer::isLoop()
{
	return positionCurvePlayer.loop ||
		rotationCurvePlayer.loop ||
		scaleCurvePlayer.loop;
}

void TransformAnimationPlayer::setLoop(bool loop)
{
	if (transformAnimationData != NULL) {
		positionCurvePlayer.loop = loop;
		rotationCurvePlayer.loop = loop;
		scaleCurvePlayer.loop = loop;
	}
}

void TransformAnimationPlayer::setTime(float time)
{
	if (transformAnimationData != NULL) {
		Vector3f pos = positionCurvePlayer.setTime(time);
		transformData.position = pos;
		if (targetTransform != NULL)
			targetTransform->setPosition(pos);

		Quaternionf rot = rotationCurvePlayer.setTime(time);
		transformData.rotation = rot;
		if (targetTransform != NULL)
			targetTransform->setRotation(rot);

		Vector3f scale = scaleCurvePlayer.setTime(time);
		transformData.scale = scale;
		if (targetTransform != NULL)
			targetTransform->setScale(scale);
	}
}

void TransformAnimationPlayer::play()
{
	if (transformAnimationData != NULL) {
		positionCurvePlayer.play();
		rotationCurvePlayer.play();
		scaleCurvePlayer.play();
	}
}

void TransformAnimationPlayer::pause()
{
	if (transformAnimationData != NULL) {
		positionCurvePlayer.pause();
		rotationCurvePlayer.pause();
		scaleCurvePlayer.pause();
	}
}

void TransformAnimationPlayer::stop()
{
	if (transformAnimationData != NULL) {
		positionCurvePlayer.stop();
		rotationCurvePlayer.stop();
		scaleCurvePlayer.stop();
	}
}

bool TransformAnimationPlayer::playing()
{
	return positionCurvePlayer.playing() ||
		rotationCurvePlayer.playing() ||
		scaleCurvePlayer.playing();
}

void TransformAnimationPlayer::setSpeed(float speed)
{
	if (transformAnimationData != NULL) {
		positionCurvePlayer.setSpeed(speed);
		rotationCurvePlayer.setSpeed(speed);
		scaleCurvePlayer.setSpeed(speed);
	}
}

void TransformAnimationPlayer::reset()
{
	if (transformAnimationData != NULL) {
		positionCurvePlayer.setCurve(transformAnimationData->positionCurve);
		rotationCurvePlayer.setCurve(transformAnimationData->rotationCurve);
		scaleCurvePlayer.setCurve(transformAnimationData->scaleCurve);
	}
}

AnimationClipData::AnimationClipData(const string & name) : name(name)
{
}

bool AnimationClipData::canApply(Skeleton & skeleton)
{
	/*if (skeletonData.boneName.size() != transformAnimationNames.size())
		return false;
	for (auto b = transformAnimationNames.begin(), b1 = skeletonData.boneName.begin(),
		e = transformAnimationNames.end(); b != e; b++, b1++)
		if (b->first != b1->first || b->second != b1->second)
			return false;*/
	return true;
}

bool AnimationClipData::canBlend(AnimationClipData & data)
{
	/*if (data.transformAnimationNames.size() != transformAnimationNames.size())
		return false;
	for (auto b = transformAnimationNames.begin(), b1 = data.transformAnimationNames.begin(),
		e = transformAnimationNames.end(); b != e; b++, b1++)
		if (b->first != b1->first || b->second != b1->second)
			return false;*/
	return true;
}

TransformAnimationData & AnimationClipData::addAnimatinData(const string & objectName)
{
	auto iter = transformAnimationNames.find(objectName);
	if (iter == transformAnimationNames.end()) {
		transformAnimationNames.insert(pair<string, unsigned int>(objectName, transformAnimationDatas.size()));
		transformAnimationDatas.push_back(TransformAnimationData());
		transformAnimationDatas.back().objectName = objectName;
		return transformAnimationDatas.back();
	}
	return transformAnimationDatas[iter->second];
}

TransformAnimationData * AnimationClipData::getAnimatinData(const string & objectName)
{
	auto iter = transformAnimationNames.find(objectName);
	if (iter != transformAnimationNames.end()) {
		return &transformAnimationDatas[iter->second];
	}
	return NULL;
}

unsigned int AnimationClipData::getAnimatinDataIndex(const string & objectName)
{
	auto iter = transformAnimationNames.find(objectName);
	if (iter != transformAnimationNames.end()) {
		return iter->second;
	}
	return -1;
}

TransformAnimationData * AnimationClipData::getAnimatinData(unsigned int index)
{
	if (index < transformAnimationDatas.size())
		return &transformAnimationDatas[index];
	return NULL;
}

Curve<float, float> & AnimationClipData::addMorphAnimationData(unsigned int index)
{
	auto iter = curves.find(index);
	if (iter == curves.end())
		return curves.insert(pair<unsigned int, Curve<float, float>>(index, Curve<float, float>())).first->second;
	else
		return iter->second;
}

Curve<float, float>& AnimationClipData::addMorphAnimationData(const string & name)
{
	auto iter = curveNames.find(name);
	if (iter == curveNames.end()) {
		unsigned int index = curveNames.size();
		curveNames.insert(make_pair(name, index));
		return addMorphAnimationData(index);
	}
	else {
		auto _iter = curves.find(iter->second);
		if (_iter == curves.end())
			return curves.insert(pair<unsigned int, Curve<float, float>>(iter->second, Curve<float, float>())).first->second;
		else
			return _iter->second;
	}
}

Curve<float, float>* AnimationClipData::getMorphAnimationData(unsigned int index)
{
	auto iter = curves.find(index);
	if (iter == curves.end())
		return NULL;
	else
		return &iter->second;
}

Curve<float, float>* AnimationClipData::getMorphAnimationData(const string & name)
{
	auto iter = curveNames.find(name);
	if (iter == curveNames.end())
		return NULL;
	else
		return getMorphAnimationData(iter->second);
}

void AnimationClipData::pack()
{
	duration = 0;
	for (int i = 0; i < transformAnimationDatas.size(); i++) {
		duration = max(transformAnimationDatas[i].positionCurve.duration, duration);
		duration = max(transformAnimationDatas[i].rotationCurve.duration, duration);
		duration = max(transformAnimationDatas[i].scaleCurve.duration, duration);
	}
	for (auto b = curves.begin(), e = curves.end(); b != e; b++) {
		duration = max(b->second.duration, duration);
	}
	for (int i = 0; i < transformAnimationDatas.size(); i++) {
		transformAnimationDatas[i].positionCurve.duration = duration;
		transformAnimationDatas[i].rotationCurve.duration = duration;
		transformAnimationDatas[i].scaleCurve.duration = duration;
	}
	for (auto b = curves.begin(), e = curves.end(); b != e; b++) {
		b->second.duration = duration;
	}
}

bool AnimationClipData::read(istream & in)
{
	const string magic = "BEAnim";
	for (int i = 0; i < magic.length(); i++) {
		uint8_t c;
		in.read((char*)&c, sizeof(uint8_t));
		if (magic[i] != c) {
			in.seekg(0, ios::beg);
			return false;
		}
	}
	uint32_t boneLen = 0;
	in.read((char*)&boneLen, sizeof(uint32_t));
	for (int i = 0; i < boneLen; i++) {
		TransformAnimationData& data = addAnimatinData(readString(in));
		data.read(in);
	}
	uint32_t morphNameLen = 0;
	in.read((char*)&morphNameLen, sizeof(uint32_t));
	for (int i = 0; i < morphNameLen; i++) {
		string name = readString(in);
		uint32_t index;
		in.read((char*)&index, sizeof(uint32_t));
		curveNames.insert(make_pair(name, index));
	}
	uint32_t morphLen = 0;
	in.read((char*)&morphLen, sizeof(uint32_t));
	for (int i = 0; i < morphLen; i++) {
		uint32_t index;
		in.read((char*)&index, sizeof(uint32_t));
		Curve<float, float>& curve = curves.insert(make_pair(index, Curve<float, float>())).first->second;
		uint32_t curveLen;
		in.read((char*)&curveLen, sizeof(uint32_t));
		for (int ci = 0; ci < curveLen; ci++) {
			float_t frame;
			in.read((char*)&frame, sizeof(float_t));
			float_t weight;
			in.read((char*)&weight, sizeof(float_t));
			uint8_t mode;
			in.read((char*)&mode, sizeof(uint8_t));
			float_t inTangent;
			in.read((char*)&inTangent, sizeof(float_t));
			float_t inRate;
			in.read((char*)&inRate, sizeof(float_t));
			float_t outTangent;
			in.read((char*)&outTangent, sizeof(float_t));
			float_t outRate;
			in.read((char*)&outRate, sizeof(float_t));
			CurveValue<float> value = CurveValue<float>((CurveValue<float>::KeySmoothMode)mode, weight);
			value.inTangent = inTangent;
			value.inRate = inRate;
			value.outTangent = outTangent;
			value.outRate = outRate;
			curve.insert(frame, value);
		}
	}
	pack();
	return true;
}

void AnimationClipData::write(ostream& out, LongProgressWork* work) const
{
	writeString("BEAnim", out, false);
	uint32_t boneLen = transformAnimationDatas.size();
	uint32_t morphNameLen = curveNames.size();
	uint32_t morphLen = curves.size();

	float totalSteps = boneLen + morphLen + 1;
	int step = 0;

	out.write((char*)&boneLen, sizeof(uint32_t));
	for (int i = 0; i < boneLen; i++) {
		const TransformAnimationData& data = transformAnimationDatas[i];
		if (work)
			work->setProgress(step / totalSteps, "Write Bone " + data.objectName);
		writeString(data.objectName, out);
		data.write(out);
		step++;
	}
	if (work)
		work->setProgress(step / totalSteps, "Write Morph Names");
	out.write((char*)&morphNameLen, sizeof(uint32_t));
	for (auto b = curveNames.begin(), e = curveNames.end(); b != e; b++) {
		writeString(b->first, out);
		uint32_t index = b->second;
		out.write((char*)&index, sizeof(uint32_t));
	}
	out.write((char*)&morphLen, sizeof(uint32_t));
	for (auto b = curves.begin(), e = curves.end(); b != e; b++) {
		uint32_t index = b->first;
		if (work)
			work->setProgress(step / totalSteps, "Write Morph " + to_string(index));
		out.write((char*)&index, sizeof(uint32_t));
		const Curve<float, float>& curve = b->second;
		uint32_t curveLen = curve.keys.size();
		out.write((char*)&curveLen, sizeof(uint32_t));
		for (auto cb = curve.keys.begin(), ce = curve.keys.end(); cb != ce; cb++) {
			float_t frame = cb->first;
			out.write((char*)&frame, sizeof(float_t));
			float_t weight = cb->second.value;
			out.write((char*)&weight, sizeof(float_t));
			uint8_t mode = cb->second.mode;
			out.write((char*)&mode, sizeof(uint8_t));
			float_t inTangent = cb->second.inTangent;
			out.write((char*)&inTangent, sizeof(float_t));
			float_t inRate = cb->second.inRate;
			out.write((char*)&inRate, sizeof(float_t));
			float_t outTangent = cb->second.outTangent;
			out.write((char*)&outTangent, sizeof(float_t));
			float_t outRate = cb->second.outRate;
			out.write((char*)&outRate, sizeof(float_t));
		}
		step++;
	}
	if (work)
		work->setProgress(1, "Complete");
}

SerializeInstance(AnimationBase);

bool AnimationBase::setupContext(const AnimationContext& context)
{
	animationPose.setContext(context);
	return true;
}

AnimationPose& AnimationBase::getPose()
{
	return animationPose;
}

Enum<AnimationUpdateFlags> AnimationBase::getUpdateFlags() const
{
	return updateFlags;
}

AnimationClip::AnimationClip()
{
}

AnimationClip::AnimationClip(AnimationClipData & clipData) : animationClipData(&clipData)
{
	reset();
}

bool AnimationClip::isValid()
{
	return animationClipData != NULL;
}

bool AnimationClip::setupDefault()
{
	if (animationClipData == NULL)
		return false;
	animationPose.transformData.resize(animationClipData->transformAnimationDatas.size());
	animationPose.morphTargetWeight.resize(animationClipData->curves.size());
	boneChannelMap.resize(animationPose.transformData.size());
	curveChannelMap.resize(animationPose.morphTargetWeight.size());
	for (int i = 0; i < boneChannelMap.size(); i++) {
		boneChannelMap[i] = i;
	}
	int i = 0;
	for (auto b = animationClipData->curves.begin();
		i < curveChannelMap.size(); b++, i++) {
		curveChannelMap[i] = b->first;
	}
	return true;
}

bool AnimationClip::setupContext(const AnimationContext& context)
{
	AnimationBase::setupContext(context);
	bindPose.setContext(context);
	bindPose.resetToBindPose();
	remapChannel();
	return true;
}

void AnimationClip::remapChannel()
{
	boneChannelMap.resize(animationPose.transformData.size(), -1);
	curveChannelMap.resize(animationPose.morphTargetWeight.size(), -1);
	if (animationClipData == NULL)
		return;
	if (animationPose.skeleton != NULL)
		for (int i = 0; i < boneChannelMap.size(); i++) {
			Skeleton::BoneInfo* info = animationPose.skeleton->getBone(i);
			if (info == NULL)
				boneChannelMap[i] = -1;
			else
				boneChannelMap[i] = animationClipData->getAnimatinDataIndex(info->data->name);
		}

	if (animationPose.morphTargetRemapper != NULL)
		for (int i = 0; i < curveChannelMap.size(); i++) {
			auto iter = animationClipData->curveNames.find(animationPose.morphTargetRemapper->morphNames[i]);
			if (iter == animationClipData->curveNames.end()) {
				curveChannelMap[i] = -1;
			}
			else {
				curveChannelMap[i] = iter->second;
			}
		}
}

void AnimationClip::setAnimationClipData(AnimationClipData* clipData)
{
	animationClipData = clipData;
	reset();
}

bool AnimationClip::setTargetTransform(Transform & target)
{
	if (animationClipData == NULL)
		return false;
	unsigned int id = animationClipData->getAnimatinDataIndex(target.name);
	if (id == -1)
		return false;
	animationPlayer[id].setTargetTransform(target);
	return true;
}

bool AnimationClip::setTargetTransform(const string & name, Transform & target)
{
	if (animationClipData == NULL)
		return false;
	unsigned int id = animationClipData->getAnimatinDataIndex(name);
	if (id == -1)
		return false;
	animationPlayer[id].setTargetTransform(target);
	return true;
}

Enum<AnimationUpdateFlags> AnimationClip::update(float deltaTime)
{
	if (animationClipData == NULL)
		return false;
	updateFlags = AnimationUpdateFlags::None;
	for (int i = 0; i < animationPlayer.size(); i++) {
		updateFlags |= animationPlayer[i].update(deltaTime);
	}
	for (int i = 0; i < boneChannelMap.size(); i++) {
		int index = boneChannelMap[i];
		if (index >= 0) {
			animationPose.transformData[i] = animationPlayer[index].getTransformData();
		}
		else {
			animationPose.transformData[i] = bindPose.transformData[i];
		}
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		float weight = b->second.update(deltaTime);
		curveCurrentValue[b->first] = weight;
		if (b->second.isPlaying)
			updateFlags |= AnimationUpdateFlags::Morph;
	}
	auto end = curveCurrentValue.end();
	for (int i = 0; i < curveChannelMap.size(); i++) {
		int index = curveChannelMap[i];
		if (index >= 0) {
			auto iter = curveCurrentValue.find(index);
			if (iter == end)
				animationPose.morphTargetWeight[i] = 0;
			else
				animationPose.morphTargetWeight[i] = iter->second;
		}
	}
	return updateFlags;
}

void AnimationClip::getTransformData(vector<TransformData>& data)
{
	int s = animationPlayer.size();
	data.resize(s);
	for (int i = 0; i < s; i++)
		data[i] = animationPlayer[i].getTransformData();
}

TransformData * AnimationClip::getTransformData(const string & name)
{
	if (animationClipData == NULL)
		return NULL;
	unsigned int index = animationClipData->getAnimatinDataIndex(name);
	if (index >= animationPlayer.size())
		return NULL;
	return &animationPlayer[index].getTransformData();
}

bool AnimationClip::isLoop()
{
	if (animationClipData == NULL)
		return false;
	bool _loop = false;
	for (int i = 0; i < animationPlayer.size(); i++) {
		if (animationPlayer[i].isLoop()) {
			_loop = true;
			break;
		}
	}
	if (!_loop)
		for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
			if (b->second.loop) {
				_loop = true;
				break;
			}
		}
	return _loop;
}

void AnimationClip::setLoop(bool loop)
{
	if (animationClipData == NULL)
		return;
	for (int i = 0; i < animationPlayer.size(); i++) {
		animationPlayer[i].setLoop(loop);
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		b->second.loop = loop;
	}
}

void AnimationClip::setTime(float time)
{
	if (animationClipData == NULL)
		return;
	updateFlags = AnimationUpdateFlags::None;
	for (int i = 0; i < animationPlayer.size(); i++) {
		animationPlayer[i].setTime(time);
		updateFlags |= animationPlayer[i].getUpdateFlags();
	}
	if (!curvePlayer.empty())
		updateFlags |= AnimationUpdateFlags::Morph;
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		b->second.setTime(time);
	}
}

float AnimationClip::getTime() const
{
	if (animationClipData == NULL || animationPlayer.empty())
		return 0.0f;
	float time = 0;
	for (int i = 0; i < animationPlayer.size(); i++) {
		const TransformAnimationPlayer& player = animationPlayer[i];
		time = player.positionCurvePlayer.keyCount > time ?
			player.positionCurvePlayer.keyCount : time;
		time = player.rotationCurvePlayer.keyCount > time ?
			player.rotationCurvePlayer.keyCount : time;
		time = player.scaleCurvePlayer.keyCount > time ?
			player.scaleCurvePlayer.keyCount : time;
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		const auto &player = b->second;
		time = player.keyCount > time ? player.keyCount : time;
	}
	return time;
}

void AnimationClip::play()
{
	if (animationClipData == NULL)
		return;
	for (int i = 0; i < animationPlayer.size(); i++) {
		animationPlayer[i].play();
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		b->second.play();
	}
}

void AnimationClip::pause()
{
	if (animationClipData == NULL)
		return;
	for (int i = 0; i < animationPlayer.size(); i++) {
		animationPlayer[i].pause();
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		b->second.pause();
	}
}

void AnimationClip::stop()
{
	if (animationClipData == NULL)
		return;
	for (int i = 0; i < animationPlayer.size(); i++) {
		animationPlayer[i].stop();
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		b->second.stop();
	}
}

bool AnimationClip::playing()
{
	if (animationClipData == NULL)
		return false;
	for (int i = 0; i < animationPlayer.size(); i++) {
		if (animationPlayer[i].playing())
			return true;
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		if (b->second.isPlaying)
			return true;
	}
	return false;
}

void AnimationClip::setSpeed(float speed)
{
	if (animationClipData == NULL)
		return;
	for (int i = 0; i < animationPlayer.size(); i++) {
		animationPlayer[i].setSpeed(speed);
	}
	for (auto b = curvePlayer.begin(), e = curvePlayer.end(); b != e; b++) {
		b->second.setSpeed(speed);
	}
}

void AnimationClip::reset()
{
	if (animationClipData != NULL) {
		if (animationClipData->transformAnimationDatas.size() != animationPlayer.size())
			animationPlayer.resize(animationClipData->transformAnimationDatas.size());
		for (int i = 0; i < animationPlayer.size(); i++) {
			animationPlayer[i].setAnimationData(animationClipData->transformAnimationDatas[i]);
		}
		for (auto b = animationClipData->curves.begin(), e = animationClipData->curves.end(); b != e; b++) {
			auto iterp = curvePlayer.find(b->first);
			if (iterp == curvePlayer.end())
				curvePlayer.insert(pair<unsigned int, CurvePlayer<float, float>>(b->first, CurvePlayer<float, float>(b->second)));
			else
				iterp->second.setCurve(b->second);
			auto iterv = curveCurrentValue.find(b->first);
			if (iterv == curveCurrentValue.end())
				curveCurrentValue.insert(pair<unsigned int, float>(b->first, 0));
			else
				iterv->second = 0;
		}
	}
	remapChannel();
}

SerializeInstance(BlendSpaceAnimation);

BlendSpaceAnimation::BlendSpaceAnimation()
{
}

bool BlendSpaceAnimation::isValid()
{
	return animationPose.skeleton != NULL;
}

bool BlendSpaceAnimation::setupContext(const AnimationContext& context)
{
	AnimationBase::setupContext(context);
	for each (auto item in animationClipWrap)
	{
		item.second->setupContext(context);
	}
	return true;
}

bool BlendSpaceAnimation::addAnimationClipData(const Vector2f & pos, AnimationClipData & clipData)
{
	if (!isValid())
		return false;
	if (!animationClipWrap.empty()) {
		AnimationClipData& data = *animationClipWrap.back().second->animationClipData;
		if (!data.canBlend(clipData))
			return false;
	}
	stop();
	animationClipWrap.push_back(make_pair(pos, new AnimationClip()));
	animationClipWrap.back().second->setAnimationClipData(&clipData);
	animationClipWrap.back().second->setLoop(true);
	setBlendWeight(blendWeight);
	return true;
}

bool BlendSpaceAnimation::removeAnimationClipData(unsigned int index)
{
	if (index < animationClipWrap.size()) {
		delete animationClipWrap[index].second;
		animationClipWrap.erase(animationClipWrap.begin() + index);
		setBlendWeight(blendWeight);
		return true;
	}
	return false;
}

void BlendSpaceAnimation::removeAllAnimationClipData()
{
	stop();
	for (int i = 0; i < animationClipWrap.size(); i++) {
		delete animationClipWrap[i].second;
	}
	animationClipWrap.clear();
	animationWeights.clear();
}

void BlendSpaceAnimation::setBlendWeight(const Vector2f & weight)
{
	blendWeight = weight;
	getWeights(animationWeights);
}

float clamp(float v, float a, float b) {
	float t = v > a ? v : a;
	return t < b ? t : b;
}

float dot(const Vector2f& a, const Vector2f& b) {
	return a.x() * b.x() + a.y() * b.y();
}

unsigned int BlendSpaceAnimation::getWeights(vector<pair<unsigned int, float>>& weights)
{
	weights.clear();
	float total_weight = 0.0;
	unsigned int POINT_COUNT = animationClipWrap.size();

	for (int i = 0; i < POINT_COUNT; ++i)
	{
		// Calc vec i -> sample
		Vector2f    point_i = animationClipWrap[i].first;
		Vector2f    vec_is = blendWeight - point_i;

		float   weight = 1.0;

		for (int j = 0; j < POINT_COUNT; ++j)
		{
			if (j == i)
				continue;

			// Calc vec i -> j
			Vector2f    point_j = animationClipWrap[j].first;
			Vector2f    vec_ij = point_j - point_i;

			// Calc Weight
			float lensq_ij = dot(vec_ij, vec_ij);
			float new_weight = dot(vec_is, vec_ij) / lensq_ij;
			new_weight = 1.0 - new_weight;
			new_weight = clamp(new_weight, 0, 1);

			weight = weight < new_weight ? weight : new_weight;
		}
		if (weight > 0)
			weights.emplace_back(make_pair(i, weight));
		total_weight += weight;
	}
	float maxW = 0;
	unsigned int maxI = -1;
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i].second /= total_weight;
		if (weights[i].second > maxW) {
			maxI = weights[i].second;
			maxI = i;
		}
	}
	return maxI;
}

Enum<AnimationUpdateFlags> BlendSpaceAnimation::update(float deltaTime)
{
	updateFlags = AnimationUpdateFlags::None;
	if (!isValid() || animationClipWrap.empty() || animationWeights.empty())
		return updateFlags;
	float duration = 0;
	for (int i = 0; i < animationWeights.size(); i++) {
		AnimationClip* clip = animationClipWrap[animationWeights[i].first].second;
		duration += clip->animationClipData->duration * animationWeights[i].second;
	}
	for (int i = 0; i < animationClipWrap.size(); i++) {
		AnimationClip* clip = animationClipWrap[i].second;
		float timeScale = clip->animationClipData->duration / duration;
		updateFlags |= clip->update(timeScale * deltaTime);
	}
	if (updateFlags == Enum<AnimationUpdateFlags>(AnimationUpdateFlags::None))
		return updateFlags;
	int size = transformDatas.size();

	const int numClips = animationWeights.size();
	AnimationClip* clip = animationClipWrap[animationWeights[0].first].second;
	animationPose = clip->getPose();
	float sumWeight = animationWeights[0].second;
	for (int i = 1; i < numClips; i++) {
		float weight = animationWeights[i].second;
		if (weight == 0)
			continue;
		sumWeight += weight;
		clip = animationClipWrap[animationWeights[i].first].second;
		animationPose.lerp(clip->getPose(), weight / sumWeight);
	}

	/*auto getTransformData = [&](int boneIndex, int weightIndex, TransformData& data)
	{
		Skeleton::BoneInfo* bone = skeleton->getBone(boneIndex);
		TransformData* pd = animationClipWrap[animationWeights[weightIndex].first].second->getTransformData(bone->data->name);
		if (pd == NULL) {
			Matrix4f& tm = bone->data->transformMatrix;
			tm.decompose(data.position, data.rotation, data.scale);
		}
		else
			data = *pd;
	};

	for (int t = 0; t < size; t++) {
		Skeleton::BoneInfo* bone = skeleton->getBone(t);
		TransformData data;
		TransformData bondPos;
		data.scale = { 0, 0, 0 };
		if (animationWeights.empty()) {
			Matrix4f& tm = bone->data->transformMatrix;
			tm.decompose(data.position, data.rotation, data.scale);
		}
		else if (animationWeights.size() == 1) {
			getTransformData(t, 0, data);
		}
		else {
			getTransformData(t, 0, data);
			float sumWeight = animationWeights[0].second;

			for (int i = 1; i < animationWeights.size(); i++) {
				TransformData d;
				getTransformData(t, i, d);
				float weight = animationWeights[i].second;
				sumWeight += weight;
				data.lerp(d, weight / sumWeight);
			}
		}
		transformDatas[t] = data;
		Transform* trans = targetTransforms[t];
		if (trans != NULL) {
			trans->setPosition(data.position);
			trans->setRotation(data.rotation);
			trans->setScale(data.scale);
		}
	}*/
	return updateFlags;
}

vector<TransformData> & BlendSpaceAnimation::getTransformData()
{
	return transformDatas;
}

bool BlendSpaceAnimation::isLoop()
{
	if (!isValid() || animationClipWrap.empty())
		return false;
	for (int i = 0; i < animationClipWrap.size(); i++) {
		if (animationClipWrap[i].second->isLoop())
			return true;
	}
	return false;
}

void BlendSpaceAnimation::setLoop(bool loop)
{
	for (int i = 0; i < animationClipWrap.size(); i++)
		animationClipWrap[i].second->setLoop(loop);
}

void BlendSpaceAnimation::play()
{
	for (int i = 0; i < animationClipWrap.size(); i++)
		animationClipWrap[i].second->play();
}

void BlendSpaceAnimation::pause()
{
	for (int i = 0; i < animationClipWrap.size(); i++)
		animationClipWrap[i].second->pause();
}

void BlendSpaceAnimation::stop()
{
	for (int i = 0; i < animationClipWrap.size(); i++)
		animationClipWrap[i].second->stop();
}

bool BlendSpaceAnimation::playing()
{
	if (!isValid() || animationClipWrap.empty())
		return false;
	for (int i = 0; i < animationClipWrap.size(); i++) {
		if (animationClipWrap[i].second->playing())
			return true;
	}
	return false;
}

void BlendSpaceAnimation::setSpeed(float speed)
{
	for (int i = 0; i < animationClipWrap.size(); i++)
		animationClipWrap[i].second->setSpeed(speed);
}

void BlendSpaceAnimation::reset()
{
	for (int i = 0; i < animationClipWrap.size(); i++)
		animationClipWrap[i].second->reset();
}

Serializable * BlendSpaceAnimation::instantiate(const SerializationInfo & from)
{
	return new BlendSpaceAnimation();
}

bool BlendSpaceAnimation::deserialize(const SerializationInfo & from)
{
	from.get("name", name);
	const SerializationInfo* clipWrapInfo = from.get("clipWrap");
	if (clipWrapInfo == NULL)
		return false;
	removeAllAnimationClipData();
	for (auto b = clipWrapInfo->sublists.begin(), e = clipWrapInfo->sublists.end(); b != e; b++) {
		SVector2f pos;
		if (!b->get("pos", pos))
			continue;
		string clipDataPath;
		if (!b->get("clipData", clipDataPath))
			continue;
		AnimationClipData* clipData = getAssetByPath<AnimationClipData>(clipDataPath);
		if (clipData != NULL)
			addAnimationClipData(pos, *clipData);
	}
	return true;
}

bool BlendSpaceAnimation::serialize(SerializationInfo & to)
{
	AnimationBase::serialize(to);
	to.set("name", name);
	SerializationInfo* clipWrapInfo = to.addArray("clipWrap", "Clip");
	for (auto b = animationClipWrap.begin(), e = animationClipWrap.end(); b != e; b++) {
		SerializationInfo* data = clipWrapInfo->push();
		data->set("pos", (SVector2f)b->first);
		string clipDataPath = AssetInfo::getPath(b->second->animationClipData);
		data->set("clipData", clipDataPath);
	}
	return true;
}

AnimationClipData * AnimationLoader::loadCameraAnimation(const string & file)
{
	ifstream f(filesystem::u8path(file));
	if (f.fail()) {
		f.close();
		return NULL;
	}
	SerializationInfoParser sp(f);
	bool ok = sp.parse();
	f.close();
	if (!ok)
		return NULL;
	SerializationInfo& info = sp.infos[0];
	struct CameraFrame
	{
		float frame;
		SVector3f position;
		SVector3f positionInTan;
		SVector3f positionInRate;
		SVector3f positionOutTan;
		SVector3f positionOutRate;
		SVector3f rotation;
		float rotationInTan;
		float rotationInRate;
		float rotationOutTan;
		float rotationOutRate;
		float distance;
		float distanceInTan;
		float distanceInRate;
		float distanceOutTan;
		float distanceOutRate;
		float fov;
		float fovInTan;
		float fovInRate;
		float fovOutTan;
		float fovOutRate;
	};
	filesystem::path p = file;
	string ext = p.extension().generic_string();
	string name = p.filename().generic_string();
	name = name.substr(0, name.size() - ext.size());
	AnimationClipData* data = new AnimationClipData(name);
	TransformAnimationData& tranData = data->addAnimatinData("Camera");
	Curve<float, float>& fovCurve = data->addMorphAnimationData(0);
	Curve<float, float>& disCurve = data->addMorphAnimationData(1);
	float maxFrame = 0;
	for (int i = 0; i < info.sublists.size(); i++) {
		SerializationInfo& fi = info.sublists[i];
		CameraFrame f;
		fi.get("frame", f.frame);
		fi.get("position", f.position);
		fi.get("positionInTan", f.positionInTan);
		fi.get("positionInRate", f.positionInRate);
		fi.get("positionOutTan", f.positionOutTan);
		fi.get("positionOutRate", f.positionOutRate);
		fi.get("rotation", f.rotation);
		fi.get("rotationInTan", f.rotationInTan);
		fi.get("rotationInRate", f.rotationInRate);
		fi.get("rotationOutTan", f.rotationOutTan);
		fi.get("rotationOutRate", f.rotationOutRate);
		fi.get("distance", f.distance);
		fi.get("distanceInTan", f.distanceInTan);
		fi.get("distanceInRate", f.distanceInRate);
		fi.get("distanceOutTan", f.distanceOutTan);
		fi.get("distanceOutRate", f.distanceOutRate);
		fi.get("fov", f.fov);
		fi.get("fovInTan", f.fovInTan);
		fi.get("fovInRate", f.fovInRate);
		fi.get("fovOutTan", f.fovOutTan);
		fi.get("fovOutRate", f.fovOutRate);
		if (maxFrame < f.frame)
			maxFrame = f.frame;
		tranData.positionCurve.insert(f.frame, CurveValue<Vector3f>(f.position, f.positionInTan, f.positionOutTan, f.positionInRate, f.positionOutRate));
		Quaternionf R = Quaternionf::FromAngleAxis(f.rotation.x / 180.0 * PI, Vector3f::UnitX()) *
			Quaternionf::FromAngleAxis(f.rotation.y / 180.0 * PI, Vector3f::UnitY()) *
			Quaternionf::FromAngleAxis(f.rotation.z / 180.0 * PI, Vector3f::UnitZ());
		tranData.rotationCurve.insert(f.frame, CurveValue<Quaternionf>(R, f.rotationInTan, f.rotationOutTan, f.rotationInRate, f.rotationOutRate));
		disCurve.insert(f.frame, CurveValue<float>(f.distance, f.distanceInTan, f.distanceOutTan, f.distanceInRate, f.distanceOutRate));
		fovCurve.insert(f.frame, CurveValue<float>(f.fov, f.fovInTan, f.fovOutTan, f.fovInRate, f.fovOutRate));
	}
	tranData.positionCurve.duration = maxFrame;
	tranData.rotationCurve.duration = maxFrame;
	fovCurve.duration = maxFrame;
	disCurve.duration = maxFrame;
	data->duration = maxFrame;
	data->pack();
	return data;
}

AnimationClipData * AnimationLoader::loadMotionAnimation(const string & file)
{
	ifstream f(file);
	if (f.fail()) {
		f.close();
		return NULL;
	}
	SerializationInfoParser sp(f);
	bool ok = sp.parse();
	f.close();
	if (!ok)
		return NULL;
	SerializationInfo& info = sp.infos[0];
	struct BoneFrame
	{
		float frame;
		SVector3f position;
		SVector3f positionInTan;
		SVector3f positionInRate;
		SVector3f positionOutTan;
		SVector3f positionOutRate;
		SQuaternionf rotation;
		float rotationInTan;
		float rotationInRate;
		float rotationOutTan;
		float rotationOutRate;
	};
	struct MorphFrame
	{
		float frame;
		float weight;
	};
	filesystem::path p = file;
	string ext = p.extension().generic_string();
	string name = p.filename().generic_string();
	name = name.substr(0, name.size() - ext.size());
	AnimationClipData* data = new AnimationClipData(name);
	float maxFrame = 0;
	const SerializationInfo* boneFrames = info.get("boneFrames");
	if (boneFrames != NULL) {
		for (int i = 0; i < boneFrames->sublists.size(); i++) {
			bool hasPos = false;
			bool hasRot = false;
			const SerializationInfo& ffi = boneFrames->sublists[i];
			TransformAnimationData& tranData = data->addAnimatinData(ffi.name);
			for (int t = 0; t < ffi.sublists.size(); t++) {
				const SerializationInfo& fi = ffi.sublists[t];
				BoneFrame f;
				fi.get("frame", f.frame);
				if (fi.get("position", f.position)) {
					bool hasInterp = true;
					hasPos = true;
					hasInterp &= fi.get("positionInTan", f.positionInTan);
					hasInterp &= fi.get("positionInRate", f.positionInRate);
					hasInterp &= fi.get("positionOutTan", f.positionOutTan);
					hasInterp &= fi.get("positionOutRate", f.positionOutRate);
					if (hasInterp)
						tranData.positionCurve.insert(f.frame, CurveValue<Vector3f>(f.position, f.positionInTan, f.positionOutTan, f.positionInRate, f.positionOutRate));
					else
						tranData.positionCurve.insert(f.frame, CurveValue<Vector3f>(CurveValue<Vector3f>::Linear, f.position));
				}
				if (fi.get("rotation", f.rotation)) {
					bool hasInterp = true;
					hasRot = true;
					hasInterp &= fi.get("rotationInTan", f.rotationInTan);
					hasInterp &= fi.get("rotationInRate", f.rotationInRate);
					hasInterp &= fi.get("rotationOutTan", f.rotationOutTan);
					hasInterp &= fi.get("rotationOutRate", f.rotationOutRate);
					if (hasInterp)
						tranData.rotationCurve.insert(f.frame, CurveValue<Quaternionf>(f.rotation, f.rotationInTan, f.rotationOutTan, f.rotationInRate, f.rotationOutRate));
					else
						tranData.rotationCurve.insert(f.frame, CurveValue<Quaternionf>(CurveValue<Quaternionf>::Linear, f.rotation));
				}
				if (maxFrame < f.frame)
					maxFrame = f.frame;
			}
		}
		for (int i = 0; i < data->transformAnimationDatas.size(); i++) {
			TransformAnimationData& animData = data->transformAnimationDatas[i];
			animData.positionCurve.duration = maxFrame;
			animData.rotationCurve.duration = maxFrame;
		}
	}
	const SerializationInfo* morphFrames = info.get("morphFrames");
	if (morphFrames != NULL) {
		for (int i = 0; i < morphFrames->sublists.size(); i++) {
			const SerializationInfo& ffi = morphFrames->sublists[i];
			Curve<float, float>& morphData = data->addMorphAnimationData(ffi.name);
			for (int t = 0; t < ffi.sublists.size(); t++) {
				const SerializationInfo& fi = ffi.sublists[t];
				MorphFrame f;
				fi.get("frame", f.frame);
				fi.get("weight", f.weight);
				morphData.insert(f.frame, CurveValue<float>(CurveValue<float>::Linear, f.weight));
			}
		}
		for (auto b = data->curves.begin(), e = data->curves.end(); b != e; b++)
			b->second.duration = maxFrame;
		data->duration = maxFrame;
	}
	data->pack();
	return data;
}

AnimationClipData * AnimationLoader::readAnimation(const string & file)
{
	ifstream f(file, ios::binary | ios::in);
	if (f.fail()) {
		f.close();
		return NULL;
	}
	filesystem::path p = file;
	string ext = p.extension().generic_string();
	string name = p.filename().generic_string();
	name = name.substr(0, name.size() - ext.size());
	AnimationClipData* data = new AnimationClipData(name);
	if (!data->read(f)) {
		delete data;
		f.close();
		return NULL;
	}
	f.close();
	return data;
}

bool AnimationLoader::writeAnimation(const AnimationClipData& data, const string & file, LongProgressWork* work)
{
	ofstream f(file, ios::binary | ios::out);
	if (f.fail()) {
		f.close();
		return false;
	}
	data.write(f, work);
	f.close();
	return true;
}

void TransformAnimationData::read(istream & in)
{
	float_t maxFrame = 0;
	uint32_t posLen = 0;
	in.read((char*)&posLen, sizeof(uint32_t));
	for (int i = 0; i < posLen; i++) {
		float_t frame;
		in.read((char*)&frame, sizeof(float_t));
		float_t pos[3];
		in.read((char*)&pos, sizeof(float_t) * 3);
		uint8_t mode;
		in.read((char*)&mode, sizeof(uint8_t));
		float_t inTangent[3];
		in.read((char*)&inTangent, sizeof(float_t) * 3);
		float_t inRate[3];
		in.read((char*)&inRate, sizeof(float_t) * 3);
		float_t outTangent[3];
		in.read((char*)&outTangent, sizeof(float_t) * 3);
		float_t outRate[3];
		in.read((char*)&outRate, sizeof(float_t) * 3);
		CurveValue<Vector3f> value = CurveValue<Vector3f>((CurveValue<Vector3f>::KeySmoothMode)mode, Vector3f(pos[0], pos[1], pos[2]));
		value.inTangent.x() = inTangent[0];
		value.inTangent.y() = inTangent[1];
		value.inTangent.z() = inTangent[2];
		value.inRate.x() = inRate[0];
		value.inRate.y() = inRate[1];
		value.inRate.z() = inRate[2];
		value.outTangent.x() = outTangent[0];
		value.outTangent.y() = outTangent[1];
		value.outTangent.z() = outTangent[2];
		value.outRate.x() = outRate[0];
		value.outRate.y() = outRate[1];
		value.outRate.z() = outRate[2];
		positionCurve.insert(frame, value);
		if (frame > maxFrame)
			maxFrame = frame;
	}
	uint32_t rotLen = 0;
	in.read((char*)&rotLen, sizeof(uint32_t));
	for (int i = 0; i < rotLen; i++) {
		float_t frame;
		in.read((char*)&frame, sizeof(float_t));
		float_t rot[4];
		in.read((char*)&rot, sizeof(float_t) * 4);
		uint8_t mode;
		in.read((char*)&mode, sizeof(uint8_t));
		float_t inTangent;
		in.read((char*)&inTangent, sizeof(float_t));
		float_t inRate;
		in.read((char*)&inRate, sizeof(float_t));
		float_t outTangent;
		in.read((char*)&outTangent, sizeof(float_t));
		float_t outRate;
		in.read((char*)&outRate, sizeof(float_t));
		CurveValue<Quaternionf> value = CurveValue<Quaternionf>((CurveValue<Quaternionf>::KeySmoothMode)mode, Quaternionf(rot[0], rot[1], rot[2], rot[3]));
		value.inTangent = inTangent;
		value.inRate = inRate;
		value.outTangent = outTangent;
		value.outRate = outRate;
		rotationCurve.insert(frame, value);
		if (frame > maxFrame)
			maxFrame = frame;
	}
	uint32_t scaLen = 0;
	in.read((char*)&scaLen, sizeof(uint32_t));
	for (int i = 0; i < scaLen; i++) {
		float_t frame;
		in.read((char*)&frame, sizeof(float_t));
		float_t pos[3];
		in.read((char*)&pos, sizeof(float_t) * 3);
		uint8_t mode;
		in.read((char*)&mode, sizeof(uint8_t));
		float_t inTangent[3];
		in.read((char*)&inTangent, sizeof(float_t) * 3);
		float_t inRate[3];
		in.read((char*)&inRate, sizeof(float_t) * 3);
		float_t outTangent[3];
		in.read((char*)&outTangent, sizeof(float_t) * 3);
		float_t outRate[3];
		in.read((char*)&outRate, sizeof(float_t) * 3);
		CurveValue<Vector3f> value = CurveValue<Vector3f>((CurveValue<Vector3f>::KeySmoothMode)mode, Vector3f(pos[0], pos[1], pos[2]));
		value.inTangent.x() = inTangent[0];
		value.inTangent.y() = inTangent[1];
		value.inTangent.z() = inTangent[2];
		value.inRate.x() = inRate[0];
		value.inRate.y() = inRate[1];
		value.inRate.z() = inRate[2];
		value.outTangent.x() = outTangent[0];
		value.outTangent.y() = outTangent[1];
		value.outTangent.z() = outTangent[2];
		value.outRate.x() = outRate[0];
		value.outRate.y() = outRate[1];
		value.outRate.z() = outRate[2];
		scaleCurve.insert(frame, value);
		if (frame > maxFrame)
			maxFrame = frame;
	}
	positionCurve.duration = maxFrame;
	rotationCurve.duration = maxFrame;
	scaleCurve.duration = maxFrame;
}

void TransformAnimationData::write(ostream & out) const
{
	uint32_t posLen = positionCurve.keys.size();
	out.write((char*)&posLen, sizeof(uint32_t));
	for (auto b = positionCurve.keys.begin(), e = positionCurve.keys.end(); b != e; b++) {
		float_t frame = b->first;
		out.write((char*)&frame, sizeof(float_t));
		float_t pos[3] = { b->second.value.x(), b->second.value.y(), b->second.value.z() };
		out.write((char*)&pos, sizeof(float_t) * 3);
		uint8_t mode = b->second.mode;
		out.write((char*)&mode, sizeof(uint8_t));
		float_t inTangent[3] = { b->second.inTangent.x(), b->second.inTangent.y(), b->second.inTangent.z() };
		out.write((char*)&inTangent, sizeof(float_t) * 3);
		float_t inRate[3] = { b->second.inRate.x(), b->second.inRate.y(), b->second.inRate.z() };
		out.write((char*)&inRate, sizeof(float_t) * 3);
		float_t outTangent[3] = { b->second.outTangent.x(), b->second.outTangent.y(), b->second.outTangent.z() };
		out.write((char*)&outTangent, sizeof(float_t) * 3);
		float_t outRate[3] = { b->second.outRate.x(), b->second.outRate.y(), b->second.outRate.z() };
		out.write((char*)&outRate, sizeof(float_t) * 3);
	}
	uint32_t rotLen = rotationCurve.keys.size();
	out.write((char*)&rotLen, sizeof(uint32_t));
	for (auto b = rotationCurve.keys.begin(), e = rotationCurve.keys.end(); b != e; b++) {
		float_t frame = b->first;
		out.write((char*)&frame, sizeof(float_t));
		float_t rot[4] = { b->second.value.x(), b->second.value.y(), b->second.value.z(), b->second.value.w() };
		out.write((char*)&rot, sizeof(float_t) * 4);
		uint8_t mode = b->second.mode;
		out.write((char*)&mode, sizeof(uint8_t));
		float_t inTangent = b->second.inTangent;
		out.write((char*)&inTangent, sizeof(float_t));
		float_t inRate = b->second.inRate;
		out.write((char*)&inRate, sizeof(float_t));
		float_t outTangent = b->second.outTangent;
		out.write((char*)&outTangent, sizeof(float_t));
		float_t outRate = b->second.outRate;
		out.write((char*)&outRate, sizeof(float_t));
	}
	uint32_t scaleLen = scaleCurve.keys.size();
	out.write((char*)&scaleLen, sizeof(uint32_t));
	for (auto b = scaleCurve.keys.begin(), e = scaleCurve.keys.end(); b != e; b++) {
		float_t frame = b->first;
		out.write((char*)&frame, sizeof(float_t));
		float_t sca[3] = { b->second.value.x(), b->second.value.y(), b->second.value.z() };
		out.write((char*)&sca, sizeof(float_t) * 3);
		uint8_t mode = b->second.mode;
		out.write((char*)&mode, sizeof(uint8_t));
		float_t inTangent[3] = { b->second.inTangent.x(), b->second.inTangent.y(), b->second.inTangent.z() };
		out.write((char*)&inTangent, sizeof(float_t) * 3);
		float_t inRate[3] = { b->second.inRate.x(), b->second.inRate.y(), b->second.inRate.z() };
		out.write((char*)&inRate, sizeof(float_t) * 3);
		float_t outTangent[3] = { b->second.outTangent.x(), b->second.outTangent.y(), b->second.outTangent.z() };
		out.write((char*)&outTangent, sizeof(float_t) * 3);
		float_t outRate[3] = { b->second.outRate.x(), b->second.outRate.y(), b->second.outRate.z() };
		out.write((char*)&outRate, sizeof(float_t) * 3);
	}
}
