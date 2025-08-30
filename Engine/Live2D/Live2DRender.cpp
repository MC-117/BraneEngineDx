#include "Live2DRender.h"
#include "../Core/Utility/MathUtility.h"

Live2DClipContext::Live2DClipContext(const int* clippingDrawableIndices, int clipCount)
	: _clippingIdList(clippingDrawableIndices), _clippingIdCount(clipCount)
{
}

void Live2DClipContext::AddClippedDrawable(int drawableIndex)
{
	_clippedDrawableIndexList.push_back(drawableIndex);
}

void Live2DClipContext::CalcClippedDrawTotalBounds(Csm::CubismModel& model)
{
    // 被クリッピングマスク（マスクされる描画オブジェクト）の全体の矩形
    float clippedDrawTotalMinX = FLT_MAX, clippedDrawTotalMinY = FLT_MAX;
    float clippedDrawTotalMaxX = -FLT_MAX, clippedDrawTotalMaxY = -FLT_MAX;

    // このマスクが実際に必要か判定する
    // このクリッピングを利用する「描画オブジェクト」がひとつでも使用可能であればマスクを生成する必要がある

    const int clippedDrawCount = _clippedDrawableIndexList.size();
    for (int clippedDrawableIndex = 0; clippedDrawableIndex < clippedDrawCount; clippedDrawableIndex++)
    {
        // マスクを使用する描画オブジェクトの描画される矩形を求める
        const int drawableIndex = _clippedDrawableIndexList[clippedDrawableIndex];

        const int drawableVertexCount = model.GetDrawableVertexCount(drawableIndex);
        const float* drawableVertexes = const_cast<float*>(model.GetDrawableVertices(drawableIndex));

        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX;

        int loop = drawableVertexCount * Csm::Constant::VertexStep;
        for (int pi = Csm::Constant::VertexOffset; pi < loop; pi += Csm::Constant::VertexStep)
        {
            float x = drawableVertexes[pi];
            float y = drawableVertexes[pi + 1];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        //
        if (minX == FLT_MAX) continue; //有効な点がひとつも取れなかったのでスキップする

        // 全体の矩形に反映
        if (minX < clippedDrawTotalMinX) clippedDrawTotalMinX = minX;
        if (minY < clippedDrawTotalMinY) clippedDrawTotalMinY = minY;
        if (maxX > clippedDrawTotalMaxX) clippedDrawTotalMaxX = maxX;
        if (maxY > clippedDrawTotalMaxY) clippedDrawTotalMaxY = maxY;
    }
    if (clippedDrawTotalMinX == FLT_MAX)
    {
        _allClippedDrawRect.x() = 0.0f;
        _allClippedDrawRect.y() = 0.0f;
        _allClippedDrawRect.z() = 0.0f;
        _allClippedDrawRect.w() = 0.0f;
        _isUsing = false;
    }
    else
    {
        _isUsing = true;
        float w = clippedDrawTotalMaxX - clippedDrawTotalMinX;
        float h = clippedDrawTotalMaxY - clippedDrawTotalMinY;
        _allClippedDrawRect.x() = clippedDrawTotalMinX;
        _allClippedDrawRect.y() = clippedDrawTotalMinY;
        _allClippedDrawRect.z() = w;
        _allClippedDrawRect.w() = h;
    }
}

Live2DRender::Live2DRender()
{
    drawTexture.setAutoGenMip(false);
    maskTexture.setAutoGenMip(false);
    drawRenderTarget.addTexture("drawTexture", drawTexture);
    maskRenderTarget.addTexture("maskTexture", maskTexture);
}

Live2DRender::~Live2DRender()
{
    release();
}

void Live2DRender::setModel(Live2DModel* modelAsset, Csm::CubismModel* model)
{
    if (!loadDefaultMaterial())
        return;

	if (modelAsset == NULL || !modelAsset->isValid() || model == NULL)
		return;

    release();

	this->modelAsset = modelAsset;
	this->model = model;
    meshData.setModel(model);

    setSize({ model->GetCanvasWidthPixel(), model->GetCanvasHeightPixel() });

    if (model->IsUsingMasking()) {
        int drawableCount = model->GetDrawableCount();
        const int** drawableMasks = model->GetDrawableMasks();
        const int* drawableMaskCounts = model->GetDrawableMaskCounts();

        //クリッピングマスクを使う描画オブジェクトを全て登録する
        //クリッピングマスクは、通常数個程度に限定して使うものとする
        for (int i = 0; i < drawableCount; i++)
        {
            if (drawableMaskCounts[i] <= 0)
            {
                //クリッピングマスクが使用されていないアートメッシュ（多くの場合使用しない）
                _clippingContextListForDraw.push_back(NULL);
                continue;
            }

            // 既にあるClipContextと同じかチェックする
            Live2DClipContext* cc = findSameClip(drawableMasks[i], drawableMaskCounts[i]);
            if (cc == NULL)
            {
                // 同一のマスクが存在していない場合は生成する
                cc = new Live2DClipContext(drawableMasks[i], drawableMaskCounts[i]);
                _clippingContextListForMask.push_back(cc);
            }

            cc->AddClippedDrawable(i);

            _clippingContextListForDraw.push_back(cc);
        }
    }

    sortedDrawableIndexList.resize(model->GetDrawableCount(), 0);
}

void Live2DRender::setSize(const Vector2f& size)
{
    viewSize = size;
}

void Live2DRender::setRefSize(float refLength, bool byWidth)
{
    if (model == NULL)
        return;
    float aspect = model->GetCanvasWidthPixel() / model->GetCanvasHeightPixel();
    float width;
    float height;
    if (byWidth) {
        width = refLength;
        height = refLength / aspect;
    }
    else {
        width = refLength * aspect;
        height = refLength;
    }
    setSize({ width, height });
}

bool Live2DRender::is3DMode()
{
    return _is3DMode;
}

void Live2DRender::set3DMode(bool value)
{
    _is3DMode = value;
}

void Live2DRender::preRender(PreRenderInfo& info)
{
}

void Live2DRender::render(RenderInfo& info)
{
	if (modelAsset == NULL || !modelAsset->isValid() || model == NULL || material == NULL)
		return;

    if (_clippingMaskBufferSize.x() == 0 || _clippingMaskBufferSize.y() == 0)
        return;

    IVendor& vender = VendorManager::getInstance().getVendor();

    if (!_is3DMode) {
        drawRenderTarget.resize(viewSize.x(), viewSize.y());
        drawRenderTarget.clearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    if (model->IsUsingMasking()) {
        maskRenderTarget.resize(_clippingMaskBufferSize.x(), _clippingMaskBufferSize.y());

        setupClippingContext(info);

        if (!_useHighPrecisionMask)
        {
            vender.setViewport(0, 0, viewSize.x(), viewSize.y());
            // ビューポートを元に戻す
            /*GetRenderStateManager()->SetViewport(s_context,
                0.0f,
                0.0f,
                static_cast<float>(s_viewportWidth),
                static_cast<float>(s_viewportHeight),
                0.0f, 1.0f);*/
        }
    }

	const int drawableCount = model->GetDrawableCount();
	const int* renderOrder = model->GetDrawableRenderOrders();

	for (int i = 0; i < drawableCount; ++i)
	{
		const int order = renderOrder[i];
		sortedDrawableIndexList[order] = i;
	}

	for (int i = 0; i < drawableCount; ++i)
	{
		const int drawableIndex = sortedDrawableIndexList[i];

		// Drawableが表示状態でなければ処理をパスする
		if (!model->GetDrawableDynamicFlagIsVisible(drawableIndex))
		{
			continue;
		}

        // クリッピングマスクをセットする
        Live2DClipContext* clipContext = model->IsUsingMasking() ?
            _clippingContextListForDraw[drawableIndex] : NULL;

        if (clipContext != NULL && _useHighPrecisionMask) // マスクを書く必要がある
        {
            if (clipContext->_isUsing) // 書くことになっていた
            {
                vender.setViewport(0, 0, _clippingMaskBufferSize.x(), _clippingMaskBufferSize.y());
                maskRenderTarget.clearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
                maskRenderTarget.bindFrame();

                const int clipDrawCount = clipContext->_clippingIdCount;
                for (int ctx = 0; ctx < clipDrawCount; ctx++)
                {
                    const int clipDrawIndex = clipContext->_clippingIdList[ctx];

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!model->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    isCulling = model->GetDrawableCulling(clipDrawIndex) != 0;

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
                    _clippingContextBufferForMask = clipContext;
                    drawMesh(info, clipDrawIndex, CubismBlendMode::CubismBlendMode_Normal, false);
                }

                //_clippingManager->_colorBuffer->EndDraw(s_context);
                _clippingContextBufferForMask = NULL;

                // ビューポートを元に戻す
                vender.setViewport(0, 0, viewSize.x(), viewSize.y());
                /*GetRenderStateManager()->SetViewport(s_context,
                    0.0f,
                    0.0f,
                    static_cast<float>(s_viewportWidth),
                    static_cast<float>(s_viewportHeight),
                    0.0f, 1.0f);*/
            }
        }
        // クリッピングマスクをセットする
        _clippingContextBufferForDraw = clipContext;

        isCulling = model->GetDrawableCulling(drawableIndex) != 0;

        if (_is3DMode) {
            info.camera->cameraRender.getRenderTarget().bindFrame();
        }
        else
            drawRenderTarget.bindFrame();

        drawMesh(info, drawableIndex, (CubismBlendMode)model->GetDrawableBlendMode(drawableIndex), model->GetDrawableInvertedMask(drawableIndex));
	}
}

Matrix4f Live2DRender::getTransformMatrix() const
{
	return Matrix4f();
}

IRendering::RenderType Live2DRender::getRenderType() const
{
	return IRendering::RenderType::Normal_Render;
}

Shape* Live2DRender::getShape() const
{
	return nullptr;
}

Material* Live2DRender::getMaterial(unsigned int index)
{
	return nullptr;
}

bool Live2DRender::getMaterialEnable(unsigned int index)
{
	return false;
}

Shader* Live2DRender::getShader() const
{
	return nullptr;
}

bool Live2DRender::loadDefaultMaterial()
{
    if (material != NULL)
        return true;
    material = getAssetByPath<Material>("Engine/Shaders/Live2D.mat");
    return material != NULL;
}

bool Live2DRender::isPremultipliedAlpha() const
{
    return _isPremultipliedAlpha;
}

void Live2DRender::setPremultipliedAlpha(bool value)
{
    _isPremultipliedAlpha = value;
}

Texture2D* Live2DRender::getOutputTexture()
{
    if (modelAsset == NULL || !modelAsset->isValid() || model == NULL || material == NULL)
        return NULL;

    if (_clippingMaskBufferSize.x() == 0 || _clippingMaskBufferSize.y() == 0)
        return NULL;

    return &drawTexture;
}

void Live2DRender::release()
{
    meshData.release();
    set<Live2DClipContext*> clipContexts;
    for (int i = 0; i < _clippingContextListForMask.size(); i++)
        if (_clippingContextListForMask[i] != NULL)
            clipContexts.insert(_clippingContextListForMask[i]);
    _clippingContextListForMask.clear();
    for (int i = 0; i < _clippingContextListForDraw.size(); i++)
        if (_clippingContextListForDraw[i] != NULL)
            clipContexts.insert(_clippingContextListForDraw[i]);
    _clippingContextListForDraw.clear();

    for (auto b = clipContexts.begin(), e = clipContexts.end(); b != e; b++)
        delete* b;

    sortedDrawableIndexList.clear();
    _clippingContextBufferForMask = NULL;
    _clippingContextBufferForDraw = NULL;
}

Live2DClipContext* Live2DRender::findSameClip(const int* drawableMasks, int drawableMaskCounts) const
{
    // 作成済みClippingContextと一致するか確認
    for (int i = 0; i < _clippingContextListForMask.size(); i++)
    {
        Live2DClipContext* cc = _clippingContextListForMask[i];
        const int count = cc->_clippingIdCount;
        if (count != drawableMaskCounts) continue; //個数が違う場合は別物
        int samecount = 0;

        // 同じIDを持つか確認。配列の数が同じなので、一致した個数が同じなら同じ物を持つとする。
        for (int j = 0; j < count; j++)
        {
            const int clipId = cc->_clippingIdList[j];
            for (int k = 0; k < count; k++)
            {
                if (drawableMasks[k] == clipId)
                {
                    samecount++;
                    break;
                }
            }
        }
        if (samecount == count)
        {
            return cc;
        }
    }
    return NULL; //見つからなかった
}

Matrix4f Live2DRender::getMvpMatrix(const ::Camera& camera)
{
    if (model == NULL)
        return Matrix4f::Identity();
    if (_is3DMode) {
        return camera.projectionViewMat * transformMat;
    }
    else {
        float aspect = viewSize.y() / viewSize.x();
        Matrix4f projMat = Matrix4f::Identity();
        projMat(0, 0) = aspect;
        Matrix4f viewMat = Math::viewport(-1, -1, 2, 2, 0, 0);
        return projMat * viewMat * transformMat;
    }
}

void Live2DRender::setupLayoutBounds(int usingClipCount)
{
    if (usingClipCount <= 0)
    {// この場合は一つのマスクターゲットを毎回クリアして使用する
        for (int index = 0; index < _clippingContextListForMask.size(); index++)
        {
            Live2DClipContext* cc = _clippingContextListForMask[index];
            cc->_layoutChannelNo = 0; // どうせ毎回消すので固定で良い
            cc->_layoutBounds.x() = 0.0f;
            cc->_layoutBounds.y() = 0.0f;
            cc->_layoutBounds.z() = 1.0f;
            cc->_layoutBounds.w() = 1.0f;
        }
        return;
    }

    // ひとつのRenderTextureを極力いっぱいに使ってマスクをレイアウトする
    // マスクグループの数が4以下ならRGBA各チャンネルに１つずつマスクを配置し、5以上6以下ならRGBAを2,2,1,1と配置する

    // RGBAを順番に使っていく。
    const int div = usingClipCount / ColorChannelCount; //１チャンネルに配置する基本のマスク個数
    const int mod = usingClipCount % ColorChannelCount; //余り、この番号のチャンネルまでに１つずつ配分する

    // RGBAそれぞれのチャンネルを用意していく(0:R , 1:G , 2:B, 3:A, )
    int curClipIndex = 0; //順番に設定していくk

    for (int channelNo = 0; channelNo < ColorChannelCount; channelNo++)
    {
        // このチャンネルにレイアウトする数
        const int layoutCount = div + (channelNo < mod ? 1 : 0);

        // 分割方法を決定する
        if (layoutCount == 0)
        {
            // 何もしない
        }
        else if (layoutCount == 1)
        {
            //全てをそのまま使う
            Live2DClipContext* cc = _clippingContextListForMask[curClipIndex++];
            cc->_layoutChannelNo = channelNo;
            cc->_layoutBounds.x() = 0.0f;
            cc->_layoutBounds.y() = 0.0f;
            cc->_layoutBounds.z() = 1.0f;
            cc->_layoutBounds.w() = 1.0f;
        }
        else if (layoutCount == 2)
        {
            for (int i = 0; i < layoutCount; i++)
            {
                const int xpos = i % 2;

                Live2DClipContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds.x() = xpos * 0.5f;
                cc->_layoutBounds.y() = 0.0f;
                cc->_layoutBounds.z() = 0.5f;
                cc->_layoutBounds.w() = 1.0f;
                //UVを2つに分解して使う
            }
        }
        else if (layoutCount <= 4)
        {
            //4分割して使う
            for (int i = 0; i < layoutCount; i++)
            {
                const int xpos = i % 2;
                const int ypos = i / 2;

                Live2DClipContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds.x() = xpos * 0.5f;
                cc->_layoutBounds.y() = ypos * 0.5f;
                cc->_layoutBounds.z() = 0.5f;
                cc->_layoutBounds.w() = 0.5f;
            }
        }
        else if (layoutCount <= 9)
        {
            //9分割して使う
            for (int i = 0; i < layoutCount; i++)
            {
                const int xpos = i % 3;
                const int ypos = i / 3;

                Live2DClipContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds.x() = xpos / 3.0f;
                cc->_layoutBounds.y() = ypos / 3.0f;
                cc->_layoutBounds.z() = 1.0f / 3.0f;
                cc->_layoutBounds.w() = 1.0f / 3.0f;
            }
        }
        else
        {
            CubismLogError("not supported mask count : %d", layoutCount);

            // 開発モードの場合は停止させる
            CSM_ASSERT(0);

            // 引き続き実行する場合、 SetupShaderProgramでオーバーアクセスが発生するので仕方なく適当に入れておく
            // もちろん描画結果はろくなことにならない
            for (int i = 0; i < layoutCount; i++)
            {
                Live2DClipContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = 0;
                cc->_layoutBounds.x() = 0.0f;
                cc->_layoutBounds.y() = 0.0f;
                cc->_layoutBounds.z() = 1.0f;
                cc->_layoutBounds.w() = 1.0f;
            }
        }
    }
}

void Live2DRender::expand(Vector4f& rect, float w, float h)
{
    rect.x() -= w;
    rect.y() -= h;
    rect.z() += w * 2.0f;
    rect.w() += h * 2.0f;
}

void Live2DRender::setupClippingContext(RenderInfo& info)
{
    int usingClipCount = 0;
    for (int clipIndex = 0; clipIndex < _clippingContextListForMask.size(); clipIndex++)
    {
        // １つのクリッピングマスクに関して
        Live2DClipContext* cc = _clippingContextListForMask[clipIndex];

        // このクリップを利用する描画オブジェクト群全体を囲む矩形を計算
        cc->CalcClippedDrawTotalBounds(*model);

        if (cc->_isUsing)
        {
            usingClipCount++; //使用中としてカウント
        }
    }

    IVendor& vender = VendorManager::getInstance().getVendor();

    // マスク作成処理
    if (usingClipCount > 0)
    {
        if (!_useHighPrecisionMask)
        {
            vender.setViewport(0, 0, _clippingMaskBufferSize.x(), _clippingMaskBufferSize.y());
            maskRenderTarget.clearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        }

        // 各マスクのレイアウトを決定していく
        setupLayoutBounds(_useHighPrecisionMask ? 0 : usingClipCount);

        // 実際にマスクを生成する
        // 全てのマスクをどの様にレイアウトして描くかを決定し、ClipContext , ClippedDrawContext に記憶する
        for (int clipIndex = 0; clipIndex < _clippingContextListForMask.size(); clipIndex++)
        {
            // --- 実際に１つのマスクを描く ---
            Live2DClipContext* clipContext = _clippingContextListForMask[clipIndex];
            Vector4f allClippedDrawRect = clipContext->_allClippedDrawRect; //このマスクを使う、全ての描画オブジェクトの論理座標上の囲み矩形
            Vector4f layoutBoundsOnTex01 = clipContext->_layoutBounds; //この中にマスクを収める
            const float MARGIN = 0.05f;
            float scaleX = 0.0f;
            float scaleY = 0.0f;


            if (_useHighPrecisionMask)
            {
                const float ppu = model->GetPixelsPerUnit();
                const float maskPixelWidth = _clippingMaskBufferSize.x();
                const float maskPixelHeight = _clippingMaskBufferSize.y();
                const float physicalMaskWidth = layoutBoundsOnTex01.z() * maskPixelWidth;
                const float physicalMaskHeight = layoutBoundsOnTex01.w() * maskPixelHeight;

                _tmpBoundsOnModel = allClippedDrawRect;

                if (_tmpBoundsOnModel.z() * ppu > physicalMaskWidth)
                {
                    expand(_tmpBoundsOnModel, allClippedDrawRect.z() * MARGIN, 0.0f);
                    scaleX = layoutBoundsOnTex01.z() / _tmpBoundsOnModel.z();
                }
                else
                {
                    scaleX = ppu / physicalMaskWidth;
                }

                if (_tmpBoundsOnModel.w() * ppu > physicalMaskHeight)
                {
                    expand(_tmpBoundsOnModel, 0.0f, allClippedDrawRect.w() * MARGIN);
                    scaleY = layoutBoundsOnTex01.w() / _tmpBoundsOnModel.w();
                }
                else
                {
                    scaleY = ppu / physicalMaskHeight;
                }
            }
            else
            {
                // モデル座標上の矩形を、適宜マージンを付けて使う
                _tmpBoundsOnModel = allClippedDrawRect;
                expand(_tmpBoundsOnModel, allClippedDrawRect.z() * MARGIN, allClippedDrawRect.w() * MARGIN);
                //########## 本来は割り当てられた領域の全体を使わず必要最低限のサイズがよい
                // シェーダ用の計算式を求める。回転を考慮しない場合は以下のとおり
                // movePeriod' = movePeriod * scaleX + offX [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
                scaleX = layoutBoundsOnTex01.z() / _tmpBoundsOnModel.z();
                scaleY = layoutBoundsOnTex01.w() / _tmpBoundsOnModel.w();
            }


            // マスク生成時に使う行列を求める
            {
                // シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
                _tmpMatrixForMask = Matrix4f::Identity();
                Matrix4f transition = Matrix4f::Identity();
                Matrix4f scale = Matrix4f::Identity();
                {
                    // Layout0..1 を -1..1に変換
                    transition(0, 3) = -1;
                    transition(1, 3) = -1;
                    _tmpMatrixForMask = transition;

                    scale(0, 0) = 2;
                    scale(1, 1) = 2;
                    //_tmpMatrixForMask = _tmpMatrixForMask * scale;
                    _tmpMatrixForMask = _tmpMatrixForMask * scale;
                }
                {
                    // view to Layout0..1
                    transition(0, 3) = layoutBoundsOnTex01.x();
                    transition(1, 3) = layoutBoundsOnTex01.y(); //new = [translate]
                    _tmpMatrixForMask = _tmpMatrixForMask * transition;
                    //new = [translate][scale]
                    scale(0, 0) = scaleX;
                    scale(1, 1) = scaleY;
                    _tmpMatrixForMask = _tmpMatrixForMask * scale;

                    transition(0, 3) = -_tmpBoundsOnModel.x();
                    transition(1, 3) = -_tmpBoundsOnModel.y();
                    _tmpMatrixForMask = _tmpMatrixForMask * transition; //new = [translate][scale][translate]
                }
            }

            //--------- draw時の mask 参照用行列を計算
            {
                // シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
                _tmpMatrixForDraw = Matrix4f::Identity();
                Matrix4f transition = Matrix4f::Identity();
                Matrix4f scale = Matrix4f::Identity();
                {
                    transition(0, 3) = layoutBoundsOnTex01.x();
                    transition(1, 3) = layoutBoundsOnTex01.y(); //new = [translate]
                    _tmpMatrixForDraw = _tmpMatrixForDraw * transition;
                    // 上下反転
                    //new = [translate][scale]
                    scale(0, 0) = scaleX;
                    scale(1, 1) = -scaleY;
                    _tmpMatrixForDraw = _tmpMatrixForDraw * scale;

                    transition(0, 3) = -_tmpBoundsOnModel.x();
                    transition(1, 3) = -_tmpBoundsOnModel.y();
                    _tmpMatrixForDraw = _tmpMatrixForDraw * transition; //new = [translate][scale][translate]
                }
            }

            clipContext->_matrixForMask = _tmpMatrixForMask;

            clipContext->_matrixForDraw = _tmpMatrixForDraw;

            if (!_useHighPrecisionMask)
            {
                const int clipDrawCount = clipContext->_clippingIdCount;
                for (int i = 0; i < clipDrawCount; i++)
                {
                    const int clipDrawIndex = clipContext->_clippingIdList[i];

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!model->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    isCulling = model->GetDrawableCulling(clipDrawIndex) != 0;

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
                    _clippingContextBufferForMask = clipContext;

                    maskRenderTarget.bindFrame();

                    drawMesh(info, clipDrawIndex, CubismBlendMode::CubismBlendMode_Normal, false);
                }
            }
            else
            {
                // NOP このモードの際はチャンネルを分けず、マトリクスの計算だけをしておいて描画自体は本体描画直前で行う
            }
        }

        if (!_useHighPrecisionMask)
        {
            //useTarget.EndDraw(renderContext);

            _clippingContextBufferForMask = NULL;
        }
    }
}

void Live2DRender::drawMesh(RenderInfo& info, int drawableIndex, CubismBlendMode colorBlendMode, bool invertedMask)
{
    int textureNo = model->GetDrawableTextureIndices(drawableIndex);
    float opacity = model->GetDrawableOpacity(drawableIndex);
    int indexCount = model->GetDrawableVertexIndexCount(drawableIndex);

    if (indexCount == 0)
    {// 描画物無し
        return;
    }

    if (opacity <= 0.0f && _clippingContextBufferForMask == NULL)
    {
        return;
    }

    // テクスチャセット
    Texture2D* textureView = NULL;
    if (textureNo >= 0)
    {
        textureView = modelAsset->getTexture(textureNo);
    }

    if (textureView == NULL) return;    // モデルが参照するテクスチャがバインドされていない場合は描画をスキップする

    IVendor& vender = VendorManager::getInstance().getVendor();

    // 裏面描画の有効・無効
    if (isCulling)
    {
        vender.setCullState(CullType::Cull_Back);
    }
    else
    {
        vender.setCullState(CullType::Cull_Off);
    }

    Color modelColorRGBA = _modelColor;

    if (_clippingContextBufferForMask == NULL) // マスク生成時以外
    {
        modelColorRGBA.a *= opacity;
        if (_isPremultipliedAlpha)
        {
            modelColorRGBA.r *= modelColorRGBA.a;
            modelColorRGBA.g *= modelColorRGBA.a;
            modelColorRGBA.b *= modelColorRGBA.a;
        }
    }

    meshData.updateVertex(drawableIndex);
    const MeshPart* meshPart = meshData.getMeshPart(drawableIndex);

    if (meshPart == NULL || material == NULL)
        return;

    const Color colorChannels[4] = {
        Color(1.0f, 0.0f, 0.0f, 0.0f),
        Color(0.0f, 1.0f, 0.0f, 0.0f),
        Color(0.0f, 0.0f, 1.0f, 0.0f),
        Color(0.0f, 0.0f, 0.0f, 1.0f)
    };

    if (_clippingContextBufferForMask != NULL) // マスク生成時
    {
        ShaderProgram* program = material->getShader()->getProgram(SetupMask);
        program->init();
        program->bind();

        // チャンネル
        const int channelNo = _clippingContextBufferForMask->_layoutChannelNo;
        // チャンネルをRGBAに変換
        const Color& colorChannel = colorChannels[channelNo];

        // マスク用ブレンドステート
        vender.setRenderPostMaskState();

        // 定数バッファ
        {
            Vector4f rect = _clippingContextBufferForMask->_layoutBounds;
            rect.z() += rect.x();
            rect.w() += rect.y();

            material->setMatrix("projectMatrix", MATRIX_UPLOAD_OP(_clippingContextBufferForMask->_matrixForMask));
            material->setColor("baseColor", rect * 2.0f - Vector4f::Ones());
            material->setColor("channelFlag", colorChannel);
        }

        // テクスチャ
        {
            material->setTexture("mainTexture", *textureView);
        }

        material->processInstanceData();

        // 描画
        {
            MeshPartDesc desc = { meshPart->meshData, meshPart->vertexFirst, meshPart->vertexCount,
                meshPart->elementFirst, meshPart->elementCount };
            vender.meshDrawCall(desc);
        }
    }
    else // マスク生成以外の場合
    {
        const bool masked = _clippingContextBufferForDraw != NULL;  // この描画オブジェクトはマスク対象か
        const bool premult = _isPremultipliedAlpha;
        const int offset = (masked ? (invertedMask ? 2 : 1) : 0) + (_isPremultipliedAlpha ? 3 : 0);

        // ブレンドステート
        switch (colorBlendMode)
        {
        case CubismBlendMode::CubismBlendMode_Normal:
        default:
            vender.setRenderPostPremultiplyAlphaState();
            break;

        case CubismBlendMode::CubismBlendMode_Additive:
            vender.setRenderPostAddState();
            break;

        case CubismBlendMode::CubismBlendMode_Multiplicative:
            vender.setRenderPostMultiplyState();
            break;
        }

        {
            Enum<ShaderFeature> shaderFeature = Normal;
            // シェーダセット
            if (masked) {
                shaderFeature |= Masked;
                if (invertedMask)
                    shaderFeature |= Inverted;
            }
            if (premult)
                shaderFeature |= PremultipliedAlpha;

            ShaderProgram* program = material->getShader()->getProgram(shaderFeature);
            program->init();
            program->bind();

            material->setTexture("mainTexture", *textureView);

            // テクスチャ+サンプラーセット
            if (masked)
            {
                material->setTexture("maskTexture", maskTexture);
            }

            // 定数バッファ
            {
                if (masked)
                {
                    // View座標をClippingContextの座標に変換するための行列を設定

                    material->setMatrix("clipMatrix", MATRIX_UPLOAD_OP(_clippingContextBufferForDraw->_matrixForDraw));

                    // 使用するカラーチャンネルを設定
                    const int channelNo = _clippingContextBufferForDraw->_layoutChannelNo;
                    const Color colorChannel = colorChannels[channelNo];
                    material->setColor("channelFlag", colorChannel);
                }

                // プロジェクションMtx
                material->setMatrix("projectMatrix", MATRIX_UPLOAD_OP(getMvpMatrix(*info.camera)));
                // 色
                material->setColor("baseColor", modelColorRGBA);

                material->processInstanceData();
            }

            // 描画
            {
                MeshPartDesc desc = { meshPart->meshData, meshPart->vertexFirst, meshPart->vertexCount,
                meshPart->elementFirst, meshPart->elementCount };
                vender.meshDrawCall(desc);
            }
        }
    }

    _clippingContextBufferForMask = NULL;
    _clippingContextBufferForDraw = NULL;
}