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

class Live2DActor : public Actor
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

	virtual void prerender(RenderCommandList& cmdLst);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:

    Live2DModel* modelAsset = NULL;
    Csm::CubismModel* _model = NULL;                          ///< Modelインスタンス

    Csm::CubismMotionManager* _motionManager = NULL;          ///< モーション管理
    Csm::CubismMotionManager* _expressionManager = NULL;      ///< 表情管理
    Csm::CubismEyeBlink* _eyeBlink = NULL;                    ///< 自動まばたき
    Csm::CubismBreath* _breath = NULL;                        ///< 呼吸
    Csm::CubismModelMatrix* _modelMatrix = NULL;              ///< モデル行列
    Csm::CubismPose* _pose = NULL;                            ///< ポーズ管理
    Csm::CubismTargetPoint* _dragManager = NULL;              ///< マウスドラッグ
    Csm::CubismPhysics* _physics = NULL;                      ///< 物理演算
    Csm::CubismModelUserData* _modelUserData = NULL;          ///< ユーザデータ

    bool    _initialized = false;                  ///< 初期化されたかどうか
    bool    _updating = false;                     ///< 更新されたかどうか
    float   _opacity = 1.0f;                       ///< 不透明度
    bool    _lipSync = true;                       ///< リップシンクするかどうか
    float   _lastLipSyncValue = 0.0f;              ///< 最後のリップシンクの制御値
    float   _dragX = 0.0f;                         ///< マウスドラッグのX位置
    float   _dragY = 0.0f;                         ///< マウスドラッグのY位置
    float   _accelerationX = 0.0f;                 ///< X軸方向の加速度
    float   _accelerationY = 0.0f;                 ///< Y軸方向の加速度
    float   _accelerationZ = 0.0f;                 ///< Z軸方向の加速度

    Csm::csmFloat32 _userTimeSeconds = 0; ///< デルタ時間の積算値[秒]
    Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds; ///< モデルに設定されたまばたき機能用パラメータID
    Csm::csmVector<Csm::CubismIdHandle> _lipSyncIds; ///< モデルに設定されたリップシンク機能用パラメータID
    map<string, Csm::ACubismMotion*>   _motions; ///< 読み込まれているモーションのリスト
    map<string, Csm::ACubismMotion*>   _expressions; ///< 読み込まれている表情のリスト
    vector<Csm::csmRectF> _hitArea;
    vector<Csm::csmRectF> _userArea;

    const Csm::CubismId* parameters[Live2DParameter::ParamCount] = { 0 };
    int parameterIndex[Live2DParameter::ParamCount] = { -1 };

    // 外部定義ファイル(json)と合わせる
    const Csm::csmChar* MotionGroupIdle = "Idle"; // アイドリング
    const Csm::csmChar* MotionGroupTapBody = "TapBody"; // 体をタップしたとき

    // 外部定義ファイル(json)と合わせる
    const Csm::csmChar* HitAreaNameHead = "Head";
    const Csm::csmChar* HitAreaNameBody = "Body";

    // モーションの優先度定数
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