#pragma once

#include <USRefl/USRefl.h>
#include <UTemplate/Func.h>
#include <USTL/tuple.h>

#include <sol/sol.hpp>

#include <array>
#include <string>

namespace Ubpa::ULuaPP::detail {
	struct NameInfo {
		NameInfo(std::string_view name) {
			size_t cur = 0;
			if (name.size() >= 7 && name.substr(0, 7) == "struct ")
				cur = 7;
			if (name.size() >= 6 && name.substr(0, 6) == "class ")
				cur = 6;
			if (name.size() >= 5 && name.substr(0, 5) == "enum ")
				cur = 5;
			std::string str;
			while (cur < name.size()) {
				if (name[cur] == ':') {
					namespaces.push_back(str);
					str.clear();
					cur++;
				}
				else if (isalnum(name[cur]) || name[cur] == '_')
					str += name[cur];
				else
					str += '_';
				cur++;
			}
			rawName = str;
		}

		std::vector<std::string> namespaces;
		std::string rawName;
	};

	template<typename ArgList>
	static constexpr bool ContainsRVRef = Length_v<Filter_t<ArgList, std::is_rvalue_reference>> > 0;

	template<typename T>
	constexpr auto DFS_GetFuncFieldList() {
		return USRefl::TypeInfo<T>::DFS_Acc(
			USRefl::ElemList<>{},
			[](auto acc, auto t, size_t) {
				return t.fields.Accumulate(
					acc,
					[](auto acc, auto field) {
						if constexpr (field.is_func) {
							using Func = std::decay_t<decltype(field.value)>;
							if constexpr (!ContainsRVRef<FuncTraits_ArgList<Func>>)
								return acc.Push(field);
							else
								return acc;
						}
						else
							return acc;
					}
				);
			}
		);
	}

	template<typename T, size_t... Ns>
	constexpr auto GetInits(std::index_sequence<Ns...>) {
		constexpr auto fields = USRefl::TypeInfo<T>::fields;
		constexpr auto constructors = fields.Accumulate(
			std::tuple<>{},
			[](auto acc, auto field) {
				if constexpr (field.NameIs(TSTR(UMeta::constructor))) {
					using MainFunc = std::decay_t<decltype(field.value)>;
					if constexpr (!ContainsRVRef<FuncTraits_ArgList<MainFunc>>) {
						auto rst = USTL::tuple_append(acc, field.value);
						if constexpr (field.attrs.Contains(TSTR(UMeta::default_functions))) {
							const auto& defaultFuncs = field.attrs.Find(TSTR(UMeta::default_functions)).value;
							auto new_rst = USTL::tuple_accumulate(defaultFuncs, rst, [](auto acc, auto func) {
								using Func = std::decay_t<decltype(func)>;
								if constexpr (ContainsRVRef<FuncTraits_ArgList<Func>>)
									return acc;
								else
									return USTL::tuple_append(acc, func);
								});
							return new_rst;
						}
						else
							return rst;
					}
					else
						return acc;
				}
				else
					return acc;
			}
		);
		if constexpr (std::tuple_size_v<std::decay_t<decltype(constructors)>> > 0)
			return std::apply([](auto...elems) { return sol::initializers(elems...); }, constructors);
		else if constexpr (std::is_default_constructible_v<T>)
			return sol::initializers(USRefl::WrapConstructor<T()>());
		else
			return sol::no_constructor;
	}

