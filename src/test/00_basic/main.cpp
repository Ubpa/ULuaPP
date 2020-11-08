#include <ULuaPP/ULuaPP.h>

#include <iostream>

using namespace Ubpa::USRefl;
using namespace std;

struct [[size(8)]] Point {
	Point() : x{ 0.f }, y{ 0.f }{}
	Point(float x, float y = 1.f) : x{ x }, y{ y }{}
	[[not_serialize]]
	float x;
	[[info("hello")]]
	float y;
	float Sum() { return x + y; }
};

template<>
struct Ubpa::USRefl::TypeInfo<Point> :
	TypeInfoBase<Point>
{
#ifdef UBPA_USREFL_NOT_USE_NAMEOF
	static constexpr char name[6] = "Point";
#endif
	static constexpr AttrList attrs = {
		Attr {TSTR("size"), 8},
	};
	static constexpr FieldList fields = {
		Field {TSTR(UMeta::constructor), WrapConstructor<Type()>()},
		Field {TSTR(UMeta::constructor), WrapConstructor<Type(float, float)>(), AttrList {
			Attr {TSTR(UMeta::default_functions), std::tuple {
				WrapConstructor<Type(float)>()
			}},
		}},
		Field {TSTR("x"), &Type::x, AttrList {
			Attr {TSTR("not_serialize")},
		}},
		Field {TSTR("y"), &Type::y, AttrList {
			Attr {TSTR("info"), "hello"},
		}},
		Field {TSTR("Sum"), &Type::Sum},
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
p0 = Point.new(3, 4)                                        -- constructor
p1 = Point.new()                                            -- constructor overload
p2 = Point.new(2)                                           -- constructor default
print(p0.x, p0.y)                                           -- get field
print(p1.x, p1.y)                                           -- get field
print(p2.x, p2.y)                                           -- get field
p1.x = 3                                                    -- set field
print(p1.x, p1.y)
print(p0:Sum())                                             -- non-static member function
--print(USRefl_TypeInfo.Point.attrs.size)                   -- USRefl type attrs
--print(USRefl_TypeInfo.Point.fields.x.attrs.not_serialize) -- USRefl field attrs
--print(USRefl_TypeInfo.Point.fields.y.attrs.info)          -- USRefl type attrs
p3 = Point.voidp(ptr)
p3.x = 520
p3.y = 1024
print(p3.x, p3.y)
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
