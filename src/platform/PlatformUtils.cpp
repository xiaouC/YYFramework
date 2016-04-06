#include "PlatformUtils.h"
#include "../Common/TLCommon.h"
#include "CCPlatformConfig.h"
#include "platform/sdk.h"
#include "MC/AssetsManager.h"
#include <string>
#include "UI/TLEditBox.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/SDKCallback.h"
//#include "OurpalmSDK.h"
#include "android/jni/JniHelper.h"
#include "android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"
#include <jni.h>
#include "CCString.h"
#include "../NetMessage/NetWorkThread.h"
#include "../NetMessage/NetMsgProcess.h"
#include "../NetMessage/NetMsg.h"

//framework::SDKType GetSdkType()
//{
//	return framework::SDK_UNKNOWN;
//}
//

framework::SDKType GetSdkType() {
    const char* s = getMetaData("sdktype");
    int n = 0;
    if (s!=NULL and s[0]!='\0') {
        n = atoi(s);
        return (framework::SDKType) n;
    }
    CCLog("ERROR: Not speifiy sdktype in AndroidManifest.xml, return default one");
    return framework::SDK_YY;
}

using namespace cocos2d;

const char* getdeviceUniqueID()
{
	return NULL;
}

const char* getMAC()
{
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "getLocalMacAddress"
                , "()Ljava/lang/String;"))
    {
        jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
        ret->autorelease();
        return ret->m_sString.c_str();
    }
    return NULL;

}

const char* getidfa()
{
	return NULL;
}

const char* getIMEI()
{
	return NULL;
}

const char* getDeviceId()
{
	JniMethodInfo t;
	if (JniHelper::getStaticMethodInfo(t
		, "org/yy/poem"
		, "getDeviceId"
		, "()Ljava/lang/String;"))
	{
		jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
		CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
		ret->autorelease();
		return ret->m_sString.c_str();
	}
	return NULL;
}

const char* getIMSI()
{
    return NULL;
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "getIMSI"
    //            , "()Ljava/lang/String;"))
    //{
    //    jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
    //    ret->autorelease();
    //    return ret->m_sString.c_str();
    //}
    //return NULL;
}

const char* getUUID()
{
    return NULL;
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "getUUID"
    //            , "()Ljava/lang/String;"))
    //{
    //    jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
    //    ret->autorelease();
    //    return ret->m_sString.c_str();
    //}
    //return NULL;
}

bool acquireWakeLock()
{
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "acquireWakeLock"
                , "()Z"))
    {
        bool b = (jboolean)t.env->CallStaticBooleanMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        return b;
    }
    return false;
}

bool releaseWakeLock()
{
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "releaseWakeLock"
                , "()Z"))
    {
        bool b = (jboolean)t.env->CallStaticBooleanMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        return b;
    }
    return false;
}

bool userActivity()
{
    return false;
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "userActivity"
    //            , "()Z"))
    //{
    //    bool b = (jboolean)t.env->CallStaticBooleanMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    return b;
    //}
    //return false;
}

void quitConfirm()
{
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "quitConfirm"
                , "()V"))
    {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}

void quitApplication()
{
	JniMethodInfo t;
	if (JniHelper::getStaticMethodInfo(t
		, "org/yy/poem"
		, "quitApplication"
		, "()V"))
	{
		t.env->CallStaticVoidMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
	}
}

int getPackageVersion()
{
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "getVersionCode"
                , "()I"))
    {
        int ver = t.env->CallStaticIntMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        return ver;
    }
    else
    {
        return 0;
    }
}

std::string getPackageName()
{
    const char* s = getMetaData("YYPackageName");
    std::string ret = "";
    if (s==NULL || s[0]=='\0') {
        ret = getPackageNameJNI();
    }
    else {
        ret = s;
    }
	CCLOG("test getPackageName name=%s", ret.c_str());
    //return ret.c_str();
    //return "xp";
	return ret;
}

const char* getNetworkTypeName()
{
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "getNetworkType"
                , "()Ljava/lang/String;"))
    {
        jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
        ret->autorelease();
        return ret->m_sString.c_str();
    }
    return NULL;
}

