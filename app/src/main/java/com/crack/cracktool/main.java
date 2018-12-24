package com.crack.cracktool;

import android.annotation.SuppressLint;
import android.os.Environment;
import android.util.Log;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;

/**
 * 代码只是为了演示入门方法，在Android 4.4.4系统通过测试。
 * 实际存在一些已知问题，比如，因为权限控制第37行代码在高版本的Android系统会执行失败。
 */
@SuppressWarnings("SpellCheckingInspection")
@SuppressLint("UnsafeDynamicallyLoadedCode")
public class main implements IXposedHookLoadPackage {
    private static final String TAG = "Q296488320";
    private static final String DUMP_LUA_SO = Environment.getDataDirectory()
            + "/data/com.crack.cracktool/lib/libdumpLua.so";

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpp) throws Throwable {


        //dump lua
        if (!lpp.packageName.equals("com.mmcy.byjhz"))
            return;
        Log.i(TAG, lpp.packageName);
        findAndHookMethod(Runtime.class, "doLoad", String.class, ClassLoader.class, new XC_MethodHook() {
            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                String name = (String) param.args[0];
                Log.i(TAG, "Load so file: " + name);

                //Hook 要加载的 so
                //当 加载 的 so 是 要 加载的 就加载自己的
                if (param.hasThrowable() || name == null || !name.endsWith("libcocos2dlua.so")) {
                    return;
                }
                Log.i(TAG, "---------------dumping--------------");
                System.load(DUMP_LUA_SO);// 因为权限控制，在高版本的Android系统会执行失败


            }
        });




        // TODO: 2018/12/11
//        if(lpp.packageName.contains("mono")){
//            Log.e("发现mono","libmono.so");
//        }


    }
}
