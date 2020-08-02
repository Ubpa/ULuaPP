#pragma once

#include <USRefl/USRefl.h>

#include "_deps/sol2/sol.hpp"

#include <array>
#include <string>

namespace Ubpa::ULuaPP::detail {
	struct NameInfo {
		NameInfo(std::string_view name) {
			size_t cur = name[0] == 's' ? 6 // struct
				: name[0] == 'c' ? 5        // class
				: 4;                        // enum
			std::string str;
			while (++cur < name.size()) {
				if (name[cur] == ':') {
					namespaces.push_back(str);
					str.clear();
					cur++;
				}
				else if (isalnum(name[cur]) || name[cur] == '_')
					str += name[cur];
				else
					str += '_';
			}
			rawName = str;
		}

		std::vector<std::string> namespaces;
		std::string rawName;
	};

	template<typename T, size_t... Ns>
	constexpr auto GetInits(std::index_sequence<Ns...>) {
		constexpr auto masks = Ubpa::USRefl::TypeInfo<T>::fields.Accumulate(
			std::array<bool, Ubpa::USRefl::TypeInfo<T>::fields.size>{},
			[idx = 0](auto&& acc, auto field) mutable {
			acc[idx++] = field.name == USRefl::Name::constructor;
			return std::forward<decltype(acc)>(acc);
		});
		constexpr auto constructors = Ubpa::USRefl::TypeInfo<T>::fields.template Accumulate<masks[Ns]...>(
			std::tuple<>{},
			[](auto acc, auto field) {
				return std::tuple_cat(acc, std::tuple{ field.value });
			}
		);
		if constexpr (std::tuple_size_v<std::decay_t<decltype(constructors)>> > 0)
			return std::apply([](auto...elems) { return sol::initializers(elems...); }, constructors);
		else {
			static_assert(std::is_default_constructible_v<T>);
			return sol::initializers(USRefl::WrapConstructor<T()>());
		}
	}

	template<typename T>
	constexpr auto GetFuncNumOverloadNum() {
		constexpr auto funcFields = Ubpa::USRefl::TypeInfo<T>::DFS_Acc(
			USRefl::ElemList<>{},
			[](auto acc, auto baseType, size_t) {
				return baseType.fields.Accumulate(acc, [](auto acc, auto field) {
					if constexpr (field.is_func)
						return acc.Push(field);
					else
						return acc;
				});
			}
		);

		constexpr auto overloadNames = funcFields.Accumulate(
			std::array<std::string_view, funcFields.size>{},
			[idx = static_cast<size_t>(0)](auto acc, auto func) mutable {
			if (func.name != USRefl::Name::constructor) {
				acc[idx] = func.name;
				for (size_t i = 0; i < idx; i++) {
					if (func.name == acc[i]) {
						acc[idx] = "";
						break;
					}
				}
			}
			else
				acc[idx] = "";
			idx++;
			return acc;
		}
		);

		constexpr auto overloadNum = std::apply([](auto... names) {
				return (0 + ... + static_cast<size_t>(!names.empty()));
			}, overloadNames);

		return std::tuple{ funcFields.size, overloadNum };
	}

