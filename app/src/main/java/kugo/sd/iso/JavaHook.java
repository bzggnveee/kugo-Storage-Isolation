package kugo.sd.iso;

import android.os.Environment;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.RandomAccessFile;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;

public class JavaHook {
    
    private static final String[] OLD_PATHS = {
        "/storage/emulated/0",
        "/sdcard"
    };
    
    // 豁免路径 - 不重定向这些路径
    private static final String[] EXEMPT_PATHS = {
        "/storage/emulated/0/kgmusic",
        "/sdcard/kgmusic",
        "/storage/emulated/0/Android/data/com.kugou.android.lite",
        "/sdcard/Android/data/com.kugou.android.lite"
    };
    
    private static String newPath;

    public static void init(String target) {
        newPath = target;
        
        hookFileConstructors();
        hookFileStreams();
        hookRandomAccessFile();
        hookEnvironment();
    }

    private static void hookFileConstructors() {
        XposedHelpers.findAndHookConstructor(
            File.class,
            String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    String path = (String) param.args[0];
                    if (shouldRedirect(path)) {
                        param.args[0] = redirectPath(path);
                    }
                }
            }
        );

        XposedHelpers.findAndHookConstructor(
            File.class,
            String.class, String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    String parent = (String) param.args[0];
                    if (shouldRedirect(parent)) {
                        param.args[0] = redirectPath(parent);
                    }
                }
            }
        );

        XposedHelpers.findAndHookConstructor(
            File.class,
            File.class, String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    File parent = (File) param.args[0];
                    if (parent != null) {
                        String path = parent.getAbsolutePath();
                        if (shouldRedirect(path)) {
                            param.args[0] = new File(redirectPath(path));
                        }
                    }
                }
            }
        );
    }

    private static void hookFileStreams() {
        XposedHelpers.findAndHookConstructor(
            FileOutputStream.class,
            String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    String path = (String) param.args[0];
                    if (shouldRedirect(path)) {
                        param.args[0] = redirectPath(path);
                    }
                }
            }
        );

        XposedHelpers.findAndHookConstructor(
            FileOutputStream.class,
            String.class, boolean.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    String path = (String) param.args[0];
                    if (shouldRedirect(path)) {
                        param.args[0] = redirectPath(path);
                    }
                }
            }
        );

        XposedHelpers.findAndHookConstructor(
            FileInputStream.class,
            String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    String path = (String) param.args[0];
                    if (shouldRedirect(path)) {
                        param.args[0] = redirectPath(path);
                    }
                }
            }
        );
    }

    private static void hookRandomAccessFile() {
        XposedHelpers.findAndHookConstructor(
            RandomAccessFile.class,
            String.class, String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    String path = (String) param.args[0];
                    if (shouldRedirect(path)) {
                        param.args[0] = redirectPath(path);
                    }
                }
            }
        );

        XposedHelpers.findAndHookConstructor(
            RandomAccessFile.class,
            File.class, String.class,
            new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    File file = (File) param.args[0];
                    if (file != null) {
                        String path = file.getAbsolutePath();
                        if (shouldRedirect(path)) {
                            param.args[0] = new File(redirectPath(path));
                        }
                    }
                }
            }
        );
    }

    private static void hookEnvironment() {
        XposedHelpers.findAndHookMethod(
            Environment.class,
            "getExternalStorageDirectory",
            new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                    if (newPath != null) {
                        param.setResult(new File(newPath));
                    }
                }
            }
        );

        XposedHelpers.findAndHookMethod(
            Environment.class,
            "getExternalStoragePublicDirectory",
            String.class,
            new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                    if (newPath != null) {
                        String type = (String) param.args[0];
                        param.setResult(new File(newPath, type));
                    }
                }
            }
        );
    }

    private static boolean isExempt(String path) {
        if (path == null) return false;
        
        for (String exempt : EXEMPT_PATHS) {
            if (path.startsWith(exempt)) {
                return true;
            }
        }
        return false;
    }

    private static boolean shouldRedirect(String path) {
        if (path == null || newPath == null) {
            return false;
        }
        
        // 先检查是否豁免
        if (isExempt(path)) {
            return false;
        }
        
        // 检查是否匹配需要重定向的路径
        for (String oldPath : OLD_PATHS) {
            if (path.startsWith(oldPath)) {
                return true;
            }
        }
        
        return false;
    }

    private static String redirectPath(String path) {
        if (path == null || newPath == null) {
            return path;
        }
        
        String redirected = path;
        
        // 替换所有可能的旧路径
        for (String oldPath : OLD_PATHS) {
            if (path.startsWith(oldPath)) {
                redirected = newPath + path.substring(oldPath.length());
                break;
            }
        }
        
        // 确保父目录存在
        File file = new File(redirected);
        File parent = file.getParentFile();
        if (parent != null && !parent.exists()) {
            parent.mkdirs();
        }
        
        return redirected;
    }
}