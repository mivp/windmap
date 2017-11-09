#include <omega.h>
#include <omegaGl.h>
#include <iostream>
#include <vector>

#include "WindMap.h"

using namespace std;
using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class WindMapRenderModule : public EngineModule
{
public:
    WindMapRenderModule() :
        EngineModule("WindMapRenderModule"), visible(true), windmap(0), initialized(false)
    {
        
    }

    virtual void initializeRenderer(Renderer* r);

    virtual void update(const UpdateContext& context)
    {
        // After a frame all render passes had a chance to update their
        // textures. reset the raster update flag.       
    }
    
    virtual void dispose()
    {
        if(windmap)
            delete windmap;
    }

    void init(string inifile)
    {
        windmap = new windmap::WindMap(inifile.c_str());
    }

    void setVisible(bool v)
    {
        visible = v;
    }

    void toggleVisible() 
    {
        visible = !visible;
    }

    windmap::WindMap* windmap;
    bool visible;
    bool initialized;
};

///////////////////////////////////////////////////////////////////////////////
class WindMapRenderPass : public RenderPass
{
public:
    WindMapRenderPass(Renderer* client, WindMapRenderModule* prm) : 
        RenderPass(client, "WindMapRenderPass"), 
        module(prm) {}
    
    virtual void initialize()
    {
        RenderPass::initialize();
    }

    virtual void render(Renderer* client, const DrawContext& context)
    {
    	if(context.task == DrawContext::SceneDrawTask)
        {
            glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
            client->getRenderer()->beginDraw3D(context);
	
    	    if(module->visible && module->windmap)
    	    {
                if(!module->initialized) {
                    module->windmap->setScreenSize(context.viewport.width(), context.viewport.height());
                    module->initialized = true;
                }

                float* MV = context.modelview.cast<float>().data();
                float* P = context.projection.cast<float>().data();

                module->windmap->render(MV, P);

                if(oglError) return;
    	    }
            
            client->getRenderer()->endDraw();
            glPopAttrib();
        }
        
    }

private:
    WindMapRenderModule* module;

};

///////////////////////////////////////////////////////////////////////////////
void WindMapRenderModule::initializeRenderer(Renderer* r)
{
    r->addRenderPass(new WindMapRenderPass(r, this));
}

///////////////////////////////////////////////////////////////////////////////
WindMapRenderModule* initialize()
{
    WindMapRenderModule* prm = new WindMapRenderModule();
    ModuleServices::addModule(prm);
    prm->doInitialize(Engine::instance());
    return prm;
}

///////////////////////////////////////////////////////////////////////////////
// Python API
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(windmap)
{
    //
    PYAPI_REF_BASE_CLASS(WindMapRenderModule)
    PYAPI_METHOD(WindMapRenderModule, init)
    PYAPI_METHOD(WindMapRenderModule, setVisible)
    PYAPI_METHOD(WindMapRenderModule, toggleVisible)
    ;

    def("initialize", initialize, PYAPI_RETURN_REF);
}
