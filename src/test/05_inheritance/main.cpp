#include <ULuaPP/ULuaPP.h>

using namespace Ubpa::USRefl;
using namespace std;

struct A { float a; };
struct B : A { float b; };

template<>
struct Ubpa::USRefl::TypeInfo<A>
	: Ubpa::USRefl::TypeInfoBase<A>
{
	static constexpr AttrList attrs = {};

	static constexpr FieldList fields = {
		Field{"a", &A::a},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<B>
	: Ubpa::USRefl::TypeInfoBase<B, Base<A, true>>
{
	static constexpr AttrList attrs = {};

	static constexpr FieldList fields = {
		Field{"b", &B::b},
	};
};

int main() {
	char buff[256];
	int error;
	lua_State* L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */
	Ubpa::ULuaPP::Register<B>(L);
	{
		sol::state_view lua(L);
		const char code[] = R"(
b = B.new()
b.a = 1
b.b = 2
print(b.a, b.b)
)";
		cout << code << endl
			<< "----------------------------" << endl;
		lua.script(code);
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
