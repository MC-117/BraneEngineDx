#include "Live2DActor.h"

const char* Live2DParameter::defaultParameterNames[Live2DParameter::ParamCount] =
{
    "HitArea",
    "Head",
    "Body",
    "Parts01Core",
    "Parts01Arm_",
    "Parts01ArmL_",
    "Parts01ArmR_",
    "ParamAngleX",
    "ParamAngleY",
    "ParamAngleZ",
    "ParamEyeLOpen",
    "ParamEyeLSmile",
    "ParamEyeROpen",
    "ParamEyeRSmile",
    "ParamEyeBallX",
    "ParamEyeBallY",
    "ParamEyeBallForm",
    "ParamBrowLY",
    "ParamBrowRY",
    "ParamBrowLX",
    "ParamBrowRX",
    "ParamBrowLAngle",
    "ParamBrowRAngle",
    "ParamBrowLForm",
    "ParamBrowRForm",
    "ParamMouthForm",
    "ParamMouthOpenY",
    "ParamCheek",
    "ParamBodyAngleX",
    "ParamBodyAngleY",
    "ParamBodyAngleZ",
    "ParamBreath",
    "ParamArmLA",
    "ParamArmRA",
    "ParamArmLB",
    "ParamArmRB",
    "ParamHandL",
    "ParamHandR",
    "ParamHairFront",
    "ParamHairSide",
    "ParamHairBack",
    "ParamHairFluffy",
    "ParamShoulderY",
    "ParamBustX",
    "ParamBustY",
    "ParamBaseX",
    "ParamBaseY",
    "NONE:"
};

SerializeInstance(Live2DActor)

Live2DActor::Live2DActor(const string& name) : Actor(name)
{
}

Live2DActor::~Live2DActor()
{
    release();
}

void Live2DActor::setModel(Live2DModel* model)
{
    release();
    if (model != NULL) {
        modelAsset = model;
        loadModel();
        live2DRender.setModel(modelAsset, _model);
    }
}

Live2DModel* Live2DActor::getModel()
{
    return modelAsset;
}

bool Live2DActor::hasParameter(Live2DParameter::Type type)
{
    return parameters[type] != 0;
}

float Live2DActor::getParameterDefault(Live2DParameter::Type type)
{
    return _model->GetParameterDefaultValue(parameterIndex[type]);
}

Range<float> Live2DActor::getParameterRange(Live2DParameter::Type type)
{
    int min = _model->GetParameterMinimumValue(parameterIndex[type]);
    int max = _model->GetParameterMaximumValue(parameterIndex[type]);
    return Range<float>(min, max);
}

float Live2DActor::getParameter(Live2DParameter::Type type)
{
    return _model->GetParameterValue(parameterIndex[type]);
}

void Live2DActor::setParameter(Live2DParameter::Type type, float value)
{
    _model->SetParameterValue(parameterIndex[type], value);
    //_model->SaveParameters();
}

const map<string, Csm::ACubismMotion*>& Live2DActor::getMotions() const
{
    return _motions;
}

const map<string, Csm::ACubismMotion*>& Live2DActor::getExpressions() const
{
    return _expressions;
}

bool Live2DActor::playMotion(const string& name)
{
    return StartMotion(name, PriorityForce) != NULL;
}

bool Live2DActor::playExpression(const string& name)
{
    return false;
}

void Live2DActor::onLive2DEvent(const string& eventValue)
{
    CubismLogInfo("%s", eventValue.c_str());
}