void openURL(const char* sUrl)
{
	if (sUrl==NULL)
	{
		return;
	}
	JniMethodInfo t;
	if (JniHelper::getStaticMethodInfo(t
		, "org/yy/poem"
		, "openURL"
		, "(Ljava/lang/String;)V"))
	{
		jstring stringUrl = t.env->NewStringUTF(sUrl);
		t.env->CallStaticVoidMethod(t.classID, t.methodID, stringUrl);
		t.env->DeleteLocalRef(stringUrl);
		t.env->DeleteLocalRef(t.classID);
	}
}

bool isETC1Supported()
{
    return false;
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "isETC1Supported"
    //            , "()Z"))
    //{
    //    bool b = (jboolean)t.env->CallStaticBooleanMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    return b;
    //}
    //return false;
}

const char* getOSVersion()
{
    return NULL;
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "getOSVersion"
    //            , "()Ljava/lang/String;"))
    //{
    //    jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
    //    ret->autorelease();
    //    return ret->m_sString.c_str();
    //}
    //return NULL;
}

const char* getResolution()
{
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "getResolution"
    //            , "()Ljava/lang/String;"))
    //{
    //    jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
    //    ret->autorelease();
    //    return ret->m_sString.c_str();
    //}
    return NULL;
}

const char* getUA()
{
    //JniMethodInfo t;
    //if (JniHelper::getStaticMethodInfo(t
    //            , "org/yy/poem"
    //            , "getUA"
    //            , "()Ljava/lang/String;"))
    //{
    //    jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
    //    t.env->DeleteLocalRef(t.classID);
    //    CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
    //    ret->autorelease();
    //    return ret->m_sString.c_str();
    //}
    return NULL;
}

const char* getMetaData(const char* name)
{
    CCLog("getMetaData %s", name);
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "getMetaData"
                , "(Ljava/lang/String;)Ljava/lang/String;"))
    {
        jstring stringName = t.env->NewStringUTF(name);
        jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID, stringName);
        t.env->DeleteLocalRef(stringName);
        t.env->DeleteLocalRef(t.classID);
        if (str != NULL) {
            CCString *ret = new CCString(JniHelper::jstring2string(str).c_str());
            ret->autorelease();
            return ret->m_sString.c_str();
        }
        else {
            return NULL;
        }
    }
    return NULL;

}
#else

#include "sprite_nodes/CCSprite.h"
#include "MC/MovieClip.h"
#include "UI/TLWindowManager.h"
#include "MC/MCLoader.h"
USING_NS_CC;

bool userActivity()
{
    return true;
}

void quitConfirm()
{
}

bool isETC1Supported()
{
    return false;
}

#if CC_TARGET_PLATFORM != CC_PLATFORM_IOS
void quitApplication()
{
    //system("pause");
    exit( 0 );
}

void openURL(const char* sUrl)
{
}

#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#else

bool acquireWakeLock()
{
    return true;
}

bool releaseWakeLock()
{
    return true;
}

const char* getdeviceUniqueID()
{
	return NULL;
}

const char* getMAC()
{
    return NULL;
}

const char* getidfa()
{
	return NULL;
}

const char* getIMEI()
{
	return NULL;
}

int getPackageVersion()
{
    return 1;
}

std::string getPackageName()
{
    return "pc";
}

const char* getNetworkTypeName()
{
    return "wifi";
}

const char* getDeviceId()
{
    //return "test_robot_240";
    return "test_device_id";
}

const char* getIMSI()
{
    return "PC";
}

const char* getUUID()
{
    char buf[32] = {0};
    XLSprintf(buf, sizeof(buf), "PC_FAKE_UUID_%05d", rand());
    CCString *ret = new CCString(buf);
    ret->autorelease();
    return ret->m_sString.c_str();
}

