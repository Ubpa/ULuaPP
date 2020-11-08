#include <ULuaPP/ULuaPP.h>

#include <iostream>

using namespace Ubpa::USRefl;
using namespace std;

struct Var {
	uint8_t v = 0;
	const uint16_t const_v = 1;
	inline static uint32_t static_v = 2;
	inline static const uint64_t static_const_v = 3;
	static constexpr int8_t constexpr_v = 4;

	void func() { std::cout << "func" << std::endl; }
	static void static_func() { std::cout << "static func" << std::endl; }
};

template<>
struct Ubpa::USRefl::TypeInfo<Var> :
    TypeInfoBase<Var>
{
#ifdef UBPA_USREFL_NOT_USE_NAMEOF
    static constexpr char name[4] = "Var";
#endif
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
        Field {TSTR("v"), &Type::v, AttrList {
            Attr {TSTR(UMeta::initializer), []()->int{ return 0; }},
        }},
        Field {TSTR("const_v"), &Type::const_v, AttrList {
            Attr {TSTR(UMeta::initializer), []()->const int{ return 1; }},
        }},
        Field {TSTR("static_v"), &Type::static_v, AttrList {
            Attr {TSTR(UMeta::initializer), []()->int{ return 2; }},
        }},
		Field {TSTR("static_const_v"), &Type::static_const_v, AttrList {
            Attr {TSTR(UMeta::initializer), []()->const int{ return 3; }},
        }},
		Field {TSTR("constexpr_v"), Type::constexpr_v},
		Field {TSTR("func"), static_cast<void(Type::*)()>(&Type::func)},
        Field {TSTR("static_func"), &Type::static_func},
    };
};

int main() {
	char buff[256];
	int error;
	lua_State* L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */
	Ubpa::ULuaPP::Register<Var>(L);
	{
		sol::state_view lua(L);
		const char code[] = R"(
v = Var.new()
print(v.v)
print(v.const_v)
print(v.static_v)
print(v.static_const_v)
print(v.constexpr_v)

v.v = 10
--v.const_v = 11
v.static_v = 12
--v.static_const_v = 13
--v.constexpr_v = 14
print(v.v)
print(v.const_v)
print(v.static_v)
print(v.static_const_v)
print(v.constexpr_v)
v:func()
v.static_func()
)";
		cout << code << endl
			<< "----------------------------" << endl;
		lua.unsafe_script(code);
	}

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
