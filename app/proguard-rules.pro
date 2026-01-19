-keep class kugo.sd.iso.** { *; }
-keep class de.robv.android.xposed.** { *; }
-keepclasseswithmembers class * {
    native <methods>;
}