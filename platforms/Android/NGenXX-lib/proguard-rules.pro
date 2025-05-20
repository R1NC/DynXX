-keep class kotlin.Function
-keep class kotlin.jvm.internal.Lambda { *; }
-keepclassmembers class * extends kotlin.jvm.internal.Lambda { *; }
-keep class kotlin.jvm.functions.Function1
-keepclassmembers class kotlin.jvm.functions.Function1 {
    *;
}
-keep class kotlin.jvm.functions.Function2
-keepclassmembers class kotlin.jvm.functions.Function2 {
    *;
}
-keep class kotlin.jvm.functions.Function3
-keepclassmembers class kotlin.jvm.functions.Function3 {
    *;
}