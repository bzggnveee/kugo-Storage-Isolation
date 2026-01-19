package kugo.sd.iso;

import android.app.Application;
import android.content.Context;

import java.io.File;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class MainHook implements IXposedHookLoadPackage {
    
    private static final String TARGET_PACKAGE = "com.kugou.android.lite";
    private static String newPath = null;
    private static boolean initialized = false;

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if (!lpparam.packageName.equals(TARGET_PACKAGE)) {
            return;
        }

        XposedHelpers.findAndHookMethod(
            Application.class,
            "attach",
            Context.class,
            new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                    if (initialized) return;
                    
                    Context context = (Context) param.args[0];
                    File externalFilesDir = context.getExternalFilesDir(null);
                    if (externalFilesDir != null) {
                        newPath = externalFilesDir.getAbsolutePath() + "/sdcard";
                        new File(newPath).mkdirs();
                        
                        JavaHook.init(newPath);
                        
                        try {
                            System.loadLibrary("native_hook");
                            NativeHook.initialize(newPath);
                            initialized = true;
                        } catch (Exception ignored) {}
                    }
                }
            }
        );
    }
}