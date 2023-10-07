#pragma once
#include "../Core/Actor.h"
#include "Live2DRender.h"

struct Live2DParameter
{
    enum Type {
        HitAreaPrefix,
        HitAreaHead,
        HitAreaBody,
        PartsIdCore,
        PartsArmPrefix,
        PartsArmLPrefix,
        PartsArmRPrefix,
        ParamAngleX,
        ParamAngleY,
        ParamAngleZ,
        ParamEyeLOpen,
        ParamEyeLSmile,
        ParamEyeROpen,
        ParamEyeRSmile,
        ParamEyeBallX,
        ParamEyeBallY,
        ParamEyeBallForm,
        ParamBrowLY,
        ParamBrowRY,
        ParamBrowLX,
        ParamBrowRX,
        ParamBrowLAngle,
        ParamBrowRAngle,
        ParamBrowLForm,
        ParamBrowRForm,
        ParamMouthForm,
        ParamMouthOpenY,
        ParamCheek,
        ParamBodyAngleX,
        ParamBodyAngleY,
        ParamBodyAngleZ,
        ParamBreath,
        ParamArmLA,
        ParamArmRA,
        ParamArmLB,
        ParamArmRB,
        ParamHandL,
        ParamHandR,
        ParamHairFront,
        ParamHairSide,
        ParamHairBack,
        ParamHairFluffy,
        ParamShoulderY,
        ParamBustX,
        ParamBustY,
        ParamBaseX,
        ParamBaseY,
        ParamNONE,
        ParamCount
    };

    static const char* defaultParameterNames[ParamCount];
};

class ENGINE_API Live2DActor : public Actor
{
public:
	Serialize(Live2DActor, Actor);

	Live2DRender live2DRender;

    Live2DActor(const string& name = "Live2DActor");
    virtual~ Live2DActor();

    void setModel(Live2DModel* model);
    Live2DModel* getModel();

    const map<string, Csm::ACubismMotion*>& getMotions() const;
    const map<string, Csm::ACubismMotion*>& getExpressions() const;

    bool hasParameter(Live2DParameter::Type type);
    float getParameterDefault(Live2DParameter::Type type);
    Range<float> getParameterRange(Live2DParameter::Type type);
    float getParameter(Live2DParameter::Type type);
    void setParameter(Live2DParameter::Type type, float value);

    bool playMotion(const string& name);
    bool playExpression(const string& name);

    virtual void onLive2DEvent(const string& eventValue);

    virtual void tick(float deltaTime);

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:

    Live2DModel* modelAsset = NULL;
    Csm::CubismModel* _model = NULL;

    Csm::CubismMotionManager* _motionManager = NULL;
    Csm::CubismMotionManager* _expressionManager = NULL;
    Csm::CubismEyeBlink* _eyeBlink = NULL;
    Csm::CubismBreath* _breath = NULL;
    Csm::CubismModelMatrix* _modelMatrix = NULL;
    Csm::CubismPose* _pose = NULL;
    Csm::CubismTargetPoint* _dragManager = NULL;
    Csm::CubismPhysics* _physics = NULL;
    Csm::CubismModelUserData* _modelUserData = NULL;

    bool    _initialized = false;
    bool    _updating = false;
    float   _opacity = 1.0f;
    bool    _lipSync = true;
    float   _lastLipSyncValue = 0.0f;
    float   _dragX = 0.0f;
    float   _dragY = 0.0f;
    float   _accelerationX = 0.0f;
    float   _accelerationY = 0.0f;
    float   _accelerationZ = 0.0f;

    Csm::csmFloat32 _userTimeSeconds = 0;
    Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds;
    Csm::csmVector<Csm::CubismIdHandle> _lipSyncIds;
    map<string, Csm::ACubismMotion*>   _motions;
    map<string, Csm::ACubismMotion*>   _expressions;
    vector<Csm::csmRectF> _hitArea;
    vector<Csm::csmRectF> _userArea;

    const Csm::CubismId* parameters[Live2DParameter::ParamCount] = { 0 };
    int parameterIndex[Live2DParameter::ParamCount] = { -1 };


    const Csm::csmChar* MotionGroupIdle = "Idle";
    const Csm::csmChar* MotionGroupTapBody = "TapBody";


    const Csm::csmChar* HitAreaNameHead = "Head";
    const Csm::csmChar* HitAreaNameBody = "Body";


    const Csm::csmInt32 PriorityNone = 0;
    const Csm::csmInt32 PriorityIdle = 1;
    const Csm::csmInt32 PriorityNormal = 2;
    const Csm::csmInt32 PriorityForce = 3;

    virtual void release();

    virtual void loadModel();
    virtual void preloadMotionGroup(const Csm::csmChar* group);

    Csm::CubismMotionQueueEntryHandle StartMotion(const string& name, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL);
    Csm::CubismMotionQueueEntryHandle StartRandomMotion(Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL);

    static void CubismDefaultMotionEventCallback(const Live2D::Cubism::Framework::CubismMotionQueueManager* caller, const Live2D::Cubism::Framework::csmString& eventValue, void* customData);
};