#ifndef __SDK_CALLBACK_H__
#define __SDK_CALLBACK_H__

#include <queue>
#include <map>
#include "Common/TLCommon.h"

// 注意注意，请不要在中间插入，影响原来的值。
class PlatformCallbackMsg
{
public:
    std::string m_strCallbackType;
    std::string arg;
};

class PlatformCallback
{
public:
    PlatformCallback();
    ~PlatformCallback();

    std::map<std::string,int> m_mapCallbackHandlers;

    XL_LOCK lock;
    std::queue<PlatformCallbackMsg*> msg_queue;
};

void register_platform_callback( const char* szCallbackType, int nCallbackHandler );
void platform_callback( const char* szCallbackType, const char* args );
void process_sdk_callback();

#endif
