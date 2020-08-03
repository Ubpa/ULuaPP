#include <ULuaPP/ULuaPP.h>

using namespace Ubpa::USRefl;
using namespace std;

enum class [[meta(520)]] Color {
	RED [[meta("a")]],
	GREEN [[meta("b")]],
	BLUE [[meta("c")]]
};

template<>
struct Ubpa::USRefl::TypeInfo<Color>
	: Ubpa::USRefl::TypeInfoBase<Color>
{
	static constexpr AttrList attrs = {
		Attr{"meta", 520},
	};

	static constexpr FieldList fields = {
		Field{"RED", Color::RED,
			AttrList{
				Attr{"meta", "a"},
			}
		},
		Field{"GREEN", Color::GREEN,
			AttrList{
				Attr{"meta", "b"},
			}
		},
		Field{"BLUE", Color::BLUE,
			AttrList{
				Attr{"meta", "c"},
			}
		},
	};
};

int main() {
	char buff[256];
	int error;
	lua_State* L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */

	Ubpa::ULuaPP::Register<Color>(L);

	{
		sol::state_view lua(L);
		const char code[] = R"(
print(Color.RED)
print(Color.GREEN)
print(Color.BLUE)
print(USRefl_TypeInfo.Color.attrs.meta)
print(USRefl_TypeInfo.Color.fields.RED.attrs.meta)
print(USRefl_TypeInfo.Color.fields.GREEN.attrs.meta)
print(USRefl_TypeInfo.Color.fields.BLUE.attrs.meta)
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