void Live2DActor::tick(float deltaTime)
{
    if (modelAsset == NULL || !modelAsset->isValid() || _model == NULL)
        return;
    const float deltaTimeSeconds = deltaTime;
    _userTimeSeconds += deltaTimeSeconds;

    _dragManager->Update(deltaTimeSeconds);
    _dragX = _dragManager->GetX();
    _dragY = _dragManager->GetY();

    // モーションによるパラメータ更新の有無
    bool motionUpdated = false;

    //-----------------------------------------------------------------
    //_model->LoadParameters(); // 前回セーブされた状態をロード
    if (_motionManager->IsFinished())
    {
        // モーションの再生がない場合、待機モーションの中からランダムで再生する
        //StartRandomMotion(PriorityIdle);
    }
    else
    {
        motionUpdated = _motionManager->UpdateMotion(_model, deltaTimeSeconds); // モーションを更新
    }
    //_model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------

    // まばたき
    if (!motionUpdated)
    {
        if (_eyeBlink != NULL)
        {
            // メインモーションの更新がないとき
            _eyeBlink->UpdateParameters(_model, deltaTimeSeconds); // 目パチ
        }
    }

    if (_expressionManager != NULL)
    {
        _expressionManager->UpdateMotion(_model, deltaTimeSeconds); // 表情でパラメータ更新（相対変化）
    }

    ////ドラッグによる変化
    ////ドラッグによる顔の向きの調整
    //_model->AddParameterValue(_idParamAngleX, _dragX * 30); // -30から30の値を加える
    //_model->AddParameterValue(_idParamAngleY, _dragY * 30);
    //_model->AddParameterValue(_idParamAngleZ, _dragX * _dragY * -30);

    ////ドラッグによる体の向きの調整
    //_model->AddParameterValue(_idParamBodyAngleX, _dragX * 10); // -10から10の値を加える

    ////ドラッグによる目の向きの調整
    //_model->AddParameterValue(_idParamEyeBallX, _dragX); // -1から1の値を加える
    //_model->AddParameterValue(_idParamEyeBallY, _dragY);

    // 呼吸など
    if (_breath != NULL)
    {
        _breath->UpdateParameters(_model, deltaTimeSeconds);
    }

    // 物理演算の設定
    if (_physics != NULL)
    {
        _physics->Evaluate(_model, deltaTimeSeconds);
    }

    // リップシンクの設定
    //if (_lipSync)
    //{
    //    // リアルタイムでリップシンクを行う場合、システムから音量を取得して0〜1の範囲で値を入力します。
    //    float value = 0.0f;

    //    // 状態更新/RMS値取得
    //    _wavFileHandler.Update(deltaTimeSeconds);
    //    value = _wavFileHandler.GetRms();

    //    for (int i = 0; i < _lipSyncIds.size(); ++i)
    //    {
    //        _model->AddParameterValue(_lipSyncIds[i], value, 0.8f);
    //    }
    //}

    // ポーズの設定
    if (_pose != NULL)
    {
        _pose->UpdateParameters(_model, deltaTimeSeconds);
    }

    _model->Update();
}

void Live2DActor::prerender(SceneRenderData& sceneData)
{
    Actor::prerender(sceneData);
    if (modelAsset == NULL || !modelAsset->isValid())
        return;
    live2DRender.loadDefaultMaterial();
    live2DRender.transformMat = transformMat;
}

Render* Live2DActor::getRender()
{
    return &live2DRender;
}

unsigned int Live2DActor::getRenders(vector<Render*>& renders)
{
    renders.push_back(&live2DRender);
    return 1;
}

void Live2DActor::setHidden(bool value)
{
}

bool Live2DActor::isHidden()
{
    return false;
}

Serializable* Live2DActor::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool Live2DActor::deserialize(const SerializationInfo& from)
{
    return false;
}

bool Live2DActor::serialize(SerializationInfo& to)
{
    return false;
}

void Live2DActor::release()
{
    if (modelAsset == NULL || !modelAsset->isValid())
        return;
    if (_model != NULL)
        modelAsset->getMoc()->DeleteModel(_model);
    _model = NULL;

    CSM_DELETE(_motionManager);
    _motionManager = NULL;
    CSM_DELETE(_expressionManager);
    _expressionManager = NULL;
    CSM_DELETE(_modelMatrix);
    _modelMatrix = NULL;

    Csm::CubismPose::Delete(_pose);
    _pose = NULL;
    Csm::CubismEyeBlink::Delete(_eyeBlink);
    _eyeBlink = NULL;
    Csm::CubismBreath::Delete(_breath);
    _breath = NULL;
    CSM_DELETE(_dragManager);
    _dragManager = NULL;
    Csm::CubismPhysics::Delete(_physics);
    _physics = NULL;
    Csm::CubismModelUserData::Delete(_modelUserData);
    _modelUserData = NULL;

    for (auto b = _expressions.begin(), e = _expressions.end(); b != e; b++) {
        Csm::ACubismMotion::Delete(b->second);
    }
    _expressions.clear();

    for (auto b = _motions.begin(), e = _motions.end(); b != e; b++) {
        Csm::ACubismMotion::Delete(b->second);
    }
    _motions.clear();

    _eyeBlinkIds.Clear();
    _lipSyncIds.Clear();
}

