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

Suppose you have a class `Vec`, what you need to do are

- write `TypeInfo<Vec>` (you can use `USRefl::AutoRefl` to generate)
- register : `Ubpa::ULuaPP::Register<Vec>(lua_State*)` 

That's all.

```c++
#include <ULuaPP/ULuaPP.h>
#include <iostream>

struct Vec {
  Vec(float x, float y) : x{ x }, y{ y } {}
  float x;
  float y;
  void Add(float dx = 1.f, float dy = 1.f) {
    x += dx;
    y += dy;
  }
};

template<>
struct Ubpa::USRefl::TypeInfo<Vec> :
  TypeInfoBase<Vec>
{
#ifdef UBPA_USREFL_NOT_USE_NAMEOF
  static constexpr char name[4] = "Vec";
#endif
  static constexpr AttrList attrs = {};
  static constexpr FieldList fields = {
    Field {TSTR(UMeta::constructor), WrapConstructor<Type(float, float)>()},
    Field {TSTR("x"), &Type::x},
    Field {TSTR("y"), &Type::y},
    Field {TSTR("Add"), &Type::Add, AttrList {
      Attr {TSTR(UMeta::default_functions), std::tuple {
        [](Type* __this, float dx) { return __this->Add(std::forward<float>(dx)); },
        [](Type* __this) { return __this->Add(); }
      }},
    }},
  };
};

int main() {
  lua_State* L = luaL_newstate(); /* opens Lua */
  luaL_openlibs(L); /* opens the standard libraries */

  // you just need to write a line of code
  Ubpa::ULuaPP::Register<Vec>(L);
	
  {
    sol::state_view lua(L);
    const char code[] = R"(
v = Vec.new(1, 2)
print(v.x, v.y)
v.x = 3
v.y = 4
print(v.x, v.y)
v:Add(1, 2)
v:Add(3)
v:Add()
print(v.x, v.y)
)";
    lua.script(code);
  }
  lua_close(L);
  return 0;
}
```

