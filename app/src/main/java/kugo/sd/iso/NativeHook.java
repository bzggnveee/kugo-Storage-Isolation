package kugo.sd.iso;

public class NativeHook {
    static {
        System.loadLibrary("native_hook");
    }

    public static native void initialize(String newPath);
}