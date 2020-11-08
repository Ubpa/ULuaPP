#include <ULuaPP/ULuaPP.h>

#include <iostream>

using namespace Ubpa::USRefl;
using namespace std;

struct A {};

struct Point {
	float x;
	float y;

	Point(float x, float y) : x{ x }, y{ y }{}
	Point AddPointer(const Point* rhs) const {
		return { x + rhs->x, y + rhs->y };
	}
	Point* This() {
		return this;
	}
	void* ThisVoid() {
		return this;
	}
	Point AddPointer2(Point rhs) const {
		return { x + rhs.x, y + rhs.y };
	}
	static void Print(Point* p) {
		cout << p << endl;
	}
	static void Print2(void* p) {
		cout << p << endl;
		cout << reinterpret_cast<void*>(*reinterpret_cast<size_t*>(p)) << endl;
	}
	static void Print3(A* p) {
		cout << p << endl;
	}
};

template<>
struct Ubpa::USRefl::TypeInfo<A> :
	TypeInfoBase<A>
{
#ifdef UBPA_USREFL_NOT_USE_NAMEOF
	static constexpr char name[2] = "A";
#endif
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {};
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
		Field {TSTR("AddPointer"), &Type::AddPointer},
		Field {TSTR("This"), &Type::This},
		Field {TSTR("ThisVoid"), &Type::ThisVoid},
		Field {TSTR("AddPointer2"), &Type::AddPointer2},
		Field {TSTR("Print"), &Type::Print},
		Field {TSTR("Print2"), &Type::Print2},
		Field {TSTR("Print3"), &Type::Print3},
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
p0 = Point.new(3, 4)
p1 = Point.new(1, 2)
p2 = p0:AddPointer(p1) -- amazing
p3 = p0:AddPointer(p1:This()) -- amazing
print(p2.x, p2.y)
print(p3.x, p3.y)

p4 = p0:This()
p5 = p0:ThisVoid()
print(p4)
print(p5)
Point.Print(p4)
--Point.Print(p5)
Point.Print2(p4)
Point.Print2(p5)
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