////static MovieClip* loadingSprite = NULL;
//static CCSprite* loadingSprite = NULL;
//
//void showLoading(const char* sMsg)
//{
//    if (loadingSprite==NULL)
//    {
//        loadingSprite = MCLoader::sharedMCLoader()->loadSprite("60007_07.png");
//        loadingSprite->retain();
//        loadingSprite->setPosition(TLWindowManager::SharedTLWindowManager()->GetCenterPoint());
//    }
//    if (loadingSprite->getParent()==NULL)
//    {
//        TLWindowManager::SharedTLWindowManager()->addChild(loadingSprite, 999);
//    }
//    loadingSprite->stopAllActions();
//    loadingSprite->setVisible(true);
//    loadingSprite->runAction(CCRepeatForever::actionWithAction(CCRotateBy::actionWithDuration(0.5, -360)));
//}
//void hideLoading()
//{
//	if( loadingSprite != NULL )
//	{
//		loadingSprite->stopAllActions();
//		loadingSprite->setVisible(false);
//	}
//}
//bool isLoadingShowing()
//{
//	if( loadingSprite != NULL )
//	    return loadingSprite->getIsVisible();
//
//	return false;
//}

framework::SDKType GetSdkType()
{
	return framework::SDK_YY;
}

const char* getOSVersion()
{
    return "PC";
}

const char* getResolution()
{
    return "PC";
}

const char* getUA()
{
    return "PC";
}

const char* getMetaData(const char* name)
{
    return "";
}
#endif
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
std::string strPlatformPostfix;
static std::string strPlatform;
#endif

const char* getPlatform()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    return "android";
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	if( strPlatformPostfix.empty() )
		return "win32";

	strPlatform = std::string( "win32_" ) + strPlatformPostfix;
	return strPlatform.c_str();
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    return "linux";
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    return "ios";
#endif
}

//////////////////////////////////////////////////////////////////////////
// 只有 lua 的 log 才会被写入文件，不然会存在线程安全问题
FILE* _logFile = NULL;

void MyLog(const char* buf)
{
	if(_logFile)
	{
		fprintf(_logFile, "%s\n", buf);
		fflush(_logFile);
	}
}

void initLog()
{
	std::string logPath = CCFileUtils::sharedFileUtils()->getWritablePath() + "debug.log";
	_logFile = fopen(logPath.c_str(), "w+");

	CCSetLogFunction(MyLog);
	CCLuaLog(logPath.c_str());
}

void finiLog()
{
	if(_logFile)
	{
		fclose(_logFile);
		_logFile = NULL;
	}
}

void useConsole() //  π”√øÿ÷∆Ã®
{
#ifdef WIN32
    if(AllocConsole())
    {
        freopen("CONOUT$","w",stdout);
    }
#endif
}

std::string g_strResourceVersion;
void setResourceVersion( const char* szResVersion )
{
    if( szResVersion != NULL )
        g_strResourceVersion = szResVersion;
}

const char* getResourceVersion()
{
    return g_strResourceVersion.c_str();
}

#if( CC_TARGET_PLATFORM != CC_PLATFORM_IOS )
void createLocalNotification( const char* nDaysLater, const char* nHour, const char* nMinute, const char* nSecond, const char* szContent, const char* key, const char* nType  )
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
    JniMethodInfo t;
    if( JniHelper::getStaticMethodInfo( t
                , "org/yy/poem"
                , "createLocalNotification"
                , "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V" ) )
    {

        jstring j_nDaysLater = t.env->NewStringUTF( nDaysLater );
        jstring j_nHour = t.env->NewStringUTF( nHour );
        jstring j_nMinute = t.env->NewStringUTF( nMinute );
        jstring j_nSecond = t.env->NewStringUTF( nSecond );
        jstring j_szContent = t.env->NewStringUTF( szContent );
        jstring j_key = t.env->NewStringUTF( key );
        jstring j_nType = t.env->NewStringUTF( nType );
        t.env->CallStaticVoidMethod( t.classID, t.methodID, j_nDaysLater, j_nHour, j_nMinute, j_nSecond, j_szContent, j_key, j_nType );

        t.env->DeleteLocalRef(j_nDaysLater);
        t.env->DeleteLocalRef(j_nHour);
        t.env->DeleteLocalRef(j_nMinute);
        t.env->DeleteLocalRef(j_nSecond);
        t.env->DeleteLocalRef(j_szContent);
        t.env->DeleteLocalRef(j_key);
        t.env->DeleteLocalRef(j_nType);
        t.env->DeleteLocalRef( t.classID );
    }

