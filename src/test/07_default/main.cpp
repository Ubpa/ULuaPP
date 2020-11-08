#include <ULuaPP/ULuaPP.h>

#include <iostream>

using namespace Ubpa::USRefl;
using namespace std;

struct Point {
	float x; float y;
	Point(float x, float y) : x{ x }, y{ y } {}
	void Add(float dx = 1.f, float dy = 1.f) {
		x += dx;
		y += dy;
	}
};

template<>
struct Ubpa::USRefl::TypeInfo<Point> :
	TypeInfoBase<Point>
{
#ifdef UBPA_USREFL_NOT_USE_NAMEOF
	static constexpr char name[6] = "Point";
#endif
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
		Field {TSTR("x"), &Type::x},
		Field {TSTR("y"), &Type::y},
		Field {TSTR(UMeta::constructor), WrapConstructor<Type(float, float)>()},
		Field {TSTR("Add"), &Type::Add, AttrList {
			Attr {TSTR(UMeta::default_functions), std::tuple {
				[](Type* __this, float dx) { return __this->Add(std::forward<float>(dx)); },
				[](Type* __this) { return __this->Add(); }
			}},
		}},
	};
};

int main() {
	char buff[256];
	int error;
	lua_State* L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */
	Ubpa::ULuaPP::Register<Point>(L);
	auto ptr = malloc(sizeof(Point));
	{
		sol::state_view lua(L);
		lua["ptr"] = ptr;
		const char code[] = R"(
p = Point.new(3, 4)
print(p.x, p.y)
p:Add(1, 2)
print(p.x, p.y)
p:Add(3)
print(p.x, p.y)
p:Add()
print(p.x, p.y)
)";
		cout << code << endl
			<< "----------------------------" << endl;
		lua.safe_script(code);
	}

	while (fgets(buff, sizeof(buff), stdin) != NULL) {
		error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1); /* pop error message from the stack */
		}
	}
	lua_close(L);
	cout << "ptr : " << ((Point*)ptr)->x << ", " << ((Point*)ptr)->y << endl;
	free(ptr);
	return 0;
}
