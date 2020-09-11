#include <ULuaPP/ULuaPP.h>

#include <iostream>

using namespace Ubpa::USRefl;
using namespace std;

template<typename T>
struct A { T a; };
struct B : A<bool> { float b; };

template<typename T>
struct Ubpa::USRefl::TypeInfo<A<T>>
	: Ubpa::USRefl::TypeInfoBase<A<T>>
{
	static constexpr AttrList attrs = {};

	static constexpr FieldList fields = {
		Field{"a", &A<T>::a},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<B>
	: Ubpa::USRefl::TypeInfoBase<B, Base<A<bool>, true>>
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
b.a = true
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
