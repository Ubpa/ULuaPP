#include <ULuaPP/ULuaPP.h>

#include <iostream>

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
	float Sum(float z) { return x + y + z; }
	[[number(1024)]]
	float Min() { return x < y ? x : y; }
	[[number(520)]]
	float Min(float z) { return x < y ? (x < z ? x : z) : (y < z ? y : z); }
	Point Add(const Point& rhs) const {
		return { x + rhs.x, y + rhs.y };
	}
	Point operator-(const Point& rhs) const {
		return { x - rhs.x, y - rhs.y };
	}
};

template<>
struct Ubpa::USRefl::TypeInfo<Point>
	: Ubpa::USRefl::TypeInfoBase<Point>
{
	static constexpr AttrList attrs = {
		Attr{"size", 8},
	};

	static constexpr FieldList fields = {
		Field{"x", &Point::x,
			AttrList{
				Attr{"not_serialize"},
			}
		},
		Field{"y", &Point::y,
			AttrList{
				Attr{"info", "hello"},
			}
		},
		Field{Name::constructor, WrapConstructor<Point()>()},
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
		Field{"Sum", static_cast<float(Point::*)()>(&Point::Sum)},
		Field{"Sum", static_cast<float(Point::*)(float)>(&Point::Sum),
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "z"},
					}
				},
			}
		},
		Field{"Min", static_cast<float(Point::*)()>(&Point::Min),
			AttrList {
				Attr{"number", 1024},
			}
		},
		Field{"Min", static_cast<float(Point::*)(float)>(&Point::Min),
			AttrList {
				Attr{"number", 520},
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "z"},
					}
				},
			}
		},
		Field{"Add", &Point::Add,
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "rhs"},
					}
				},
			}
		},
		Field{"operator-", &Point::operator-,
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
	{
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
print(p0:Sum(2))                                           -- non-static member function overload
print(p0:Min())                                            -- non-static member function overload
print(p0:Min(-1))                                          -- non-static member function overload
print(USRefl_TypeInfo.Point.fields.Min_0.attrs.number)     -- non-static member function overload
print(USRefl_TypeInfo.Point.fields.Min_1.attrs.number)     -- non-static member function overload
p2 = p0:Add(p1)                                            -- usertype argument
print(p2.x, p2.y)
p3 = p0 - p1                                               -- meta function
print(p3.x, p3.y)
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
