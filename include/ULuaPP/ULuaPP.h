#pragma once

#include <lua.hpp>

namespace Ubpa::ULuaPP {
	template<typename T>
	void Register(lua_State* L);
}

#include "details/ULuaPP.inl"
