
#include "dmfilesearch.h"
#include "gtest.h"

class env_dmfilesearch
{
public:
    void init(){}
    void uninit(){}
};

class frame_dmfilesearch : public testing::Test
{
public:
    virtual void SetUp()
    {
        env.init();
    }
    virtual void TearDown()
    {
        env.uninit();
    }
protected:
    env_dmfilesearch env;
};

TEST_F(frame_dmfilesearch, init)
{
    Idmfilesearch* module = dmfilesearchGetModule();
    if (module)
    {
        module->Test();
        module->Release();
    }
}