void Live2DActor::loadModel()
{
    if (modelAsset == NULL || !modelAsset->isValid())
        return;
    _model = modelAsset->getMoc()->CreateModel();

    if (_model == NULL)
    {
        CubismLogError("Failed to CreateModel().");
        return;
    }

    _model->SaveParameters();

    for (int i = 0; i < Live2DParameter::ParamCount; i++) {
        parameters[i] = Csm::CubismFramework::GetIdManager()->GetId(Live2DParameter::defaultParameterNames[i]);
        parameterIndex[i] = _model->GetParameterIndex(parameters[i]);
    }

    Csm::ICubismModelSetting* setting = modelAsset->getSetting();

    // モーションマネージャーを作成
    // MotionQueueManagerクラスからの継承なので使い方は同じ
    _motionManager = CSM_NEW Csm::CubismMotionManager();
    _motionManager->SetEventCallback(CubismDefaultMotionEventCallback, this);

    // 表情モーションマネージャを作成
    _expressionManager = CSM_NEW Csm::CubismMotionManager();

    // ドラッグによるアニメーション
    _dragManager = CSM_NEW Csm::CubismTargetPoint();

    _model->SaveParameters();
    _modelMatrix = CSM_NEW Csm::CubismModelMatrix(_model->GetCanvasWidth(), _model->GetCanvasHeight());

    modelAsset->createExpression(_expressions);
    _physics = modelAsset->createPhysics();
    _pose = modelAsset->createPose();
    _modelUserData = modelAsset->createUserData();

    /*{
        _breath = Csm::CubismBreath::Create();
        Csm::csmVector<Csm::CubismBreath::BreathParameterData> breathParameters;

        breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(parameters[Live2DParameter::ParamAngleX], 0.0f, 15.0f, 6.5345f, 0.5f));
        breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(parameters[Live2DParameter::ParamAngleY], 0.0f, 8.0f, 3.5345f, 0.5f));
        breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(parameters[Live2DParameter::ParamAngleZ], 0.0f, 10.0f, 5.5345f, 0.5f));
        breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(parameters[Live2DParameter::ParamBodyAngleX], 0.0f, 4.0f, 15.5345f, 0.5f));
        breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(Csm::CubismFramework::GetIdManager()->GetId(
            Csm::DefaultParameterId::ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));

        _breath->SetParameters(breathParameters);
    }*/

    _eyeBlink = modelAsset->createEyeBlink();

    // EyeBlinkIds
    {
        int eyeBlinkIdCount = setting->GetEyeBlinkParameterCount();
        for (int i = 0; i < eyeBlinkIdCount; ++i)
        {
            _eyeBlinkIds.PushBack(setting->GetEyeBlinkParameterId(i));
        }
    }

    // LipSyncIds
    {
        int lipSyncIdCount = setting->GetLipSyncParameterCount();
        for (int i = 0; i < lipSyncIdCount; ++i)
        {
            _lipSyncIds.PushBack(setting->GetLipSyncParameterId(i));
        }
    }

    //Layout
    Csm::csmMap<Csm::csmString, Csm::csmFloat32> layout;
    setting->GetLayoutMap(layout);
    _modelMatrix->SetupFromLayout(layout);

    _model->SaveParameters();

    for (int i = 0; i < setting->GetMotionGroupCount(); i++)
    {
        const Csm::csmChar* group = setting->GetMotionGroupName(i);
        preloadMotionGroup(group);
    }

    _motionManager->StopAllMotions();
}

