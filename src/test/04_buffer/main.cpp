#include <ULuaPP/ULuaPP.h>

#include <iostream>

using namespace Ubpa::USRefl;
using namespace std;

class Buffer {
public:
	Buffer(size_t s) {
		buffer = malloc(s);
		cout << "malloc buffer @" << buffer << endl;
	}

	~Buffer() {
		cout << "free buffer @" << buffer << endl;
		free(buffer);
	}

	double& GetNumber(size_t offset) {
		return *(double*)((uint8_t*)buffer + offset);
	}

	void SetNumber(size_t offset, double value) {
		GetNumber(offset) = value;
	}

	void InitTable(size_t offset) {
		new((uint8_t*)buffer + offset)sol::table;
	}

	void InitTable(size_t offset, sol::table value) {
		new((uint8_t*)buffer + offset)sol::table(move(value));
	}

	sol::table& GetTable(size_t offset) {
		return *(sol::table*)((uint8_t*)buffer + offset);
	}

	void SetTable(size_t offset, sol::table value) {
		GetTable(offset) = value;
	}
private:
	void* buffer;
};

template<>
struct Ubpa::USRefl::TypeInfo<Buffer>
	: Ubpa::USRefl::TypeInfoBase<Buffer>
{
	static constexpr AttrList attrs = {};

	static constexpr FieldList fields = {
		Field{Name::constructor, WrapConstructor<Buffer(size_t)>(),
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "s"},
					}
				},
			}
		},
		Field{Name::destructor, WrapDestructor<Buffer>()},
		Field{"GetNumber", &Buffer::GetNumber,
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "offset"},
					}
				},
			}
		},
		Field{"SetNumber", &Buffer::SetNumber,
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "offset"},
					}
				},
				Attr{UBPA_USREFL_NAME_ARG(1),
					AttrList{
						Attr{Name::name, "value"},
					}
				},
			}
		},
		Field{"InitTable", static_cast<void(Buffer::*)(size_t)>(&Buffer::InitTable),
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "offset"},
					}
				},
			}
		},
		Field{"InitTable", static_cast<void(Buffer::*)(size_t, sol::table)>(&Buffer::InitTable),
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "offset"},
					}
				},
				Attr{UBPA_USREFL_NAME_ARG(1),
					AttrList{
						Attr{Name::name, "value"},
					}
				},
			}
		},
		Field{"GetTable", &Buffer::GetTable,
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "offset"},
					}
				},
			}
		},
		Field{"SetTable", &Buffer::SetTable,
			AttrList {
				Attr{UBPA_USREFL_NAME_ARG(0),
					AttrList{
						Attr{Name::name, "offset"},
					}
				},
				Attr{UBPA_USREFL_NAME_ARG(1),
					AttrList{
						Attr{Name::name, "value"},
					}
				},
			}
		},
	};
};

constexpr int f() {
	int a = 1;
	return a;
}

int main() {
	char buff[256];
	int error;
	lua_State* L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */
	Ubpa::ULuaPP::Register<Buffer>(L);
	{
		sol::state_view lua(L);
		const char code[] = R"(
buf = Buffer.new(32) -- 2 double + 1 table
buf:SetNumber(0, 512)
buf:SetNumber(8, 1024)
print(buf:GetNumber(0))
print(buf:GetNumber(8))
buf:InitTable(16, {hello="world"})
t = buf:GetTable(16)
t["Lua"] = "nice"
for k,v in pairs(buf:GetTable(16)) do print(k, v) end
t = nil
for k,v in pairs(buf:GetTable(16)) do print(k, v) end
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
