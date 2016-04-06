package org.yy;

import java.lang.Class;
import java.lang.reflect.Constructor;
import org.json.JSONObject;
import org.json.JSONException;

import android.app.Activity;
import android.content.Intent;
import android.content.Context;
import android.util.Log;

public class GameProxy {
    public static GameProxy instance = null;

    public interface onExitListener {
        public void onExit();
    }

    public static GameProxy getInstance() {
        if (instance == null) {
            try {
                Class klass = Class.forName("org.yy.GameProxyImpl");
                Constructor ctor = klass.getConstructor();
                instance = (GameProxy)ctor.newInstance();
            } catch (Exception e) {
                Log.e("poem", "org.yy.GameProxyImpl not found");
            }
        }
        return instance;
    }

    public void login(Activity activity,Object customParams) {
        // 登录，customParams透传给回调
    }

    public void logout(Activity activity,Object customParams) {
        onLogout();
    }

    public void switchAccount(Activity activity,Object customParams) {
        // 切换帐号（目前没用到），customParams透传给回调
    }

    public void pay( Activity activity, String ID, String name, String orderID, float price, String callBackInfo, JSONObject roleInfo ) {
        // 支付 ID：商品ID，name：商品名，orderID：CP订单号，price：金额（单位元），callBackInfo：需要透传给服务器回调，roleInfo：角色信息json，payCallBack：支付回调
        /*
         * local roleInfo = {
         *  id = g_player.entityID,
         *  name = g_player.name,
         *  faction = '',
         *  vip = g_player.vip,
         *  level = g_player.level,
         *  serverID = self.server_id,
         *  raw_username = g_sdk_username,
         *}
         */
    }

    public void openCommunity(Activity activity) {
        // 打开社区
    }

    public void exit( final Activity activity ) {
        // 这里默认使用的是游戏的退出窗口
        // 如果退出窗口由渠道提供，游戏只须实现资源回收，杀死进程等退出逻辑
        onExitListener exit_listener = new onExitListener() {
            public void onExit() {
                activity.finish();
                android.os.Process.killProcess( android.os.Process.myPid() );
            }
        };

        if( !on3rdExit( activity, exit_listener ) ) {
            sdk.platformCallback( "SDK_EXIT", "" );
        }
    }

    public boolean on3rdExit( Activity activity, onExitListener exit_listener ) {
        return false;
    }

    public void setExtData(Context context, String ext) {
        // 上报角色数据给sdk，ext:json数据，格式如下
        /* local info = {
         *     state = send_type,                   -- type
         *     id = roleId,                      -- roleId
         *     name = roleName,                    -- roleName
         *     level = roleLevel,                   -- roleLevel
         *     serverID = zoneId,                      -- zoneId
         *     serverName = zoneName,                    -- zoneName
         *     gold = balance,                     -- balance
         *     vip = vip,                         -- vip
         *     factionName = partyName                    -- partyName
         * }
         */
    }

    public boolean supportLogin() {
        // 是否支持sdk登录
        return true;
    }

    public boolean supportCommunity() {
        // 是否支持打开社区
        return true;
    }

    public boolean supportPay() {
        // 是否支持支付
        return true;
    }

    public boolean supportLogout() {
        // 是否支持登出（显示登出按钮）
        return false;
    }

    public void onCreate(Activity activity) {
        // 以下为activity生命周期，有些sdk会要求在里面加入调用。
    }

    public void onStart(Activity activity) {
    }

    public void onStop(Activity activity) {
    }

    public void onDestroy(Activity activity) {
    }

    public void onResume(Activity activity) {
    }

    public void onPause(Activity activity) {
    }

    public void onRestart(Activity activity) {
    }

    public void onNewIntent(Activity activity, Intent intent) {
    }

    public void onActivityResult(Activity activity, int requestCode, int resultCode, Intent data) {
    }

    public void onAttachedToWindow(Activity activity) {
    }

    public void onDetachedFromWindow(Activity activity) {
    }

    public void finish(Activity activity) {
    }

    // 
    public void onLoginFailed() {
        sdk.platformCallback( "SDK_LOGIN_FAILED", "" );
    }

    public void onLoginSuccess( String userName, String userID, String token, String channelID ) {
        try {
            JSONObject json = new JSONObject();
            json.put( "username", userName );
            json.put( "userID", userID );
            json.put( "token", token );
            json.put( "channelID", channelID );
            sdk.platformCallback( "SDK_LOGIN_SUCCESS", json.toString() );
        } catch ( JSONException e ) {
            Log.v( "poem", "onloginSuccess, JSONError" );
        }
    }

    public void onLogout() {
        sdk.platformCallback( "SDK_LOGOUT", "" );
    }

    public void onPaySuccess() {
        sdk.platformCallback( "SDK_PAY_SUCCESS", "" );
    }

    public void onPayFail() {
        sdk.platformCallback( "SDK_PAY_FAIL", "" );
    }
}
