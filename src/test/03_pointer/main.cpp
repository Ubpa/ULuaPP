#include <ULuaPP/ULuaPP.h>

using namespace Ubpa::USRefl;
using namespace std;

struct Point {
	float x;
	float y;

	Point(float x, float y) : x{ x }, y{ y }{}
	Point AddPointer(const Point* rhs) const {
		return { x + rhs->x, y + rhs->y };
	}
};

template<>
struct Ubpa::USRefl::TypeInfo<Point>
	: Ubpa::USRefl::TypeInfoBase<Point>
{
	static constexpr AttrList attrs = {};

	static constexpr FieldList fields = {
		Field{"x", &Point::x},
		Field{"y", &Point::y},
		Field{Name::constructor, WrapConstructor<Point(float, float)>(),
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "x"},
					}
				},
				Attr{UBPA_USREFL_NAME_ARG(1),
					AttrList{
						Attr{Name::name, "y"},
					}
				},
			}
		},
		Field{"AddPointer", &Point::AddPointer,
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "rhs"},
					}
				},
			}
		},
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
p0 = Point.new(3, 4)
p1 = Point.new(1, 2)
p2 = p0:AddPointer(p1) -- amazing
print(p2.x, p2.y)
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
