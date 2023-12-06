#include "../Camera.h"
#include "Gizmo.h"

class ENGINE_API GUISurface
{
public:
    Gizmo gizmo;

    GUISurface(const string& name);
    virtual ~GUISurface();
	
    void bindCamera(Camera* newCamera);
    void setSize(const Vector2i& size);

    bool isValid() const;

    void gizmoUpdate();
    void gizmoFrame(ImDrawList* drawList, Object* rootObject);
    void gizmoRender2D(ImDrawList* drawList = NULL);
    void gizmoRender3D(RenderInfo& info);

    Camera* getCamera();
    virtual CameraRender* getCameraRender();
    virtual Texture* getSceneTexture();
    virtual RenderTarget* getRenderTarget();
	
    static GUISurface& getFullScreenGUISurface();
    static GUISurface& getMainViewportGUISurface();

    static GUISurface& getMainGUISurface();
protected:
    Ref<Camera> camera;
    Vector2i surfaceSize;
};
