
#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

#include <stdlib.h>

#include <stdio.h>
#include "include/inlineHook.h"


//#include "mono/metadata/image.h"
//#include "mono/eglib/glib.h"


#define TAG "Q296488320"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);


jboolean isU3d = false;



struct _MonoImage{
    int ref_count;
    void *raw_data_handle;
    char *raw_data;
    int raw_data_len;

}typedef MonoImage;




#define PACKAGE_NAME "com.mmcy.byjhz" // 目标应用的包名
#define TARGET_SO "/data/data/com.mmcy.byjhz/lib/libcocos2dlua.so" // 目标应用的libcocos2dlua.so
//#define TARGET_SO "/data/data/com.avalon.caveonline.cn.leiting/lib/libgame.so"





//#define TARGET_SO "/data/data/com.sqview.arcard/lib/libmono.so"


MonoImage *  (*my_mono_image_init_mod_t)(char *data,
                                 size_t data_len,
                                 int  need_copy,
                                 void *status,
                                 int  refonly, char *name)=NULL;


int (*origin_luaL_loadbuffer)(void *lua_state, char *buff, size_t size, char *name) = NULL;

void free(void *__ptr);


int my_luaL_loadbuffer(void *lua_state, char *buff, size_t size, char *name) {
    LOGD("lua size: %d, name: %s", (uint32_t) size, name);  // 打印lua脚本的大小和名称

    if (name != NULL) {
        //strdup()在内部调用了malloc()为变量分配内存

        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            if (8 < name_len <= 100) {
                char *base_dir = (char *) "/sdcard/hookLua/";
                int i = 0;
                while (i < name_len) {
                    if (name_t[i] == '/') {
                        name_t[i] = '.';
                    }
                    i++;
                }
                if (strstr(name_t, ".lua")) {
                    sprintf(full_name, "%s%s", base_dir, name_t);
                    //lua脚本保存
                    file = fopen(full_name, "wb");
                    if (file != NULL) {
                        fwrite(buff, 1, size, file);
                        fclose(file);
                        free(name_t);
                    }



                    //lua脚本hook加载
                    /*file = fopen(full_name, "r");
                    if (file != NULL) {
                        LOGD("[Tencent]-------path-----%s", full_name);
                        fseek(file, 0, SEEK_END);
                        size_t new_size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        char *new_buff = (char *) alloca(new_size + 1);
                        fread(new_buff, new_size, 1, file);
                        fclose(file);
                        return origin_luaL_loadbuffer(lua_state, buff, size, name);
                    }*/




                }
            }
        }
    }

    return origin_luaL_loadbuffer(lua_state, buff, size, name);
}


//mono_image_open_from_data_with_name
//        (char *data,
//         guint32 data_len,
//         gboolean need_copy,
//         MonoImageOpenStatus *status,
//         gboolean refonly,
//         const char *name)
//data: 脚本内容
//
//data_len:脚本长度
//
//name:脚本名称

MonoImage * my_mono_image_init_mod(char *data,
                                   size_t data_len,
                                   int need_copy,
                                   void *status,
                                   int refonly,
                                   char *name) {


    if (name != NULL) {
        //strdup()在内部调用了malloc()为变量分配内存
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            //if (8 < name_len <= 100) {
            char *base_dir = (char *) "/sdcard/hookDll/";
            int i = 0;
            while (i < name_len) {
                //将 / 换成 .
                if (name_t[i] == '/') {
                    name_t[i] = '.';
                }
                i++;
            }
            //lua脚本保存
            file = fopen(full_name, "wb");
            if (file != NULL) {
                fwrite(data, 1, data_len, file);
                fclose(file);
                free(name_t);
            }

        }
    }
    return my_mono_image_init_mod_t(data,data_len,need_copy,status,refonly,name);

}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {


    LOGD("JNI_OnLoad enter");
    //在 onload 改变 指定函数 函数地址 替换成自己的
    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOGD("GetEnv OK");

        char so_name[128] = {0};
        //吧 TARGET_SO 内容写到so_name
        sprintf(so_name, TARGET_SO, PACKAGE_NAME);
        //该函数将打开一个新库 返回个句柄
        void *handle = dlopen(so_name, RTLD_NOW);


        if (handle) {
            LOGD("dlopen() return %08x", (uint32_t) handle);


            if (isU3d) {
                //从句柄里拿到 luaL_loadbuffer 函数
                //根据 动态链接库 操作句柄(handle)与符号(symbol)，
                // 返回符号对应的地址。使用这个函数不但可以获取函数地址，
                // 也可以获取变量地址。
                //是否是 dump  u3d的
                void *mono_image_open = dlsym(handle, "mono_image_open_from_data_with_name");

                LOGD("dlsym() ", (uint32_t) mono_image_open);
                if (mono_image_open) {
                    if (ELE7EN_OK == registerInlineHook((uint32_t) mono_image_open,
                                                        (uint32_t) my_mono_image_init_mod,
                                                        (uint32_t **) &my_mono_image_init_mod_t)) {

                        LOGD("registerInlineHook mono_image_open_from_data_with_name success");
                        if (ELE7EN_OK == inlineHook((uint32_t) mono_image_open)) {
                            LOGD("inlineHook mono_image_open_from_data_with_name success");
                        } else {
                            LOGD("inlineHook mono_image_open_from_data_with_name failure");
                        }
                    } else {
                        LOGD("registerInlineHook mono_image_open_from_data_with_name failure");
                    }
                }
///         dump lua的 Hook的 函数
            } else {
                LOGD(TAG, "Lua");
                void *luabuffer = dlsym(handle, "luaL_loadbuffer");       //luaL_loadbuffer
                LOGD(TAG, "luaL_loadbuffer");
                if (luabuffer) {
                    LOGD("luaL_loadbuffer function address:%08X", (uint32_t) luabuffer);
                    //这行 是 你需要 Hook的 函数 和原函数名字的 地址进行替换
                    if (ELE7EN_OK == registerInlineHook((uint32_t) luabuffer,
                                                        (uint32_t) my_luaL_loadbuffer,
                                                        (uint32_t **) &origin_luaL_loadbuffer)) {
                        LOGD("registerInlineHook luaL_loadbuffer success");
                        if (ELE7EN_OK == inlineHook((uint32_t) luabuffer)) {
                            LOGD("inlineHook luaL_loadbuffer success");
                        }
                    }
                }
            }
        }
        LOGD("JNI_OnLoad leave");
        return JNI_VERSION_1_6;
    }
}


