```
 _   _ _                ____________ 
| | | | |               | ___ \ ___ \
| | | | |    _   _  __ _| |_/ / |_/ /
| | | | |   | | | |/ _` |  __/|  __/ 
| |_| | |___| |_| | (_| | |   | |    
 \___/\_____/\__,_|\__,_\_|   \_|    
                                     
```

[![repo-size](https://img.shields.io/github/languages/code-size/Ubpa/ULuaPP?style=flat)](https://github.com/Ubpa/ULuaPP/archive/master.zip) [![tag](https://img.shields.io/github/v/tag/Ubpa/ULuaPP)](https://github.com/Ubpa/ULuaPP/tags) [![license](https://img.shields.io/github/license/Ubpa/ULuaPP)](LICENSE) 

⭐ Star us on GitHub — it helps!

# ULuaPP

Ubpa Lua++ (Lua &amp; C++)

Auto register C++ class to Lua with [sol2](https://github.com/ThePhD/sol2) and [USRefl](https://github.com/Ubpa/USRefl) 

## Example

```c++
#include <ULuaPP/ULuaPP.h>

using namespace Ubpa::USRefl;
using namespace std;

struct [[size(8)]] Point {
    Point() : x{ 0.f }, y{ 0.f }{}
    Point(float x, float y) : x{ x }, y{ y }{}
    [[not_serialize]]
    float x;
    [[info("hello")]]
    float y;
    float Sum() { return x + y; }
};

template<>
struct TypeInfo<Point> : TypeInfoBase<Point> {
    static constexpr FieldList fields = {
        Field{"constructor", static_cast<void(*)(Point*)>([](Point* p) { new(p)Point; })},
        Field{"constructor",
            static_cast<void(*)(Point*,float,float)>([](Point* p, float x, float y) { new(p)Point{x,y}; })},
        Field{"x", &Point::x, AttrList{ Attr{ "not_serialize" } }},
        Field{"y", &Point::y, AttrList{ Attr{ "info", "hello" } }},
        Field{"Sum", &Point::Sum}
    };

    static constexpr AttrList attrs = {
        Attr{ "size", 8 }
    };
};

int main() {
    char buff[256];
    int error;
    lua_State* L = luaL_newstate(); /* opens Lua */
    luaL_openlibs(L); /* opens the standard libraries */

    Ubpa::ULuaPP::Register<Point>(L);
    sol::state_view lua(L);
    const char code[] = R"(
p0 = Point.new(3, 4)                                       -- constructor
p1 = Point.new()                                           -- constructor overload
print(p0.x, p0.y)                                          -- get field
p1.x = 3                                                   -- set field
print(p1.x, p1.y)
print(p0:Sum())                                            -- non-static member function
print(USRefl_TypeInfo.Point.attrs.size)                    -- USRefl type attrs
print(USRefl_TypeInfo.Point.fields.x.attrs.not_serialize)  -- USRefl field attrs
print(USRefl_TypeInfo.Point.fields.y.attrs.info)           -- USRefl type attrs
)";
    cout << code << endl
        << "----------------------------" << endl;
    lua.script(code);

    while (fgets(buff, sizeof(buff), stdin) != NULL) {
        error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1); /* pop error message from the stack */
        }
    }
    lua_close(L);
    return 0;
}
```