void Live2DActor::preloadMotionGroup(const Csm::csmChar* group)
{
    Csm::ICubismModelSetting* setting = modelAsset->getSetting();
    const int count = setting->GetMotionCount(group);

    for (int i = 0; i < count; i++) {
        string name = string(group) + '_' + to_string(i);
        const vector<Csm::csmByte>* buffer = modelAsset->getMotion(name);

        if (buffer == NULL)
            continue;

        Csm::CubismMotion* tmpMotion = Csm::CubismMotion::Create(buffer->data(), buffer->size());

        Csm::csmFloat32 fadeTime = setting->GetMotionFadeInTimeValue(group, i);
        if (fadeTime >= 0.0f)
        {
            tmpMotion->SetFadeInTime(fadeTime);
        }

        fadeTime = setting->GetMotionFadeOutTimeValue(group, i);
        if (fadeTime >= 0.0f)
        {
            tmpMotion->SetFadeOutTime(fadeTime);
        }
        tmpMotion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);

        auto iter = _motions.find(name);
        if (iter == _motions.end())
        {
            _motions.insert(make_pair(name, tmpMotion)).first->second;
        }
        else {
            Csm::ACubismMotion::Delete(iter->second);
            iter->second = tmpMotion;
        }
    }
}

Csm::CubismMotionQueueEntryHandle Live2DActor::StartMotion(const string& name, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (modelAsset == NULL || !modelAsset->isValid())
        return Csm::InvalidMotionQueueEntryHandleValue;
    auto setting = modelAsset->getSetting();
    if (priority == PriorityForce)
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        return Csm::InvalidMotionQueueEntryHandleValue;
    }

    auto iter = _motions.find(name);
    if (iter == _motions.end())
        return Csm::InvalidMotionQueueEntryHandleValue;

    //ex) idle_0
    Csm::CubismMotion* motion = static_cast<Csm::CubismMotion*>(iter->second);
    bool autoDelete = false;

    if (motion == NULL)
    {
        //string path = fileName;
        //path = _modelHomeDir + path;

        //csmByte* buffer;
        //csmSizeInt size;
        //buffer = CreateBuffer(path.GetRawString(), &size);
        //motion = static_cast<CubismMotion*>(LoadMotion(buffer, size, NULL, onFinishedMotionHandler));
        //csmFloat32 fadeTime = _modelSetting->GetMotionFadeInTimeValue(group, no);
        //if (fadeTime >= 0.0f)
        //{
        //    motion->SetFadeInTime(fadeTime);
        //}

        //fadeTime = _modelSetting->GetMotionFadeOutTimeValue(group, no);
        //if (fadeTime >= 0.0f)
        //{
        //    motion->SetFadeOutTime(fadeTime);
        //}
        //motion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);
        //autoDelete = true; // 終了時にメモリから削除

        //DeleteBuffer(buffer, path.GetRawString());
    }
    else
    {
        motion->SetFinishedMotionHandler(onFinishedMotionHandler);
    }

    //voice
    /*Csm::csmString voice = setting->GetMotionSoundFileName(group, no);
    if (strcmp(voice.GetRawString(), "") != 0)
    {
        setting path = voice;
        path = _modelHomeDir + path;
        _wavFileHandler.Start(path);
    }*/

    return  _motionManager->StartMotionPriority(motion, autoDelete, priority);
}

Csm::CubismMotionQueueEntryHandle Live2DActor::StartRandomMotion(Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (modelAsset == NULL || !modelAsset->isValid())
        return Csm::InvalidMotionQueueEntryHandleValue;
    auto setting = modelAsset->getSetting();

    int no = rand() % _motions.size();

    string motionName;
    auto b = _motions.begin();
    for (int i = 0; i < no; i++)
        b++;

    return StartMotion(b->first, priority, onFinishedMotionHandler);
}

void Live2DActor::CubismDefaultMotionEventCallback(const Live2D::Cubism::Framework::CubismMotionQueueManager* caller, const Live2D::Cubism::Framework::csmString& eventValue, void* customData)
{
    Live2DActor* actor = reinterpret_cast<Live2DActor*>(customData);
    if (actor != NULL)
    {
        actor->onLive2DEvent(eventValue.GetRawString());
    }
}
