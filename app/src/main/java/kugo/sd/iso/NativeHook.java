package kugo.sd.iso;

public class NativeHook {
    
    static {
        try {
            System.loadLibrary("native_hook");
        } catch (UnsatisfiedLinkError e) {
            // 忽略加载失败
        }
    }

    public static native void initialize(String newPath);
    public static native String redirectPath(String path);
}