/*##############################################################################

 HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ############################################################################## */

#include "platform.h"
#include "RInside.h"
#include "jexcept.hpp"
#include "jthread.hpp"
#include "hqlplugins.hpp"
#include "deftype.hpp"
#include "eclrtl.hpp"
#include "eclrtl_imp.hpp"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

static const char * compatibleVersions[] =
{ "R Embed Helper 1.0.0", NULL };

static const char *version = "R Embed Helper 1.0.0";

extern "C" EXPORT bool getECLPluginDefinition(ECLPluginDefinitionBlock *pb)
{
    if (pb->size == sizeof(ECLPluginDefinitionBlockEx))
    {
        ECLPluginDefinitionBlockEx * pbx = (ECLPluginDefinitionBlockEx *) pb;
        pbx->compatibleVersions = compatibleVersions;
    }
    else if (pb->size != sizeof(ECLPluginDefinitionBlock))
        return false;
    pb->magicVersion = PLUGIN_VERSION;
    pb->version = version;
    pb->moduleName = "+R+"; // Hack - we don't want to export any ECL, but if we don't export something,
    pb->ECL = "";           // Hack - the dll is unloaded at startup when compiling, and the R runtime closes stdin when unloaded
    pb->flags = PLUGIN_MULTIPLE_VERSIONS;
    pb->description = "R Embed Helper";
    return true;
}

