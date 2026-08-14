#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Util/MarcioCommonLibsConfiguration.h"

#include <sstream>

#define FUNCTIONSTR2(x) #x
#define FUNCTIONSTR TEXT(FUNCTIONSTR2(__FUNCTION__))

class CommaLog
{
public:
	inline CommaLog&
	operator,(const TCHAR* value)
	{
		for (size_t x = 0; value[x]; x++)
		{
			wos << static_cast<wchar_t>(value[x]);
		}

		return *this;
	}

	inline CommaLog&
	operator,(const FString& value)
	{
		for (TCHAR ch : value)
		{
			wos << static_cast<wchar_t>(ch);
		}

		return *this;
	}

	inline CommaLog&
	operator,(const FText& value)
	{
		for (TCHAR ch : value.ToString())
		{
			wos << static_cast<wchar_t>(ch);
		}

		return *this;
	}

	template <typename T>
	inline CommaLog&
	operator,(const T& value)
	{
		wos << value;

		return *this;
	}
	
	std::wostringstream wos;
};

DECLARE_LOG_CATEGORY_EXTERN(LogMarcioCommonLibs, Log, All)

#if PLATFORM_LINUX
#define MCL_LOG_Verbosity(verbosity, first, ...) \
	do { \
		CommaLog l; \
		l, first __VA_OPT__(,) __VA_ARGS__; \
		const auto Message = l.wos.str(); \
		const auto AnsiStr = StringCast<ANSICHAR>(Message.c_str()); \
		UE_LOG(LogMarcioCommonLibs, verbosity, TEXT("%hs"), AnsiStr.Get()); \
	} while (false)
#else
#define MCL_LOG_Verbosity(verbosity, first, ...) \
	do { \
		CommaLog l; \
		l, first __VA_OPT__(,) __VA_ARGS__; \
		const auto Message = l.wos.str(); \
		UE_LOG(LogMarcioCommonLibs, verbosity, TEXT("%s"), Message.c_str()); \
	} while (false)
#endif

#define MCL_LOG_Log(first, ...) MCL_LOG_Verbosity(Log, first __VA_OPT__(,) __VA_ARGS__)
#define MCL_LOG_Display(first, ...) MCL_LOG_Verbosity(Display, first __VA_OPT__(,) __VA_ARGS__)
#define MCL_LOG_Warning(first, ...) MCL_LOG_Verbosity(Warning, first __VA_OPT__(,) __VA_ARGS__)
#define MCL_LOG_Error(first, ...) MCL_LOG_Verbosity(Error, first __VA_OPT__(,) __VA_ARGS__)

#define IS_MCL_LOG_LEVEL(level) (UMarcioCommonLibsConfiguration::GetLogLevelMCL() > 0 && UMarcioCommonLibsConfiguration::GetLogLevelMCL() >= static_cast<int32>(level))

#define MCL_LOG_Log_Condition(first, ...) do { if (IS_MCL_LOG_LEVEL(ELogVerbosity::Log)) { MCL_LOG_Log(first __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define MCL_LOG_Display_Condition(first, ...) do { if (IS_MCL_LOG_LEVEL(ELogVerbosity::Display)) { MCL_LOG_Display(first __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define MCL_LOG_Warning_Condition(first, ...) do { if (IS_MCL_LOG_LEVEL(ELogVerbosity::Warning)) { MCL_LOG_Warning(first __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define MCL_LOG_Error_Condition(first, ...) do { if (IS_MCL_LOG_LEVEL(ELogVerbosity::Error)) { MCL_LOG_Error(first __VA_OPT__(,) __VA_ARGS__); } } while (false)