	template<typename T, size_t OverloadNum, size_t Index, size_t... Ns>
	constexpr auto GetOverloadFuncListAt(std::index_sequence<Ns...>) {
		constexpr auto funcFields = Ubpa::USRefl::TypeInfo<T>::DFS_Acc(
			USRefl::ElemList<>{},
			[](auto acc, auto baseType, size_t) {
				return baseType.fields.Accumulate(acc, [](auto acc, auto field) {
					if constexpr (field.is_func)
						return acc.Push(field);
					else
						return acc;
				});
			}
		);

		constexpr auto overloadNames = funcFields.Accumulate(
			std::array<std::string_view, funcFields.size>{},
			[idx = static_cast<size_t>(0)](auto acc, auto func) mutable {
			if (func.name != USRefl::Name::constructor) {
				acc[idx] = func.name;
				for (size_t i = 0; i < idx; i++) {
					if (func.name == acc[i]) {
						acc[idx] = "";
						break;
					}
				}
			}
			else
				acc[idx] = "";
			idx++;
			return acc;
		}
		);

		constexpr auto indices = funcFields.Accumulate(
			std::array<size_t, OverloadNum>{},
			[overloadNames, idx = static_cast<size_t>(0), indicesCur = static_cast<size_t>(0)] (auto acc, auto func) mutable {
				if (!overloadNames[idx].empty())
					acc[indicesCur++] = idx;
				idx++;
				return acc;
			}
		);

		constexpr auto name = funcFields.Get<indices[Index]>().name;

		constexpr auto masks = funcFields.Accumulate(
			std::array<bool, funcFields.size>{},
			[name, idx = static_cast<size_t>(0)](auto acc, auto func) mutable {
				acc[idx++] = func.name == name;
				return acc;
			}
		);

		constexpr auto funcList = funcFields.template Accumulate<masks[Ns]...>(
			USRefl::ElemList<>{},
			[](auto acc, auto func) {
				return acc.Push(func);
			}
		);

		return funcList;
	}
	
	template<typename T, size_t... Ns, size_t... Ms>
	constexpr auto GetOverloadFuncListTuple(std::index_sequence<Ns...>, std::index_sequence<Ms...>) {
		constexpr size_t OverloadNum = sizeof...(Ms);
		return std::tuple{ GetOverloadFuncListAt<T, OverloadNum, Ms>(std::index_sequence<Ns...>{})... };
	}

	template<typename T>
	constexpr auto GetOverload() {
		constexpr auto funcNumOverloadNum = GetFuncNumOverloadNum<T>();

		return GetOverloadFuncListTuple<T>(
			std::make_index_sequence<std::get<0>(funcNumOverloadNum)>{},
			std::make_index_sequence<std::get<1>(funcNumOverloadNum)>{}
		);
	}

	template<typename T>
	void RegisterClass(lua_State* L) {
		sol::state_view lua(L);
		sol::table typeinfo = lua["USRefl_TypeInfo"].get_or_create<sol::table>();
		detail::NameInfo nameInfo(Ubpa::USRefl::TypeInfo<T>::name);
		
		sol::usertype<T> type = lua.new_usertype<T>(nameInfo.rawName,
			detail::GetInits<T>(std::make_index_sequence<Ubpa::USRefl::TypeInfo<T>::fields.size>{}));

		sol::table typeinfo_type = typeinfo[nameInfo.rawName].get_or_create<sol::table>();
		sol::table typeinfo_type_attrs = typeinfo_type["attrs"].get_or_create<sol::table>();
		sol::table typeinfo_type_fields = typeinfo_type["fields"].get_or_create<sol::table>();
		USRefl::TypeInfo<T>::attrs.ForEach([&](auto attr) {
			if constexpr (attr.has_value)
				typeinfo_type_attrs[attr.name] = attr.value;
			else
				typeinfo_type_attrs[attr.name] = true; // default
		});

		constexpr auto overloadFuncListTuple = detail::GetOverload<T>();
		std::apply([&](auto... funcLists) {
			(std::apply([&](auto... funcs) {
				auto packedFuncs = sol::overload(funcs.value...);
				auto name = std::get<0>(std::tuple{ funcs... }).name;
				if (name == "operator+")
					type[sol::meta_function::addition] = packedFuncs;
				else if (name == "operator-")
					type[sol::meta_function::subtraction] = packedFuncs;
				else if (name == "operator*")
					type[sol::meta_function::multiplication] = packedFuncs;
				else if (name == "operator/")
					type[sol::meta_function::division] = packedFuncs;
				else if (name == "operator<")
					type[sol::meta_function::less_than] = packedFuncs;
				else if (name == "operator<=")
					type[sol::meta_function::less_than_or_equal_to] = packedFuncs;
				else if (name == "operator==")
					type[sol::meta_function::equal_to] = packedFuncs;
				else if (name == "operator[]")
					type[sol::meta_function::index] = packedFuncs;
				else if (name == "operator()")
					type[sol::meta_function::call] = packedFuncs;
				else
					type[name] = packedFuncs;
				constexpr bool needPostfix = sizeof...(funcs) > 0;
				USRefl::ElemList{ funcs... }.ForEach([&, idx = static_cast<size_t>(0)](auto func)mutable{
					std::string name = std::string(func.name) + (needPostfix ? ("_" + std::to_string(idx)) : "");
					sol::table typeinfo_type_fields_field = typeinfo_type_fields[name].get_or_create<sol::table>();
					sol::table typeinfo_type_fields_field_attrs = typeinfo_type_fields_field["attrs"].get_or_create<sol::table>();
					if constexpr (func.attrs.size > 0) {
						func.attrs.ForEach([&](auto attr) {
							if constexpr (attr.has_value)
								typeinfo_type_fields_field_attrs[attr.name] = attr.value;
							else
								typeinfo_type_fields_field_attrs[attr.name] = true; // default
						});
					}
					idx++;
				});
			}, funcLists.elems), ...);
		}, overloadFuncListTuple);

		// variable
		USRefl::TypeInfo<T>::DFS_ForEach([&](auto t, size_t) {
			t.fields.ForEach([&](auto field) {
				if constexpr (!field.is_func) {
					type[field.name] = field.value;

					sol::table typeinfo_type_fields_field = typeinfo_type_fields[field.name].get_or_create<sol::table>();
					sol::table typeinfo_type_fields_field_attrs = typeinfo_type_fields_field["attrs"].get_or_create<sol::table>();
					if constexpr (field.attrs.size > 0) {
						field.attrs.ForEach([&](auto attr) {
							if constexpr (attr.has_value)
								typeinfo_type_fields_field_attrs[attr.name] = attr.value;
							else
								typeinfo_type_fields_field_attrs[attr.name] = true; // default
						});
					}
				}
			});
		});
	}

