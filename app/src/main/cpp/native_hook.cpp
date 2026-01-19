include <jni.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "dobby.h"

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

// Hook 函数定义
typedef int (*open_func_t)(const char*, int, ...);
static open_func_t original_open = nullptr;

static int hooked_open(const char* pathname, int flags, ...) {
    std::string redirected = redirect_path(pathname);
    
    if (flags & O_CREAT) {
        ensure_parent_dir(redirected);
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return original_open(redirected.c_str(), flags, mode);
    }
    
    return original_open(redirected.c_str(), flags);
}

typedef int (*open64_func_t)(const char*, int, ...);
static open64_func_t original_open64 = nullptr;

static int hooked_open64(const char* pathname, int flags, ...) {
    std::string redirected = redirect_path(pathname);
    
    if (flags & O_CREAT) {
        ensure_parent_dir(redirected);
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return original_open64(redirected.c_str(), flags, mode);
    }
    
    return original_open64(redirected.c_str(), flags);
}

typedef FILE* (*fopen_func_t)(const char*, const char*);
static fopen_func_t original_fopen = nullptr;

static FILE* hooked_fopen(const char* pathname, const char* mode) {
    std::string redirected = redirect_path(pathname);
    
    if (strchr(mode, 'w') || strchr(mode, 'a')) {
        ensure_parent_dir(redirected);
    }
    
    return original_fopen(redirected.c_str(), mode);
}

typedef FILE* (*fopen64_func_t)(const char*, const char*);
static fopen64_func_t original_fopen64 = nullptr;

static FILE* hooked_fopen64(const char* pathname, const char* mode) {
    std::string redirected = redirect_path(pathname);
    
    if (strchr(mode, 'w') || strchr(mode, 'a')) {
        ensure_parent_dir(redirected);
    }
    
    return original_fopen64(redirected.c_str(), mode);
}

typedef int (*access_func_t)(const char*, int);
static access_func_t original_access = nullptr;

static int hooked_access(const char* pathname, int mode) {
    std::string redirected = redirect_path(pathname);
    return original_access(redirected.c_str(), mode);
}

typedef int (*stat_func_t)(const char*, struct stat*);
static stat_func_t original_stat = nullptr;

static int hooked_stat(const char* pathname, struct stat* buf) {
    std::string redirected = redirect_path(pathname);
    return original_stat(redirected.c_str(), buf);
}

typedef int (*stat64_func_t)(const char*, struct stat64*);
static stat64_func_t original_stat64 = nullptr;

static int hooked_stat64(const char* pathname, struct stat64* buf) {
    std::string redirected = redirect_path(pathname);
    return original_stat64(redirected.c_str(), buf);
}

typedef int (*lstat_func_t)(const char*, struct stat*);
static lstat_func_t original_lstat = nullptr;

static int hooked_lstat(const char* pathname, struct stat* buf) {
    std::string redirected = redirect_path(pathname);
    return original_lstat(redirected.c_str(), buf);
}

typedef int (*lstat64_func_t)(const char*, struct stat64*);
static lstat64_func_t original_lstat64 = nullptr;

static int hooked_lstat64(const char* pathname, struct stat64* buf) {
    std::string redirected = redirect_path(pathname);
    return original_lstat64(redirected.c_str(), buf);
}

typedef int (*mkdir_func_t)(const char*, mode_t);
static mkdir_func_t original_mkdir = nullptr;

static int hooked_mkdir(const char* pathname, mode_t mode) {
    std::string redirected = redirect_path(pathname);
    ensure_parent_dir(redirected);
    return original_mkdir(redirected.c_str(), mode);
}

typedef int (*rmdir_func_t)(const char*);
static rmdir_func_t original_rmdir = nullptr;

static int hooked_rmdir(const char* pathname) {
    std::string redirected = redirect_path(pathname);
    return original_rmdir(redirected.c_str());
}

typedef int (*unlink_func_t)(const char*);
static unlink_func_t original_unlink = nullptr;

static int hooked_unlink(const char* pathname) {
    std::string redirected = redirect_path(pathname);
    return original_unlink(redirected.c_str());
}

typedef int (*rename_func_t)(const char*, const char*);
static rename_func_t original_rename = nullptr;

static int hooked_rename(const char* oldpath, const char* newpath) {
    std::string old_redirected = redirect_path(oldpath);
    std::string new_redirected = redirect_path(newpath);
    ensure_parent_dir(new_redirected);
    return original_rename(old_redirected.c_str(), new_redirected.c_str());
}

typedef ssize_t (*readlink_func_t)(const char*, char*, size_t);
static readlink_func_t original_readlink = nullptr;

static ssize_t hooked_readlink(const char* pathname, char* buf, size_t bufsiz) {
    std::string redirected = redirect_path(pathname);
    return original_readlink(redirected.c_str(), buf, bufsiz);
}

typedef int (*openat_func_t)(int, const char*, int, ...);
static openat_func_t original_openat = nullptr;

static int hooked_openat(int dirfd, const char* pathname, int flags, ...) {
    std::string redirected = redirect_path(pathname);
    
    if (flags & O_CREAT) {
        ensure_parent_dir(redirected);
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return original_openat(dirfd, redirected.c_str(), flags, mode);
    }
    
    return original_openat(dirfd, redirected.c_str(), flags);
}

static void install_hooks() {
    void* libc = dlopen("libc.so", RTLD_NOW);
    if (!libc) return;

    #define HOOK_FUNC(name) \
        do { \
            void* func = dlsym(libc, #name); \
            if (func) { \
                DobbyHook(func, (void*)hooked_##name, (void**)&original_##name); \
            } \
        } while(0)

    HOOK_FUNC(open);
    HOOK_FUNC(open64);
    HOOK_FUNC(openat);
    HOOK_FUNC(fopen);
    HOOK_FUNC(fopen64);
    HOOK_FUNC(access);
    HOOK_FUNC(stat);
    HOOK_FUNC(stat64);
    HOOK_FUNC(lstat);
    HOOK_FUNC(lstat64);
    HOOK_FUNC(mkdir);
    HOOK_FUNC(rmdir);
    HOOK_FUNC(unlink);
    HOOK_FUNC(rename);
    HOOK_FUNC(readlink);

    #undef HOOK_FUNC
    
    dlclose(libc);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_storageredirect_NativeHook_initialize(
        JNIEnv* env,
        jclass clazz,
        jstring newPath) {
    
    const char* new_path_str = env->GetStringUTFChars(newPath, nullptr);
    g_new_path = new_path_str;
    env->ReleaseStringUTFChars(newPath, new_path_str);
    
    install_hooks();
}