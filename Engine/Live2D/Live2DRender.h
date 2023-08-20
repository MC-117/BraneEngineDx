#pragma once

#include "Live2DModel.h"
#include "Live2DMeshData.h"
#include "Live2DCamera.h"
#include "../Core/Render.h"

struct Live2DClipContext
{
    bool _isUsing = false;                                ///< 現在の描画状態でマスクの準備が必要ならtrue
    const int* _clippingIdList;                 ///< クリッピングマスクのIDリスト
    int _clippingIdCount = 0;                       ///< クリッピングマスクの数
    int _layoutChannelNo = 0;                       ///< RGBAのいずれのチャンネルにこのクリップを配置するか(0:R , 1:G , 2:B , 3:A)
    Vector4f _layoutBounds;                         ///< マスク用チャンネルのどの領域にマスクを入れるか(View座標-1..1, UVは0..1に直す)
    Vector4f _allClippedDrawRect;                   ///< このクリッピングで、クリッピングされる全ての描画オブジェクトの囲み矩形（毎回更新）
    Matrix4f _matrixForMask;                   ///< マスクの位置計算結果を保持する行列
    Matrix4f _matrixForDraw;                   ///< 描画オブジェクトの位置計算結果を保持する行列
    vector<int> _clippedDrawableIndexList;  ///< このマスクにクリップされる描画オブジェクトのリスト

    Live2DClipContext(const int* clippingDrawableIndices, int clipCount);
    void AddClippedDrawable(int drawableIndex);

    void CalcClippedDrawTotalBounds(Csm::CubismModel& model);
};

class Live2DRender : public Render
{
public:
    Live2DRender();
    virtual ~Live2DRender();

    void setModel(Live2DModel* modelAsset, Csm::CubismModel* model);

    void setSize(const Vector2f& size);
    void setRefSize(float refLength, bool byWidth = true);

    bool is3DMode();
    void set3DMode(bool value);

    bool isPremultipliedAlpha() const;
    void setPremultipliedAlpha(bool value);

    bool loadDefaultMaterial();

    Texture2D* getOutputTexture();

    virtual void preRender();
    virtual void render(RenderInfo& info);
    virtual Matrix4f getTransformMatrix() const;
    virtual IRendering::RenderType getRenderType() const;
    virtual Shape* getShape() const;
    virtual Material* getMaterial(unsigned int index = 0);
    virtual bool getMaterialEnable(unsigned int index = 0);
    virtual Shader* getShader() const;
protected:
    enum Live2DShaderTag
    {
        SetupMask = ShaderFeature::Shader_Custom_1,
        Normal = ShaderFeature::Shader_Custom_2,
        Masked = ShaderFeature::Shader_Custom_3,
        Inverted = ShaderFeature::Shader_Custom_4,
        PremultipliedAlpha = ShaderFeature::Shader_Custom_5,
    };

    enum CubismBlendMode
    {
        CubismBlendMode_Normal = 0,          ///< 通常

        CubismBlendMode_Additive = 1,        ///< 加算

        CubismBlendMode_Multiplicative = 2,  ///< 乗算
    };  // CubismBlendMode

    bool _is3DMode = false;

    Vector2f viewSize = { 512, 512 };

    Material* material = NULL;

    Texture2D drawTexture = Texture2D(512, 512, 4, false, { TW_Repeat, TW_Repeat, TF_Linear, TF_Linear });
    Texture2D maskTexture = Texture2D(512, 512, 4, false, { TW_Repeat, TW_Repeat, TF_Linear, TF_Linear, TIT_Default, { 1.0f, 1.0f, 1.0f, 1.0f } });
    RenderTarget drawRenderTarget = RenderTarget(512, 512, 4);
    RenderTarget maskRenderTarget = RenderTarget(512, 512, 4);
    Live2DMeshData meshData;

    Live2DModel* modelAsset = NULL;
    Csm::CubismModel* model = NULL;
    vector<int> sortedDrawableIndexList;

    bool isCulling = false;

    bool _useHighPrecisionMask = false;
    bool _isPremultipliedAlpha = false;

    Color _modelColor = { 1.0f, 1.0f, 1.0f, 1.0f };
 
    const int ColorChannelCount = 4;

    vector<Live2DClipContext*> _clippingContextListForMask;   ///< マスク用クリッピングコンテキストのリスト
    vector<Live2DClipContext*> _clippingContextListForDraw;   ///< 描画用クリッピングコンテキストのリスト
    Vector2f _clippingMaskBufferSize = { 512, 512 };

    Live2DClipContext* _clippingContextBufferForMask = NULL;
    Live2DClipContext* _clippingContextBufferForDraw = NULL;

    Matrix4f  _tmpMatrix;              ///< マスク計算用の行列
    Matrix4f  _tmpMatrixForMask;       ///< マスク計算用の行列
    Matrix4f  _tmpMatrixForDraw;       ///< マスク計算用の行列
    Vector4f  _tmpBoundsOnModel;       ///< マスク配置計算用の矩形

    void release();

    Matrix4f getMvpMatrix(const ::Camera& camera);

    Live2DClipContext* findSameClip(const int* drawableMasks, int drawableMaskCounts) const;

    void setupLayoutBounds(int usingClipCount);
    void expand(Vector4f& rect, float w, float h);
    void setupClippingContext(RenderInfo& info);

    void drawMesh(RenderInfo& info, int drawableIndex, CubismBlendMode colorBlendMode, bool invertedMask);
};