#ifndef __DDR_TRAITS_HELPER_MACROS_H_INCLUDED__
#define __DDR_TRAITS_HELPER_MACROS_H_INCLUDED__

#define CREATE_MEMBER_CHECK(memberName)																				\
template <class TT, typename = int> struct Has_Member_##memberName : std::false_type {};								\
template <class TT> struct Has_Member_##memberName<TT, decltype(&TT::memberName, 0)> : std::true_type {};
#define HAS_MEMBER(typeName, memberName) Has_Member_##memberName<typeName>::value

template <typename TT, typename = void> struct got_type : std::false_type {};
template <typename TT> struct got_type<TT> : std::true_type { typedef TT type; };

#define CREATE_MEMBER_VAR_CHECK(varName)																			\
template <class TT, typename = std::true_type> struct Has_MemberVar_##varName : std::false_type {};					\
template <class TT> struct Has_MemberVar_##varName																	\
<TT, std::bool_constant<!std::is_member_function_pointer<decltype(&TT::varName)>::value>> : std::true_type {};
#define HAS_MEMBER_VAR(typeName, varName) Has_MemberVar_##varName<typeName>::value

#define CREATE_MEMBER_CLASS_CHECK(clsName)																			\
template <class TT, typename = std::true_type> struct Has_MemberClass_##clsName : std::false_type {};				\
template <class TT> struct Has_MemberClass_##clsName																	\
<TT, std::bool_constant<std::is_class<typename got_type<typename TT::clsName>::type>::value>> : std::true_type {};
#define HAS_MEMBER_CLASS(typeName, clsName) Has_MemberClass_##clsName<typeName>::value

#define CREATE_MEMBER_UNION_CHECK(unionName)																		\
template <class TT, typename = std::true_type> struct Has_MemberUnion_##unionName : std::false_type {};				\
template <class TT> struct Has_MemberUnion_##unionName																\
<TT, std::bool_constant<std::is_union<typename got_type<typename TT::unionName>::type>::value>> : std::true_type {};
#define HAS_MEMBER_UNION(typeName, unionName) Has_MemberUnion_##unionName<typeName>::value

#define CREATE_MEMBER_ENUM_CHECK(enumName)																			\
template <class TT, typename = std::true_type> struct Has_MemberEnum_##enumName : std::false_type {};				\
template <class TT> struct Has_MemberEnum_##enumName																	\
<TT, std::bool_constant<std::is_enum<typename got_type<typename TT::enumName>::type>::value>> : std::true_type {};
#define HAS_MEMBER_ENUM(typeName, enumName) Has_MemberEnum_##enumName<typeName>::value

template<typename TT, TT> struct sig_check : std::true_type {};

#define CREATE_MEMBER_FUNC_SIG_CHECK(funcName, funcSig, decorName)													\
template<class TT, typename = std::true_type>  struct Has_MemberFuncSig_##decorName : std::false_type{};				\
template<class TT> struct Has_MemberFuncSig_##decorName																\
<TT, std::bool_constant<sig_check<funcSig, &TT::funcName>::value>> : std::true_type {};
#define HAS_MEMBER_FUNCSIG(typeName, decorName) Has_MemberFuncSig_##decorName<typeName>::value

template <typename TT, typename ...ARGS> struct AndBoolTypes : std::false_type {};
template <typename ...ARGS> struct AndBoolTypes<std::true_type, ARGS...> : AndBoolTypes<ARGS...> {};
template <> struct AndBoolTypes<std::true_type> : std::true_type {};

template <typename TT, typename ...ARGS> struct OrBoolTypes : std::true_type {};
template <typename ...ARGS> struct OrBoolTypes<std::false_type, ARGS...> : OrBoolTypes<ARGS...> {};
template <> struct OrBoolTypes<std::false_type> : std::false_type {};

#endif // __DDR_TRAITS_HELPER_MACROS_H_INCLUDED__
