/*
 Copyright (c) 2019 MICROTRUST Incorporated
 All rights reserved

 This file and software is confidential and proprietary to MICROTRUST Inc.
 Unauthorized copying of this file and software is strictly prohibited.
 You MUST NOT disclose this file and software unless you get a license
 agreement from MICROTRUST Incorporated.
*/

package teei_daemon

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
)

func init() {
    // for Debug
    fmt.Println("init teei_daemon start");
    android.RegisterModuleType("microtrust_teei_daemon_defaults", teei_daemonDefaultsFactory);
}

func teei_daemonDefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, teei_daemonDefaults)
    return module
}


func teei_daemonDefaults(ctx android.LoadHookContext) {

    type props struct {
        Cflags []string
    }

    p := &props{}
    p.Cflags = globalDefaults(ctx)
    ctx.AppendProperties(p)

}


func globalDefaults(ctx android.BaseContext) ([]string) {
    var cppflags []string
    // Get config from Projectconfig.mk
    vars := ctx.Config().VendorConfig("mtkPlugin")
    // For debug print config, too much make log
    //fmt.Println("microtrust vendor config:", vars)
    // Get MICROTRUST_TEE_CONFIG value
    fmt.Println("microtrust tee support:", vars.Bool("MICROTRUST_TEE_SUPPORT"))
    fmt.Println("microtrust svp support:", vars.Bool("MTK_SEC_VIDEO_PATH_SUPPORT"))
    fmt.Println("microtrust sc support:", vars.Bool("MTK_CAM_SECURITY_SUPPORT"))

    ret_tee := vars.String("MICROTRUST_TEE_SUPPORT")
    if ret_tee == "yes" {
        fmt.Println("microtrust tee is supported")
        cppflags = append(cppflags, "-DMICROTRUST_TEE_SUPPORT")
    }


    ret_drm := vars.String("MTK_SEC_VIDEO_PATH_SUPPORT")
    if ret_drm == "yes" {
        fmt.Println("MTK_SEC_VIDEO_PATH_SUPPORT is supported")
        cppflags = append(cppflags, "-DMTK_DRM_SUPPORT")
    }

    ret_cam := vars.String("MTK_CAM_SECURITY_SUPPORT")
    if ret_cam == "yes" {
        fmt.Println("MTK_CAM_SECURITY_SUPPORT is supported")
        cppflags = append(cppflags, "-DMTK_CAM_SUPPORT")
    }

    ret_sdsp := vars.String("MTK_GZ_SUPPORT_SDSP")
    if ret_sdsp == "yes" {
        fmt.Println("MTK_GZ_SUPPORT_SDSP is supported")
        cppflags = append(cppflags, "-DMTK_SDSP_SUPPORT")
    }

    // Get env config
    fmt.Println("microtrust TARGET_BUILD_VARIANT:",
        ctx.AConfig().Getenv("TARGET_BUILD_VARIANT"))

    if ctx.AConfig().IsEnvTrue("ENABLE_USER2ENG") {
          cppflags = append(cppflags,
                         "-DALLOW_ADBD_DISABLE_VERITY=1",
                         "-DENABLE_USER2ENG=1")
    }

    return cppflags
}
