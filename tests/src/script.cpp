#include <sp2/script/environment.h>
#include "doctest.h"

static int luaYield(lua_State* lua)
{
    return lua_yield(lua, 0);
}

class TestObject : public sp::script::BindingObject
{
public:
    int prop = 0;
    sp::script::Callback callback;

    void test()
    {
    }

    sp::Vector2d testVector2d(sp::Vector2d v)
    {
        return v + sp::Vector2d(1, 2);
    }

    sp::P<TestObject> testObj(sp::P<TestObject> in)
    {
        CHECK(in == this);
        return this;
    }

    int testLua(lua_State* L)
    {
        lua_pushnumber(L, 1);
        return 1;
    }

    void onRegisterScriptBindings(sp::script::BindingClass& script_binding_class) override
    {
        script_binding_class.bind("test", &TestObject::test);
        script_binding_class.bind("testV2d", &TestObject::testVector2d);
        script_binding_class.bind("testObj", &TestObject::testObj);
        script_binding_class.bind("testLua", &TestObject::testLua);
        script_binding_class.bind("callback", callback);
        script_binding_class.bindProperty("prop", prop);
    }
};

TEST_CASE("script")
{
    sp::script::Environment env;

    CHECK(env.run("print(1)").isOk() == true);
    CHECK(env.run("nofunc(1)").isOk() == false);
    CHECK(env.run("function test() print(1) end").isOk() == true);
    CHECK(env.call("test").isOk() == true);
}

TEST_CASE("lua language extentions")
{
    sp::script::Environment env;

    //Allow != next to ~=
    CHECK(env.run("assert(1 ~= 0)").isOk() == true);
    CHECK(env.run("assert(1 != 0)").isOk() == true);

    CHECK(env.run("assert(1 && 1)").isOk() == true);
    CHECK(env.run("assert(1 || 1)").isOk() == true);
    CHECK(env.run("assert(!false)").isOk() == true);
}

TEST_CASE("coroutines")
{
    sp::script::Environment env;

    env.setGlobal("yield", luaYield);
    CHECK(env.call("yield").isOk() == false);
    CHECK(env.callCoroutine("yield").value() != nullptr);
    CHECK(env.callCoroutine("yield").value()->resume().isOk() == true);
	CHECK(env.callCoroutine("yield").value()->resume().value() == false);

    CHECK(env.run("yield()").isOk() == false);
    CHECK(env.runCoroutine("yield()").value() != nullptr);
    CHECK(env.runCoroutine("yield()").value()->resume().isOk() == true);
    CHECK(env.runCoroutine("yield()").value()->resume().value() == false);
    CHECK(env.runCoroutine("yield() yield()").value()->resume().value() == true);
}

TEST_CASE("sandbox")
{
    sp::script::Environment::SandboxConfig config{1024 * 30, 100000};
    sp::script::Environment env(config);
    LOG(Info, "Sandbox test start");
    CHECK(env.run("print(2)").isOk() == true);
    SUBCASE("endless") {
        CHECK(env.run("while true do end").isOk() == false);
    }
    SUBCASE("memory") {
        CHECK(env.run("a = {}; while true do a[#a + 1] = 1.1; end").isOk() == false);
    }
    SUBCASE("coroutine memory") {
        env.setGlobal("yield", luaYield);
        auto co = env.runCoroutine("a = {}; while true do a[#a + 1] = 1; yield(); end").value();
        CHECK(co != nullptr);
        while(true) {
            auto res = co->resume();
            if (res.isErr())
                break;
            if (!res.value())
                break;
        }
    }
}

TEST_CASE("sandbox memory")
{
    for(size_t mem=1024*50; mem>1024*2; mem-=1024)
    {
        CAPTURE(mem);
        sp::script::Environment::SandboxConfig config{mem, 100000};
        sp::script::Environment env(config);

        CHECK(env.run("a = {}; while true do a[#a + 1] = tostring(1.1); end").isOk() == false);
    }
}

TEST_CASE("sandbox memory coroutine")
{
    for(size_t mem=1024*50; mem>1024*2; mem-=1024)
    {
        CAPTURE(mem);
        sp::script::Environment::SandboxConfig config{mem, 100000};
        sp::script::Environment env(config);

        env.setGlobal("yield", luaYield);
        auto cores = env.runCoroutine("a = {}; while true do a[#a + 1] = 1; yield(); end");
        if (cores.isOk()) {
            auto co = cores.value();
            while(true) {
                auto res = co->resume();
                if (res.isErr())
                    break;
                if (!res.value())
                    break;
            }
        }
    }
}

TEST_CASE("object")
{
    sp::script::Environment::SandboxConfig config{1024*1024, 100000};
    sp::script::Environment env(config);
    TestObject test;
    env.setGlobal("test", &test);
    CHECK(env.run("test.test()").isOk() == true);
    test.prop = 2;
    CHECK(env.run("assert(test.prop == 2)").isOk() == true);
    CHECK(env.run("test.prop = 1").isOk() == true);
    CHECK(test.prop == 1);
    CHECK(test.callback.call().isOk() == true);
    CHECK(env.run("test.callback(function() print(1) end)").isOk() == true);
    CHECK(test.callback.call().isOk() == true);
    CHECK(test.callback.callCoroutine().value() == nullptr);
    CHECK(env.run("assert(test.testV2d({1, 1}).x == 2)").isOk() == true);
    CHECK(env.run("assert(test.testV2d({1, 1}).y == 3)").isOk() == true);
    CHECK(env.run("assert(test.testObj(test) == test)").isOk() == true);
    CHECK(env.run("assert(test.testLua()) == 1)").isOk() == true);

    env.setGlobal("yield", luaYield);
    CHECK(env.run("test.callback(function() print('pre'); yield(); print('post'); yield(); end)").isOk() == true);
    CHECK(test.callback.callCoroutine().value() != nullptr);
    CHECK(test.callback.callCoroutine().value()->resume().value() == true);
    CHECK(env.run("test.callback(function() while true do math.sin(0) end end)").isOk() == true);
    CHECK(test.callback.callCoroutine().value() == nullptr);
    CHECK(env.run("test.callback(function() yield() while true do math.sin(0) end end)").isOk() == true);
    CHECK(test.callback.callCoroutine().value()->resume().isOk() == false);
}

TEST_CASE("coroutine callback")
{
    sp::script::Environment env;

    TestObject test;
    env.setGlobal("test", &test);
    CHECK(env.runCoroutine("test.callback(function() assert(true) end)").value() == nullptr);
    test.callback.call();
}
