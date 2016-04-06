/****************************************************************************
  Copyright (c) 2010-2012 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/
package org.yy;

import android.os.Handler;
import android.os.Message;


import org.cocos2dx.lib.Cocos2dxActivity;

import org.json.JSONObject;
import org.json.JSONException;
import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.net.ConnectivityManager;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.os.Build;
import android.content.Context;
import android.telephony.TelephonyManager;
import android.os.PowerManager;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.drawable.ColorDrawable;

import java.io.UnsupportedEncodingException;
import android.content.ActivityNotFoundException;

import java.util.Map;
import java.util.HashMap;
import java.util.Calendar;
import java.util.Date;
import java.lang.Integer;
import java.text.SimpleDateFormat;

import android.app.PendingIntent;

import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.ByteArrayOutputStream;
import java.util.Iterator;
import java.io.File;

import android.net.wifi.WifiManager;
import android.net.wifi.WifiInfo;
import android.net.NetworkInfo;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;

import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.text.format.Formatter;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.AlarmManager;
import android.text.format.Time;

public class poem extends Cocos2dxActivity {
    static {
        System.loadLibrary("game");
    }

    private static String deviceId;
    private static PowerManager.WakeLock wakeLock;
    private static PowerManager powerManager;
    private static boolean lockAcquired = false;

    private static int versionCode = 0;

    public static Context ctx = null;
    public static poem app;

    private ConnectionChangeReceiver connChangeReceiver;

    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);

        GameProxy.getInstance().onCreate(this);

        ctx = this;
        app = this;
        // get the packageName,it's used to set the resource path
        String packageName = getApplication().getPackageName();
        //super.setPackageName(packageName);

        try
        {
            PackageInfo pkgManager = getPackageManager().getPackageInfo(packageName, 0);
            versionCode = pkgManager.versionCode;
        }catch (NameNotFoundException e) {
            ;
        }

        powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        //wakeLock = powerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK, "poem");
        wakeLock = powerManager.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE, "poem");

        TelephonyManager telephonyManager = (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
        deviceId = telephonyManager.getDeviceId();
        if (deviceId==null || deviceId=="")
            deviceId = genPseudoId();
        if (deviceId==null)
            deviceId = "";

        // 监听网络改变
        IntentFilter filter = new IntentFilter( ConnectivityManager.CONNECTIVITY_ACTION );
        connChangeReceiver = new ConnectionChangeReceiver();
        this.registerReceiver( connChangeReceiver, filter );

        //
        Intent intent = new Intent( this, PushService.class );
        startService( intent );
    }

    public static void quitConfirm(){
        if (app != null) {
            app.runOnUiThread(new Runnable() {
                public void run() {
                    GameProxy.getInstance().exit( app );
                }
            });
        }
    }

    public static void quitApplication()
    {
        if (app != null) {
            app.finish();
            //GameProxy.getInstance().onDestroy(app);
            android.os.Process.killProcess(android.os.Process.myPid());
        }
    }

    public void setImmersionMode(){
        if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT ) {
            View decorView = getWindow().getDecorView();
            int uiFlags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                //| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
                | View.SYSTEM_UI_FLAG_FULLSCREEN; // hide status bar

            if( android.os.Build.VERSION.SDK_INT >= 19 ){
                uiFlags |= 0x00001000;    //SYSTEM_UI_FLAG_IMMERSIVE_STICKY: hide navigation bars - compatibility: building API level is lower thatn 19, use magic number directly for higher API target level
            } else {
                uiFlags |= View.SYSTEM_UI_FLAG_LOW_PROFILE;
            }

            decorView.setSystemUiVisibility( uiFlags );
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        setImmersionMode();
        return true;
    }

    public static int getVersionCode()
    {
        return versionCode;
    }

    public static String getMetaData(String name)
    {
        try {
            ApplicationInfo ai = ctx.getPackageManager().getApplicationInfo(ctx.getPackageName(),PackageManager.GET_META_DATA);
            if (ai.metaData == null) {
                return "";
            }

            String result = null;
            int n = ai.metaData.getInt(name);
            if (n==0) {
                result = ai.metaData.getString(name);
            }
            else {
                result = Integer.toString(n);
            }

            if (result == null) {
                return "";
            }

            return result;
        } catch(NameNotFoundException exc) {
            return "";
        }
    }

    public static void openURL(String url) {
        if ( app == null )
        {
            return;
        }

        Intent i = new Intent(Intent.ACTION_VIEW);
        i.setData(Uri.parse(url));
        app.startActivity(i);
    }

    public static String getDeviceId() {
        return deviceId;
    }

    public static String genPseudoId()
    {
        return "" + //we make this look like a valid IMEI
            Build.BOARD.length()%10+ Build.BRAND.length()%10 +
            Build.CPU_ABI.length()%10 + Build.DEVICE.length()%10 +
            Build.DISPLAY.length()%10 + Build.HOST.length()%10 +
            Build.ID.length()%10 + Build.MANUFACTURER.length()%10 +
            Build.MODEL.length()%10 + Build.PRODUCT.length()%10 +
            Build.TAGS.length()%10 + Build.TYPE.length()%10 +
            Build.USER.length()%10 ; //13 digits
    }

    public static boolean acquireWakeLock() {
        if( lockAcquired )
            return true;
        if( wakeLock != null ) {
            try {
                wakeLock.acquire();
                lockAcquired = true;
            }
            catch( SecurityException e )
            {
                Log.e( "cocos2d", "can't acquire wake lock:" + e.toString() );
            }

            return true;
        }
        return false;
    }

    public static boolean releaseWakeLock() {
        if( wakeLock != null ) {
            try {
                wakeLock.setReferenceCounted(false);
                wakeLock.release();
                lockAcquired = false;
            }
            catch( SecurityException e )
            {
                Log.e( "cocos2d", "can't release wake lock:" + e.toString() );
            }
            return true;
        }
        return false;
    }

    public static void accountLogin(final String arg) {
        if (app != null) {
            app.runOnUiThread(new Runnable() {
                public void run() {
                    GameProxy.getInstance().login(app, arg);
                }
            });
        }
    }

    public static void accountLogout() {
        if (app != null) {
            app.runOnUiThread(new Runnable() {
                public void run() {
                    GameProxy.getInstance().logout(app, null);
                }
            });
        }
    }

    public static void accountSwitch() {
        if (app != null) {
            app.runOnUiThread(new Runnable() {
                public void run() {
                    GameProxy.getInstance().switchAccount(app, null);
                }
            });
        }
    }

    public static void submitExtendData(final String json) {
        Log.v("sdk", json);
        if (app != null)
            app.runOnUiThread(new Runnable() {
                public void run() {
                    GameProxy.getInstance().setExtData(app, json);
                }
            });
    }

    public static void pay(final String goodID, final String goodName, final String orderID, final float goodPrice, final String callBackInfo, final String roleInfo)
    {
        if (app != null) {
            app.runOnUiThread(new Runnable() {
                public void run() {
                    JSONObject b = null;
                    try {
                        GameProxy.getInstance().pay(
                            app,
                            goodID,                          // 商品名
                            goodName,                        // 商品名
                            orderID,
                            goodPrice,     // 商品价格
                            callBackInfo,
                            new JSONObject(roleInfo)
                        );
                    } catch(JSONException e) {
                        Log.e("poem", "roleInfo json parse failed: " + roleInfo);
                        return;
                    }
                }
            });
        }
    }

    public static void playVideo( String url ) {
        if( app == null )
            return;

        try {
            Intent i = new Intent( app, VideoActivity.class );
            i.putExtra( "video_path", url );
            app.startActivity( i );
        } catch ( ActivityNotFoundException e ) {
            e.printStackTrace();
        }
    }

    public static void createLocalNotification(String nDaysLater, String nHour, String nMinute, String nSecond, String szContent, String key, String nType ) {
        //Time t = new Time();
        //t.setToNow();
        //int nYear = t.year;
        //int nMonth = t.month;
        //int nDate = t.monthDay;

        //Calendar c=Calendar.getInstance();
        //c.set( nYear, nMonth, nDate, 0, 0, 30 );

        //Intent intent = new Intent( "cn.pocketdigi.update.alarm" );
        //intent.setClass( app, AlarmReceiver.class );

        //PendingIntent pi=PendingIntent.getBroadcast( app, 0, intent, 0 );
        ////设置一个PendingIntent对象，发送广播
        //AlarmManager am = (AlarmManager)app.getSystemService( ALARM_SERVICE );
        ////获取AlarmManager对象
        ////am.set( AlarmManager.RTC_WAKEUP, c.getTimeInMillis(), pi );
        //am.setRepeating( AlarmManager.RTC_WAKEUP, c.getTimeInMillis(), 10000, pi );
        //
        ////时间到时，执行PendingIntent，只执行一次
        ////AlarmManager.RTC_WAKEUP休眠时会运行，如果是AlarmManager.RTC,在休眠时不会运行
        ////am.setRepeating(AlarmManager.RTC_WAKEUP, c.getTimeInMillis(), 10000, pi);
        ////如果需要重复执行，使用上面一行的setRepeating方法，倒数第二参数为间隔时间,单位为毫秒





        ////// 定义Notication的各种属性
        ////long when = System.currentTimeMillis();
        ////Notification noti = new Notification( R.drawable.ic_launcher, "神奇小精灵", when + 10000 );
        ////noti.flags = Notification.FLAG_INSISTENT;

        //// 创建一个通知
        //Notification mNotification = new Notification();

        //// 设置属性值
        //mNotification.icon = R.drawable.ic_launcher;
        //mNotification.tickerText = "NotificationTest";
        //mNotification.when = System.currentTimeMillis() + 20000; // 立即发生此通知

        //// 带参数的构造函数,属性值如上
        //// Notification mNotification = = new Notification(R.drawable.icon,"NotificationTest", System.currentTimeMillis()));

        //// 添加声音效果
        //mNotification.defaults |= Notification.DEFAULT_SOUND;

        //// 添加震动,后来得知需要添加震动权限 : Virbate Permission
        ////mNotification.defaults |= Notification.DEFAULT_VIBRATE ;

        ////添加状态标志

        ////FLAG_AUTO_CANCEL          该通知能被状态栏的清除按钮给清除掉
        ////FLAG_NO_CLEAR             该通知能被状态栏的清除按钮给清除掉
        ////FLAG_ONGOING_EVENT      通知放置在正在运行
        ////FLAG_INSISTENT             通知的音乐效果一直播放
        //mNotification.flags = Notification.FLAG_INSISTENT ;

        ////将该通知显示为默认View
        //PendingIntent contentIntent = PendingIntent.getActivity( app, 0, new Intent( "android.settings.SETTINGS" ), 0 );
        //mNotification.setLatestEventInfo( app, "通知类型：默认View", "一般般哟。。。。", contentIntent );

        //// 设置setLatestEventInfo方法,如果不设置会App报错异常
        //NotificationManager mNotificationManager = (NotificationManager)app.getSystemService( Context.NOTIFICATION_SERVICE );

        ////注册此通知
        //// 如果该NOTIFICATION_ID的通知已存在，会显示最新通知的相关信息 ，比如tickerText 等
        //mNotificationManager.notify( 2, mNotification );

        Message msg = new Message();
        msg.what = PushService.ADD_NOTIFICATION;
        Bundle b = msg.getData();
        b.putString( "nDaysLater", nDaysLater );
        b.putString( "nHour", nHour );
        b.putString( "nMinute", nMinute );
        b.putString( "nSecond", nSecond );
        b.putString( "szContent", szContent );
        b.putString( "key", key );
        b.putString( "nType", nType );

        PushService.notification_handler.sendMessage( msg );
    }

    public static void releaseLocalNotification(String key) {
        Message msg = new Message();
        msg.what = PushService.REMOVE_NOTIFICATION;
        Bundle b = msg.getData();
        b.putString("key", key);

        PushService.notification_handler.sendMessage( msg );
    }

    public static void releaseAllLocalNotification() {
        Message msg = new Message();
        msg.what = PushService.REMOVE_ALL_NOTIFICATION;

        PushService.notification_handler.sendMessage( msg );
    }

    public static String getLocalMacAddress()
    {
        String str = "";
        try{
            WifiManager wifi = (WifiManager) app.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            WifiInfo info = wifi.getConnectionInfo();
            str = info.getMacAddress();
        }catch(Exception e){
            //Log.v("getLocalMacAddress", "Exception e: " + e);
        }

        return str;
    }

    public static String querySDKFeature() {
        try {
            JSONObject result = new JSONObject();
            result.put("login", GameProxy.getInstance().supportLogin());
            result.put("community", GameProxy.getInstance().supportCommunity());
            result.put("pay", GameProxy.getInstance().supportPay());
            result.put("logout", GameProxy.getInstance().supportLogout());
            return result.toString();
        } catch (JSONException e) {
            return "{}";
        }
    }

    public static void enterPlatform() {
        if (app != null) {
            app.runOnUiThread(new Runnable() {
                public void run() {
                    GameProxy.getInstance().openCommunity(app);
                }
            });
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        GameProxy.getInstance().onStart(this);
    }

    @Override
    protected void onStop() {
        GameProxy.getInstance().onStop(this);
        super.onStop();
    }

    @Override
    public void finish() {
        GameProxy.getInstance().finish(this);
        super.finish();
    }

    @Override
    protected void onDestroy() {
        GameProxy.getInstance().onDestroy(this);

        try {
            unregisterReceiver(connChangeReceiver);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        releaseWakeLock();

        super.onDestroy();
    }

    @Override
	public void onAttachedToWindow() {
		super.onAttachedToWindow();
        GameProxy.getInstance().onAttachedToWindow(this);
    }

    @Override
	public void onDetachedFromWindow() {
		super.onDetachedFromWindow();
        GameProxy.getInstance().onDetachedFromWindow(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        setImmersionMode();
        GameProxy.getInstance().onResume(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        GameProxy.getInstance().onPause(this);
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        GameProxy.getInstance().onRestart(this);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        GameProxy.getInstance().onNewIntent(this, intent);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        GameProxy.getInstance().onActivityResult(this, requestCode, resultCode, data);
    }

    public static String getAvailMemory()
    {
        ActivityManager am = (ActivityManager)app.getApplicationContext().getSystemService( Context.ACTIVITY_SERVICE );
        MemoryInfo mi = new MemoryInfo();
        am.getMemoryInfo( mi );
        Float f = Float.parseFloat( String.valueOf( mi.availMem ) ) / 1024.0f / 1024.0f;
        //return Formatter.formatFileSize( app.getBaseContext(), mi.availMem );
        //return Long.toString( mi.availMem );
        return Float.toString( f );
    }

    public static String getNetworkType()
    {
        ConnectivityManager connectMgr = (ConnectivityManager)app.getApplicationContext().getSystemService( Context.CONNECTIVITY_SERVICE );
        NetworkInfo info = connectMgr.getActiveNetworkInfo();
        if( info == null )
            return "";

        // WIFI
        if( info.getType() == ConnectivityManager.TYPE_WIFI )
        {
            WifiManager wifi = (WifiManager)app.getApplicationContext().getSystemService( Context.WIFI_SERVICE );
            WifiInfo wifi_info = wifi.getConnectionInfo();
            String ssid = wifi_info.getSSID();

            return "WIFI-" + ssid;
        }

        // MOBILE
        int nSubType = info.getSubtype();
        return "MOBILE-" + nSubType;
    }
}

