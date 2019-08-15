#ifndef __DDR_TRAITS_HELPER_MACROS_H_INCLUDED__
#define __DDR_TRAITS_HELPER_MACROS_H_INCLUDED__

#define CREATE_MEMBER_CHECK(memberName)																				\
template <class T, typename = int> struct Has_Member_##memberName : std::false_type {};								\
template <class T> struct Has_Member_##memberName<T, decltype(&T::##memberName, 0)> : std::true_type {};
#define HAS_MEMBER(typeName, memberName) Has_Member_##memberName<typeName>::value

template <typename T, typename = void> struct got_type : std::false_type {};
template <typename T> struct got_type<T> : std::true_type { typedef T type; };

#define CREATE_MEMBER_VAR_CHECK(varName)																			\
template <class T, typename = std::true_type> struct Has_MemberVar_##varName : std::false_type {};					\
template <class T> struct Has_MemberVar_##varName																	\
<T, std::bool_constant<!std::is_member_function_pointer<decltype(&T::varName)>::value>> : std::true_type {};
#define HAS_MEMBER_VAR(typeName, varName) Has_MemberVar_##varName<typeName>::value

#define CREATE_MEMBER_CLASS_CHECK(clsName)																			\
template <class T, typename = std::true_type> struct Has_MemberClass_##clsName : std::false_type {};				\
template <class T> struct Has_MemberClass_##clsName																	\
<T, std::bool_constant<std::is_class<typename got_type<typename T::clsName>::type>::value>> : std::true_type {};
#define HAS_MEMBER_CLASS(typeName, clsName) Has_MemberClass_##clsName<typeName>::value

#define CREATE_MEMBER_UNION_CHECK(unionName)																		\
template <class T, typename = std::true_type> struct Has_MemberUnion_##unionName : std::false_type {};				\
template <class T> struct Has_MemberUnion_##unionName																\
<T, std::bool_constant<std::is_union<typename got_type<typename T::unionName>::type>::value>> : std::true_type {};
#define HAS_MEMBER_UNION(typeName, unionName) Has_MemberUnion_##unionName<typeName>::value

#define CREATE_MEMBER_ENUM_CHECK(enumName)																			\
template <class T, typename = std::true_type> struct Has_MemberEnum_##enumName : std::false_type {};				\
template <class T> struct Has_MemberEnum_##enumName																	\
<T, std::bool_constant<std::is_enum<typename got_type<typename T::enumName>::type>::value>> : std::true_type {};
#define HAS_MEMBER_ENUM(typeName, enumName) Has_MemberEnum_##enumName<typeName>::value

template<typename T, T> struct sig_check : std::true_type {};

#define CREATE_MEMBER_FUNC_SIG_CHECK(funcName, funcSig, decorName)													\
template<class T, typename = std::true_type>  struct Has_MemberFuncSig_##decorName : std::false_type{};				\
template<class T> struct Has_MemberFuncSig_##decorName																\
<T, std::bool_constant<sig_check<funcSig, &T::funcName>::value>> : std::true_type {};
#define HAS_MEMBER_FUNCSIG(typeName, decorName) Has_MemberFuncSig_##decorName<typeName>::value

template <typename T, typename ...ARGS> struct AndBoolTypes : std::false_type {};
template <typename ...ARGS> struct AndBoolTypes<std::true_type, ARGS...> : AndBoolTypes<ARGS...> {};
template <> struct AndBoolTypes<std::true_type> : std::true_type {};

template <typename T, typename ...ARGS> struct OrBoolTypes : std::true_type {};
template <typename ...ARGS> struct OrBoolTypes<std::false_type, ARGS...> : OrBoolTypes<ARGS...> {};
template <> struct OrBoolTypes<std::false_type> : std::false_type {};

#endif // __DDR_TRAITS_HELPER_MACROS_H_INCLUDED__