	template<size_t Index, typename T, typename FuncFieldList, size_t... Ns>
	void SetOverloadFuncsImpl(
		sol::usertype<T>& type,
		FuncFieldList funcFieldList,
		std::index_sequence<Ns...> seq
	) {
		if constexpr (Index != static_cast<size_t>(-1)) {
			using TheFuncField = std::decay_t<decltype(funcFieldList.template Get<Index>())>;
			using Name = typename TheFuncField::Tag;
			auto funcs = funcFieldList.Accumulate(
				std::tuple<>{},
				[](auto acc, auto field) {
					if constexpr (field.template NameIs<Name>()) {
						using MainFunc = std::decay_t<decltype(field.value)>;
						if constexpr (!ContainsRVRef<FuncTraits_ArgList<MainFunc>>) {
							auto rst = USTL::tuple_append(acc, field.value);
							if constexpr (field.attrs.Contains(TSTR(UMeta::default_functions))) {
								const auto& defaultFuncs = field.attrs.Find(TSTR(UMeta::default_functions)).value;
								auto new_rst = USTL::tuple_accumulate(defaultFuncs, rst, [](auto acc, auto func) {
									using Func = std::decay_t<decltype(func)>;
									if constexpr (ContainsRVRef<FuncTraits_ArgList<Func>>)
										return acc;
									else
										return USTL::tuple_append(acc, func);
								});
								return new_rst;
							}
							else
								return rst;
						}
						else
							return acc;
					}
					else
						return acc;
				}
			);
			auto packedFuncs = std::apply([](auto... funcs) {return sol::overload(funcs...); }, funcs);
			constexpr auto name = Name::name;
			assert(name != "voidp");
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
		}
	}

	template<typename List>
	struct OverloadFalgsOf;
	template<typename... Fields>
	struct OverloadFalgsOf<USRefl::ElemList<Fields...>> {
		static constexpr auto get() {
			constexpr auto names = std::array{ Fields::name... };
			constexpr size_t N = sizeof...(Fields);
			std::array<size_t, N> flags{};
			for (size_t i = 0; i < N; i++) {
				flags[i] = i;
				for (size_t j = 0; j < i; j++) {
					if(names[j] == names[i]) {
						flags[i] = static_cast<size_t>(-1);
						break;
					}
				}
			}
			return flags;
		}
	};
	
	template<typename T, typename FuncFieldList, size_t... Ns>
	void SetFuncsImpl(
		sol::usertype<T>& type,
		FuncFieldList funcFieldList,
		std::index_sequence<Ns...> seq
	) {
		constexpr auto indices = OverloadFalgsOf<FuncFieldList>::get();
		(SetOverloadFuncsImpl<indices[Ns]>(type, funcFieldList, seq), ...);
	}

	template<typename T>
	void SetFuncs(sol::usertype<T>& type) {
		constexpr auto funcFieldList = DFS_GetFuncFieldList<T>();
		if constexpr (funcFieldList.size > 0)
			SetFuncsImpl(type, funcFieldList, std::make_index_sequence<funcFieldList.size>());
	}

	template<typename T>
	void RegisterClass(lua_State* L) {
		sol::state_view lua(L);
		
		NameInfo nameInfo(USRefl::TypeInfo<T>::name);

		sol::usertype<T> type;
		if (nameInfo.namespaces.empty()) {
			type = lua.new_usertype<T>(
				nameInfo.rawName,
				GetInits<T>(std::make_index_sequence<USRefl::TypeInfo<T>::fields.size>{})
			);
		}
		else {
			sol::table nsTable = lua[nameInfo.namespaces.front()].get_or_create<sol::table>();
			for (size_t i = 1; i < nameInfo.namespaces.size(); i++)
				nsTable = nsTable[nameInfo.namespaces[i]].get_or_create<sol::table>();
			type = nsTable.new_usertype<T>(
				nameInfo.rawName,
				GetInits<T>(std::make_index_sequence<USRefl::TypeInfo<T>::fields.size>{})
			);
		}

		// set void* cast
		type["voidp"] = [](void* p) {return static_cast<T*>(p); };

		SetFuncs(type);

		// variable
		USRefl::TypeInfo<T>::DFS_ForEach([&](auto t, size_t) {
			t.fields.ForEach([&](auto field) {
				if constexpr (!field.is_func) {
					using Value = std::decay_t<decltype(field.value)>;
					if constexpr (std::is_member_object_pointer_v<Value>)
						type[field.name] = field.value;
					else if constexpr (std::is_pointer_v<Value>)
						type[field.name] = sol::var(std::ref(*field.value));
					else
						type[field.name] = sol::property([v = field.value](){return v; });
				}
			});
		});
	}

	template<typename T>
	void RegisterEnum(lua_State* L) {
		sol::state_view lua(L);
		NameInfo nameInfo(USRefl::TypeInfo<T>::name);
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