#endif
}
void releaseLocalNotification( const char* key )
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
    JniMethodInfo t;
    if( JniHelper::getStaticMethodInfo( t
                , "org/yy/poem"
                , "releaseLocalNotification"
                , "(Ljava/lang/String;)V" ) )
    {

        jstring j_key = t.env->NewStringUTF( key );
        t.env->CallStaticVoidMethod( t.classID, t.methodID, j_key );

        t.env->DeleteLocalRef(j_key);
        t.env->DeleteLocalRef( t.classID );
    }


#endif
}
void releaseAllLocalNotification()
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
    JniMethodInfo t;
    if( JniHelper::getStaticMethodInfo( t
                , "org/yy/poem"
                , "releaseAllLocalNotification"
                , "()V" ) )
    {

        t.env->CallStaticVoidMethod( t.classID, t.methodID );

        t.env->DeleteLocalRef( t.classID );
    }
#endif
}

void playMedia( const char* szPath )
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
    JniMethodInfo t;
    if( JniHelper::getStaticMethodInfo( t
                , "org/yy/poem"
                , "playVideo"
                , "(Ljava/lang/String;)V" ) )
    {
		jstring string_path = t.env->NewStringUTF( szPath );
		t.env->CallStaticVoidMethod( t.classID, t.methodID, string_path );
		t.env->DeleteLocalRef( string_path );
		t.env->DeleteLocalRef( t.classID );
    }
#endif
}

void stopMedia()
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
    JniMethodInfo t;
    if( JniHelper::getStaticMethodInfo( t
                , "org/yy/poem"
                , "stopMdeia"
                , "()V" ) )
    {
		t.env->CallStaticVoidMethod( t.classID, t.methodID );
		t.env->DeleteLocalRef( t.classID );
    }
#endif
}
#endif

void hideWindow()
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 )
    CCEGLView* pMainWnd = CCEGLView::sharedOpenGLView();
    ShowWindow( pMainWnd->getHWnd(), SW_HIDE );
#endif
}

#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
extern "C"
{
JNIEXPORT jboolean JNICALL Java_org_yy_sdk_sendTalkStr(JNIEnv* env, jobject thiz, jstring talkStr)
{
    TLEditBox *s_TLEditBox = TLEditBox::getCurrentTLEditBox();
    s_TLEditBox->sentNetMsgToSelf(env->GetStringUTFChars(talkStr, 0));
	return true;
}

JNIEXPORT jboolean JNICALL Java_org_yy_sdk_checkTalkStr(JNIEnv* env, jobject thiz, jstring talkStr, jint talkStrLength)
{
    TLEditBox *s_TLEditBox = TLEditBox::getCurrentTLEditBox();
    return s_TLEditBox->checkText(env->GetStringUTFChars(talkStr, 0), talkStrLength);
}

JNIEXPORT jboolean JNICALL Java_org_yy_sdk_platformCallback(JNIEnv* env, jobject thiz, jstring type, jstring args)
{
    platform_callback(env->GetStringUTFChars(type, 0), env->GetStringUTFChars(args, 0));
    return true;
}


}
#endif

#if( CC_TARGET_PLATFORM != CC_PLATFORM_IOS )
const char* getAvailMemory()
{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
    JniMethodInfo t;
    if( JniHelper::getStaticMethodInfo(t
                , "org/yy/poem"
                , "getAvailMemory"
                , "()Ljava/lang/String;"))
    {
        jstring str = (jstring)t.env->CallStaticObjectMethod( t.classID, t.methodID );
        t.env->DeleteLocalRef( t.classID );

        if( str != NULL )
        {
            CCString *ret = new CCString( JniHelper::jstring2string( str ).c_str() );
            ret->autorelease();

            return ret->m_sString.c_str();
        }
    }
#endif

    return "1024";
}
#endif
