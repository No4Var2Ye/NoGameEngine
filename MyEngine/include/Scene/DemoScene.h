
// ======================================================================
#ifndef __DEMO_SCENE_H__
#define __DEMO_SCENE_H__
// ======================================================================
#include "Scene/Scene.h"
// ======================================================================

class CDemoScene : public CScene
{
public:
    CDemoScene() : CScene("DemoScene") {}
    virtual ~CDemoScene() = default;

    virtual BOOL Initialize() override;
    virtual void Shutdown() override;
    virtual void Update(float deltaTime) override;
    virtual void Render() override;

private:
    void DrawGrid();
};

#endif // __DEMO_SCENE_H__