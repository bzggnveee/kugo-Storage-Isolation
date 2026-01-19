#include <jni.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dlfcn.h>

static std::string g_new_path;

static const char* OLD_PATHS[] = {
    "/storage/emulated/0",
    "/sdcard"
};

static const char* EXEMPT_PATHS[] = {
    "/storage/emulated/0/kgmusic",
    "/sdcard/kgmusic",
    "/storage/emulated/0/Android/data/com.kugou.android.lite",
    "/sdcard/Android/data/com.kugou.android.lite"
};

static bool is_exempt(const char* path) {
    if (!path) return false;
    for (const char* exempt : EXEMPT_PATHS) {
        if (strncmp(path, exempt, strlen(exempt)) == 0) {
            return true;
        }
    }
    return false;
}

static std::string redirect_path(const char* path) {
    if (!path || g_new_path.empty() || is_exempt(path)) {
        return path;
    }
    
    std::string str_path(path);
    for (const char* old_path : OLD_PATHS) {
        size_t old_len = strlen(old_path);
        if (strncmp(path, old_path, old_len) == 0) {
            return g_new_path + str_path.substr(old_len);
        }
    }
    return path;
}

static void ensure_parent_dir(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        std::string parent = path.substr(0, pos);
        struct stat st;
        if (stat(parent.c_str(), &st) != 0) {
            ensure_parent_dir(parent);
            mkdir(parent.c_str(), 0755);
        }
    }
}

// 不使用 Hook 库，仅提供路径重定向接口
extern "C"
JNIEXPORT void JNICALL
Java_com_example_storageredirect_NativeHook_initialize(
        JNIEnv* env,
        jclass clazz,
        jstring newPath) {
    
    const char* new_path_str = env->GetStringUTFChars(newPath, nullptr);
    g_new_path = new_path_str;
    env->ReleaseStringUTFChars(newPath, new_path_str);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_storageredirect_NativeHook_redirectPath(
        JNIEnv* env,
        jclass clazz,
        jstring path) {
    
    const char* path_str = env->GetStringUTFChars(path, nullptr);
    std::string redirected = redirect_path(path_str);
    env->ReleaseStringUTFChars(path, path_str);
    
    return env->NewStringUTF(redirected.c_str());
}