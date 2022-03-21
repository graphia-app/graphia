#ifndef EXCEPTIONRECORD_H
#define EXCEPTIONRECORD_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <Windows.h>

#include <typeinfo>
#include <exception>
#include <cstdint>
#include <cstring>

// https://stackoverflow.com/questions/39113168
static const char* exceptionRecordType(const EXCEPTION_RECORD* exr)
{
#pragma pack(push, 4)
    struct _PMD
    {
        int mdisp;
        int pdisp;
        int vdisp;
    };

    struct CatchableType
    {
        int32_t properties;
        int32_t pType;
        _PMD    thisDisplacement;
        int32_t sizeOrOffset;
        int32_t copyFunction;
    };

    struct ThrowInfo
    {
        int32_t attributes;
        int32_t pmfnUnwind;
        int32_t pForwardCompat;
        int32_t pCatchableTypeArray;
    };

    struct CatchableTypeArray
    {
        int nCatchableTypes;
        CatchableType *arrayOfCatchableTypes[];
    };
#pragma pack (pop)


    HMODULE module = (exr->NumberParameters >= 4) ?
        reinterpret_cast<HMODULE>(exr->ExceptionInformation[3]) : nullptr;

    if(exr->NumberParameters < 2)
        return nullptr;

    const auto* throwInfo = reinterpret_cast<const ThrowInfo*>(exr->ExceptionInformation[2]);

    if(throwInfo == nullptr)
        return nullptr;

#define RVA_TO_VA(TYPE, ADDR) (reinterpret_cast<TYPE>( \
    (uintptr_t)(module) + (uint32_t)(ADDR)))


    const auto* cArray = RVA_TO_VA(const CatchableTypeArray*, throwInfo->pCatchableTypeArray);
    const auto* cType = RVA_TO_VA(const CatchableType*, cArray->arrayOfCatchableTypes[0]);

    const auto* type = RVA_TO_VA(const std::type_info*, cType->pType);

#undef RVA_TO_VA

    // Honestly, this is pretty sketchy, on multiple levels, but should yield
    // a bit more information when a std::exception occurs
    if(std::strstr(type->name(), "class std::") == type->name())
    {
        const auto* stdException = reinterpret_cast<const std::exception*>(exr->ExceptionInformation[1]);

        static char stdExceptionText[1024] = {0};
        strncat(stdExceptionText, type->name(), sizeof(stdExceptionText) - 1);
        strncat(stdExceptionText, ": ", sizeof(stdExceptionText) - 1);
        strncat(stdExceptionText, stdException->what(), sizeof(stdExceptionText) - 1);

        return stdExceptionText;
    }

    return type->name();
}

#endif // Q_OS_WIN32

#endif // EXCEPTIONRECORD_H