#ifdef _WIN32
    EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace Rembed
{

// Use a global object to ensure that the R instance is initialized only once
// Because of R's dodgy stack checks, we also have to do so on main thread

static class RGlobalState
{
public:
    RGlobalState()
    {
        const char *args[] = {"R", "--slave" };
        R = new RInside(2, args, true, false, true);  // Setting interactive mode=true prevents R syntax errors from terminating the process
        // The R code for checking stack limits assumes that all calls are on the same thread
        // as the original context was created on - this will not always be true in ECL (and hardly
        // ever true in Roxie
        // Setting the stack limit to -1 disables this check
        R_CStackLimit = -1;
// Make sure we are never unloaded (as R does not support it)
// we do this by doing a dynamic load of the Rembed library
#ifdef _WIN32
        char path[_MAX_PATH];
        ::GetModuleFileName((HINSTANCE)&__ImageBase, path, _MAX_PATH);
        if (strstr(path, "Rembed"))
        {
            HINSTANCE h = LoadSharedObject(path, false, false);
            DBGLOG("LoadSharedObject returned %p", h);
        }
#else
        FILE *diskfp = fopen("/proc/self/maps", "r");
        if (diskfp)
        {
            char ln[_MAX_PATH];
            while (fgets(ln, sizeof(ln), diskfp))
            {
                if (strstr(ln, "libRembed"))
                {
                    const char *fullName = strchr(ln, '/');
                    if (fullName)
                    {
                        char *tail = (char *) strstr(fullName, SharedObjectExtension);
                        if (tail)
                        {
                            tail[strlen(SharedObjectExtension)] = 0;
                            HINSTANCE h = LoadSharedObject(fullName, false, false);
                            break;
                        }
                    }
                }
            }
            fclose(diskfp);
        }
#endif
    }
    ~RGlobalState()
    {
        delete R;
    }
    RInside *R;
}* globalState = NULL;

static CriticalSection RCrit;  // R is single threaded - need to own this before making any call to R

static RGlobalState *queryGlobalState()
{
    CriticalBlock b(RCrit);
    if (!globalState)
        globalState = new RGlobalState;
    return globalState;
}

extern void unload()
{
    CriticalBlock b(RCrit);
    if (globalState)
        delete globalState;
    globalState = NULL;
}

MODULE_INIT(INIT_PRIORITY_STANDARD)
{
    queryGlobalState(); // make sure gets loaded by main thread
    return true;
}
MODULE_EXIT()
{
// Don't unload, because R seems to have problems with being reloaded, i.e. crashes on next use
//    unload();
}

// Each call to a R function will use a new REmbedFunctionContext object
// This takes care of ensuring that the critsec is locked while we are executing R code,
// and released when we are not

class REmbedFunctionContext: public CInterfaceOf<IEmbedFunctionContext>
{
public:
    REmbedFunctionContext(RInside &_R, const char *options)
    : R(_R), block(RCrit), result(R_NilValue)
    {
    }
    ~REmbedFunctionContext()
    {
    }

    virtual bool getBooleanResult()
    {
        try
        {
            return ::Rcpp::as<bool>(result);
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
    virtual void getDataResult(size32_t &__len, void * &__result)
    {
        try
        {
            std::vector<byte> vval = ::Rcpp::as<std::vector<byte> >(result);
            rtlStrToDataX(__len, __result, vval.size(), vval.data());
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
    virtual double getRealResult()
    {
        try
        {
            return ::Rcpp::as<double>(result);
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
    virtual __int64 getSignedResult()
    {
        try
        {
            return ::Rcpp::as<long int>(result); // Should really be long long, but RInside does not support that
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
    virtual unsigned __int64 getUnsignedResult()
    {
        try
        {
            return ::Rcpp::as<unsigned long int>(result); // Should really be long long, but RInside does not support that
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
    virtual void getStringResult(size32_t &__len, char * &__result)
    {
        try
        {
            std::string str = ::Rcpp::as<std::string>(result);
            rtlStrToStrX(__len, __result, str.length(), str.data());
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
    virtual void getUTF8Result(size32_t &chars, char * &result)
    {
        throw MakeStringException(MSGAUD_user, 0, "Rembed: %s: Unicode/UTF8 results not supported", func.c_str());
    }
    virtual void getUnicodeResult(size32_t &chars, UChar * &result)
    {
        throw MakeStringException(MSGAUD_user, 0, "Rembed: %s: Unicode/UTF8 results not supported", func.c_str());
    }
    virtual void getSetResult(bool & __isAllResult, size32_t & __resultBytes, void * & __result, int _elemType, size32_t elemSize)
    {
        try
        {
            type_t elemType = (type_t) _elemType;
            __isAllResult = false;
            switch(elemType)
            {

#define FETCH_ARRAY(type) \
{  \
    std::vector<type> vval = ::Rcpp::as< std::vector<type> >(result); \
    rtlStrToDataX(__resultBytes, __result, vval.size()*elemSize, (const void *) vval.data()); \
}

            case type_boolean:
            {
                std::vector<bool> vval = ::Rcpp::as< std::vector<bool> >(result);
                size32_t size = vval.size();
                // Vector of bool is odd, and can't be retrieved via data()
                // Instead we need to iterate, I guess
                rtlDataAttr out(size);
                bool *outData = (bool *) out.getdata();
                for (std::vector<bool>::iterator iter = vval.begin(); iter < vval.end(); iter++)
                {
                    *outData++ = *iter;
                }
                __resultBytes = size;
                __result = out.detachdata();
                break;
            }
            case type_int:
                /* if (elemSize == sizeof(signed char))  // rcpp does not seem to support...
                    FETCH_ARRAY(signed char)
                else */ if (elemSize == sizeof(short))
                    FETCH_ARRAY(short)
                else if (elemSize == sizeof(int))
                    FETCH_ARRAY(int)
                else if (elemSize == sizeof(long))    // __int64 / long long does not work...
                    FETCH_ARRAY(long)
                else
                    rtlFail(0, "Rembed: Unsupported result type");
                break;
            case type_unsigned:
                if (elemSize == sizeof(byte))
                    FETCH_ARRAY(byte)
                else if (elemSize == sizeof(unsigned short))
                    FETCH_ARRAY(unsigned short)
                else if (elemSize == sizeof(unsigned int))
                    FETCH_ARRAY(unsigned int)
                else if (elemSize == sizeof(unsigned long))    // __int64 / long long does not work...
                    FETCH_ARRAY(unsigned long)
                else
                    rtlFail(0, "Rembed: Unsupported result type");
                break;
            case type_real:
                if (elemSize == sizeof(float))
                    FETCH_ARRAY(float)
                else if (elemSize == sizeof(double))
                    FETCH_ARRAY(double)
                else
                    rtlFail(0, "Rembed: Unsupported result type");
                break;
            case type_string:
            case type_varstring:
            {
                std::vector<std::string> vval = ::Rcpp::as< std::vector<std::string> >(result);
                size32_t numResults = vval.size();
                rtlRowBuilder out;
                byte *outData = NULL;
                size32_t outBytes = 0;
                if (elemSize != UNKNOWN_LENGTH)
                {
                    outBytes = numResults * elemSize;  // MORE - check for overflow?
                    out.ensureAvailable(outBytes);
                    outData = out.getbytes();
                }
                for (std::vector<std::string>::iterator iter = vval.begin(); iter < vval.end(); iter++)
                {
                    size32_t lenBytes = (*iter).size();
                    const char *text = (*iter).data();
                    if (elemType == type_string)
                    {
                        if (elemSize == UNKNOWN_LENGTH)
                        {
                            out.ensureAvailable(outBytes + lenBytes + sizeof(size32_t));
                            outData = out.getbytes() + outBytes;
                            * (size32_t *) outData = lenBytes;
                            rtlStrToStr(lenBytes, outData+sizeof(size32_t), lenBytes, text);
                            outBytes += lenBytes + sizeof(size32_t);
                        }
                        else
                        {
                            rtlStrToStr(elemSize, outData, lenBytes, text);
                            outData += elemSize;
                        }
                    }
                    else
                    {
                        if (elemSize == UNKNOWN_LENGTH)
                        {
                            out.ensureAvailable(outBytes + lenBytes + 1);
                            outData = out.getbytes() + outBytes;
                            rtlStrToVStr(0, outData, lenBytes, text);
                            outBytes += lenBytes + 1;
                        }
                        else
                        {
                            rtlStrToVStr(elemSize, outData, lenBytes, text);  // Fixed size null terminated strings... weird.
                            outData += elemSize;
                        }
                    }
                }
                __resultBytes = outBytes;
                __result = out.detachdata();
                break;
            }
            default:
                rtlFail(0, "REmbed: Unsupported result type");
                break;
            }
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }

    virtual void bindBooleanParam(const char *name, bool val)
    {
        R[name] = val;
    }
    virtual void bindDataParam(const char *name, size32_t len, const void *val)
    {
        std::vector<byte> vval;
        const byte *cval = (const byte *) val;
        vval.assign(cval, cval+len);
        R[name] = vval;
    }
    virtual void bindRealParam(const char *name, double val)
    {
        R[name] = val;
    }
    virtual void bindSignedParam(const char *name, __int64 val)
    {
        R[name] = (long int) val;
    }
    virtual void bindUnsignedParam(const char *name, unsigned __int64 val)
    {
        R[name] = (unsigned long int) val;
    }
    virtual void bindStringParam(const char *name, size32_t len, const char *val)
    {
        std::string s(val, len);
        R[name] = s;
    }
    virtual void bindVStringParam(const char *name, const char *val)
    {
        R[name] = val;
    }
    virtual void bindUTF8Param(const char *name, size32_t chars, const char *val)
    {
        rtlFail(0, "Rembed: Unsupported parameter type UTF8");
    }
    virtual void bindUnicodeParam(const char *name, size32_t chars, const UChar *val)
    {
        rtlFail(0, "Rembed: Unsupported parameter type UNICODE");
    }

    virtual void bindSetParam(const char *name, int _elemType, size32_t elemSize, bool isAll, size32_t totalBytes, void *setData)
    {
        if (isAll)
            rtlFail(0, "Rembed: Unsupported parameter type ALL");
        type_t elemType = (type_t) _elemType;
        int numElems = totalBytes / elemSize;
        switch(elemType)
        {

#define BIND_ARRAY(type) \
{  \
    std::vector<type> vval; \
    const type *start = (const type *) setData; \
    vval.assign(start, start+numElems); \
    R[name] = vval; \
}

        case type_boolean:
            BIND_ARRAY(bool)
            break;
        case type_int:
            /* if (elemSize == sizeof(signed char))  // No binding exists in rcpp
                BIND_ARRAY(signed char)
            else */ if (elemSize == sizeof(short))
                BIND_ARRAY(short)
            else if (elemSize == sizeof(int))
                BIND_ARRAY(int)
            else if (elemSize == sizeof(long))    // __int64 / long long does not work...
                BIND_ARRAY(long)
            else
                rtlFail(0, "Rembed: Unsupported parameter type");
            break;
        case type_unsigned:
            if (elemSize == sizeof(unsigned char))
                BIND_ARRAY(unsigned char)
            else if (elemSize == sizeof(unsigned short))
                BIND_ARRAY(unsigned short)
            else if (elemSize == sizeof(unsigned int))
                BIND_ARRAY(unsigned int)
            else if (elemSize == sizeof(unsigned long))    // __int64 / long long does not work...
                BIND_ARRAY(unsigned long)
            else
                rtlFail(0, "Rembed: Unsupported parameter type");
            break;
        case type_real:
            if (elemSize == sizeof(float))
                BIND_ARRAY(float)
            else if (elemSize == sizeof(double))
                BIND_ARRAY(double)
            else
                rtlFail(0, "Rembed: Unsupported parameter type");
            break;
        case type_string:
        case type_varstring:
        {
            std::vector<std::string> vval;
            const byte *inData = (const byte *) setData;
            const byte *endData = inData + totalBytes;
            while (inData < endData)
            {
                int thisSize;
                if (elemSize == UNKNOWN_LENGTH)
                {
                    if (elemType==type_varstring)
                        thisSize = strlen((const char *) inData) + 1;
                    else
                    {
                        thisSize = * (size32_t *) inData;
                        inData += sizeof(size32_t);
                    }
                }
                else
                    thisSize = elemSize;
                std::string s((const char *) inData, thisSize);
                vval.push_back(s);
                inData += thisSize;
                numElems++;
            }
            R[name] = vval;
            break;
        }
        default:
            rtlFail(0, "REmbed: Unsupported parameter type");
            break;
        }
    }

    virtual void importFunction(size32_t lenChars, const char *utf)
    {
        throwUnexpected();
    }
    virtual void compileEmbeddedScript(size32_t lenChars, const char *utf)
    {
        StringBuffer text(rtlUtf8Size(lenChars, utf), utf);
        text.stripChar('\r');
        func.assign(text.str());
    }

    virtual void callFunction()
    {
        try
        {
            result = R.parseEval(func);
        }
        catch (std::runtime_error &E)
        {
            rtlFail(0, E.what());
        }
    }
private:
    RInside &R;
    RInside::Proxy result;
    std::string func;
    CriticalBlock block;
};

class REmbedContext: public CInterfaceOf<IEmbedContext>
{
public:
    virtual IEmbedFunctionContext *createFunctionContext(bool isImport, const char *options)
    {
        return new REmbedFunctionContext(*queryGlobalState()->R, options);
    }
};

extern IEmbedContext* getEmbedContext()
{
    return new REmbedContext;
}

extern bool syntaxCheck(const char *script)
{
    return true; // MORE
}

} // namespace
