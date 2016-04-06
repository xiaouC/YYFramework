#include "platform/SDKCallback.h"

static PlatformCallback g_platform_callback;

// exposed to lua
void register_platform_callback( const char* szCallbackType, int nCallbackHandler )
{
    std::map<std::string,int>::iterator iter = g_platform_callback.m_mapCallbackHandlers.find( szCallbackType );
    if( iter != g_platform_callback.m_mapCallbackHandlers.end() )
        CCLuaEngine::defaultEngine()->getLuaStack()->removeScriptHandler( iter->second );

    g_platform_callback.m_mapCallbackHandlers[szCallbackType] = nCallbackHandler;
}

// called by java
void platform_callback( const char* szCallbackType, const char* arg )
{
    PlatformCallbackMsg* msg = new PlatformCallbackMsg();
    msg->m_strCallbackType = szCallbackType;
    msg->arg = arg;

	CAutoLock autoLock( g_platform_callback.lock );
    g_platform_callback.msg_queue.push( msg );
}

// run in cocos2d-x render thread
void process_sdk_callback() {
	CAutoLock autoLock( g_platform_callback.lock );

    while( !g_platform_callback.msg_queue.empty() ) {
        PlatformCallbackMsg* msg = g_platform_callback.msg_queue.front();
		g_platform_callback.msg_queue.pop();

        std::map<std::string,int>::iterator iter = g_platform_callback.m_mapCallbackHandlers.find( msg->m_strCallbackType );
        if( iter != g_platform_callback.m_mapCallbackHandlers.end() )
        {
            CCLuaEngine::defaultEngine()->getLuaStack()->pushString( msg->arg.c_str() );
            CCLuaEngine::defaultEngine()->getLuaStack()->executeFunctionByHandler( iter->second, 1 );
        }

        delete msg;
    }
}

PlatformCallback::PlatformCallback() {
    LockInit(lock);
}

PlatformCallback::~PlatformCallback() {
    std::map<std::string,int>::iterator iter = m_mapCallbackHandlers.begin();
    std::map<std::string,int>::iterator iter_end = m_mapCallbackHandlers.end();
    for( ; iter != iter_end; ++iter )
        CCLuaEngine::defaultEngine()->getLuaStack()->removeScriptHandler( iter->second );
    m_mapCallbackHandlers.clear();

    LockRelease(lock);
}