	template<typename T>
	void RegisterEnum(lua_State* L) {
		sol::state_view lua(L);
		detail::NameInfo nameInfo(Ubpa::USRefl::TypeInfo<T>::name);
		sol::table typeinfo = lua["USRefl_TypeInfo"].get_or_create<sol::table>();
		sol::table typeinfo_enum = typeinfo[nameInfo.rawName].get_or_create<sol::table>();
		sol::table typeinfo_enum_attrs = typeinfo_enum["attrs"].get_or_create<sol::table>();
		sol::table typeinfo_enum_fields = typeinfo_enum["fields"].get_or_create<sol::table>();

		USRefl::TypeInfo<T>::attrs.ForEach([&](auto attr) {
			if constexpr (attr.has_value)
				typeinfo_enum_attrs[attr.name] = attr.value;
			else
				typeinfo_enum_attrs[attr.name] = true; // default
		});

		USRefl::TypeInfo<T>::fields.ForEach(
			[&](auto field) {
				sol::table typeinfo_enum_fields_field = typeinfo_enum_fields[field.name].get_or_create<sol::table>();
				sol::table typeinfo_type_fields_field_attrs = typeinfo_enum_fields_field["attrs"].get_or_create<sol::table>();
				field.attrs.ForEach([&](auto attr) {
					if constexpr (attr.has_value)
						typeinfo_type_fields_field_attrs[attr.name] = attr.value;
					else
						typeinfo_type_fields_field_attrs[attr.name] = true; // default
				});
			}
		);

		constexpr auto nvs = USRefl::TypeInfo<T>::fields.Accumulate(
			std::tuple<>{},
			[](auto acc, auto field) {
				return std::tuple_cat(acc, std::tuple{ field.name, field.value });
			}
		);
		std::apply([&](auto... values) {
			lua.new_enum(nameInfo.rawName, values...);
		}, nvs);
	}
}

namespace Ubpa::ULuaPP {
	template<typename T>
	void Register(lua_State* L) {
		if constexpr (std::is_enum_v<T>)
			detail::RegisterEnum<T>(L);
		else
			detail::RegisterClass<T>(L);
	}
}
