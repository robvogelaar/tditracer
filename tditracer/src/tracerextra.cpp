#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "tdi.h"

// If we need this, there is some egl header that we will need to add.
//#include "/data_local/workspace/build/DMC7002KLG_CADA_MW_DEBUG/staging/usr/include/refsw/default_nexus.h"


#include <refsw/nexus_config.h>
#include <refsw/nexus_platform.h>
#include <refsw/nexus_display.h>
#include <refsw/nexus_core_utils.h>
#include <refsw/default_nexus.h>

// #include "refsw/nexus_frontend.h"


#include "/home/rev/git/buildroot/output/build/bcm-refsw-20121210/nexus/modules/frontend/common/include/nexus_frontend_qam.h"

#include "refsw/nexus_platform.h"
#include "refsw/nexus_platform_client.h"

#include "refsw/nexus_base_types.h"

#include "refsw/nexus_security.h"
#include "refsw/nexus_display_vbi.h"
#include "refsw/nexus_graphics2d.h"
#include "refsw/nexus_hdmi_output_hdcp.h"
#include "refsw/nexus_message.h"
#include "refsw/nexus_mpod.h"
#include "refsw/nexus_recpump.h"

#include "refsw/nexus_surface_client.h"
#include "refsw/nexus_surface_compositor.h"

#include "refsw/nexus_sync_channel.h"

#include "refsw/nexus_video_decoder.h"
#include "refsw/nexus_video_decoder_types.h"
#include "refsw/nexus_video_adj.h"
#include "refsw/nexus_video_decoder_primer.h"
#include "refsw/nexus_video_input_vbi.h"

#include "refsw/nexus_audio.h"
#include "refsw/nexus_audio_decoder.h"
#include "refsw/nexus_audio_decoder_types.h"
#include "refsw/nexus_audio_decoder_primer.h"
#include "refsw/nexus_audio_decoder_trick.h"
#include "refsw/nexus_audio_input.h"
#include "refsw/nexus_audio_output.h"
#include "refsw/nexus_audio_types.h"


#include "refsw/nexus_sync_channel.h"
#include "refsw/nexus_pid_channel.h"



//#include "/data_local/workspace/.buildbox/root/RDK/trunk/ri/mpe_os/target/hal/include/vlHalSubtitles.h"
//#include "/data_local/workspace/.buildbox/root/RDK/trunk/ri/mpe_os/target/hal/include/vlHalRefswClient.h"


extern "C" int extra(int in, int *out) {

    static int (*__extra)(int, int *) = NULL;

    if (__extra == NULL) {
        __extra = (int (*)(int, int *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __extra) {
            fprintf(stderr, "Error in `dlsym`: %s %s\n", __func__, dlerror());
        }
    }

    tditrace_ex("@T+extra()");
    int ret = __extra(in, out);
    tditrace_ex("@T-extra()");

    return ret;
}

extern void pbPostInputKeyEvent(unsigned int key, bool rel)
{
    static void (*__pbPostInputKeyEvent)(uint32_t, bool) = NULL;

    if (__pbPostInputKeyEvent==NULL) {
        __pbPostInputKeyEvent = (void (*)(uint32_t, bool))dlsym(RTLD_NEXT, "_Z19pbPostInputKeyEventjb");
        if (NULL == __pbPostInputKeyEvent) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+pbPostInputKeyEvent() %d %s", key, rel ? "TRUE":"FALSE");
    __pbPostInputKeyEvent(key, rel);
    tditrace_ex("@T-pbPostInputKeyEvent()");
}

extern "C" NEXUS_Error NEXUS_Frontend_GetQamStatus(NEXUS_FrontendHandle handle, NEXUS_FrontendQamStatus *pStatus)
{
    static NEXUS_Error (*__NEXUS_Frontend_GetQamStatus)(NEXUS_FrontendHandle, NEXUS_FrontendQamStatus *) = NULL;

    if (__NEXUS_Frontend_GetQamStatus==NULL) {
        __NEXUS_Frontend_GetQamStatus = (NEXUS_Error (*)(NEXUS_FrontendHandle, NEXUS_FrontendQamStatus *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_Frontend_GetQamStatus) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_Frontend_GetQamStatus()");
    NEXUS_Error ret = __NEXUS_Frontend_GetQamStatus(handle, pStatus);
    tditrace_ex("@T-NEXUS_Frontend_GetQamStatus()");

    return ret;
}

extern "C" NEXUS_Error NEXUS_Frontend_TuneQam(NEXUS_FrontendHandle handle, const NEXUS_FrontendQamSettings *pSettings)
{
    static NEXUS_Error (*__NEXUS_Frontend_TuneQam)(NEXUS_FrontendHandle, const NEXUS_FrontendQamSettings *) = NULL;

    if (__NEXUS_Frontend_TuneQam==NULL) {
        __NEXUS_Frontend_TuneQam = (NEXUS_Error (*)(NEXUS_FrontendHandle, const NEXUS_FrontendQamSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_Frontend_TuneQam) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_Frontend_TuneQam() freq=%d", pSettings->frequency);
    NEXUS_Error ret = __NEXUS_Frontend_TuneQam(handle, pSettings);
    tditrace_ex("@T-NEXUS_Frontend_TuneQam()", pSettings->frequency);

    return ret;
}

extern "C" NEXUS_KeySlotHandle NEXUS_Security_AllocateKeySlot(const NEXUS_SecurityKeySlotSettings *pSettings)
{
    static NEXUS_KeySlotHandle (*__NEXUS_Security_AllocateKeySlot)(const NEXUS_SecurityKeySlotSettings *) = NULL;

    if (__NEXUS_Security_AllocateKeySlot == NULL)
    {
        __NEXUS_Security_AllocateKeySlot = (NEXUS_KeySlotHandle (*)(const NEXUS_SecurityKeySlotSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_Security_AllocateKeySlot)
        {
            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_Security_AllocateKeySlot()");
    NEXUS_KeySlotHandle ret = __NEXUS_Security_AllocateKeySlot(pSettings);
    tditrace_ex("@T-NEXUS_Security_AllocateKeySlot()");

    return ret;
}

extern "C" NEXUS_Error NEXUS_VideoDecoder_Start(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderStartSettings *pSettings)
{
    static NEXUS_Error (*__NEXUS_VideoDecoder_Start)(NEXUS_VideoDecoderHandle, const NEXUS_VideoDecoderStartSettings *) = NULL;

    if (__NEXUS_VideoDecoder_Start==NULL) {
        __NEXUS_VideoDecoder_Start = (NEXUS_Error (*)(NEXUS_VideoDecoderHandle, const NEXUS_VideoDecoderStartSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_VideoDecoder_Start) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_VideoDecoder_Start() 0x%x", handle);
    NEXUS_Error ret = __NEXUS_VideoDecoder_Start(handle, pSettings);
    tditrace_ex("@T-NEXUS_VideoDecoder_Start()", handle);

    return ret;
}

extern "C" void NEXUS_VideoDecoder_Stop(NEXUS_VideoDecoderHandle handle)
{
    static void(*__NEXUS_VideoDecoder_Stop)(NEXUS_VideoDecoderHandle) = NULL;

    if (__NEXUS_VideoDecoder_Stop==NULL) {
        __NEXUS_VideoDecoder_Stop = (void (*)(NEXUS_VideoDecoderHandle))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_VideoDecoder_Stop) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_VideoDecoder_Stop() 0x%x", handle);
    __NEXUS_VideoDecoder_Stop(handle);
    tditrace_ex("@T-NEXUS_VideoDecoder_Stop()", handle);
}


//int vlhal_view_StartAudio(void *audio_pid, int format, void *stc, void *avsync)
//int vlhal_view_StopAudio()
//int vlhal_view_MuteAudio(bool mute)

extern "C" int vlhal_view_StartAudio(void *audio_pid, int format, void *stc, void *avsync)
{
    static int (*__vlhal_view_StartAudio)(void *, int, void *, void *) = NULL;

    if (__vlhal_view_StartAudio==NULL) {
        __vlhal_view_StartAudio = (int (*)(void *, int, void *, void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_StartAudio) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_StartAudio()");
    int ret = __vlhal_view_StartAudio(audio_pid, format, stc, avsync);
    tditrace_ex("@T-vlhal_view_StartAudio()");

    return ret;
}

extern "C" int vlhal_view_StopAudio()
{
    static int (*__vlhal_view_StopAudio)() = NULL;

    if (__vlhal_view_StopAudio==NULL) {
        __vlhal_view_StopAudio = (int (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_StopAudio) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_StopAudio()");
    int ret = __vlhal_view_StopAudio();
    tditrace_ex("@T-vlhal_view_StopAudio()");

    return ret;
}

extern "C" int vlhal_view_MuteAudio(bool mute)
{
    static int (*__vlhal_view_MuteAudio)(bool) = NULL;

    if (__vlhal_view_MuteAudio==NULL) {
        __vlhal_view_MuteAudio = (int (*)(bool))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_MuteAudio) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_MuteAudio() %s", mute ? "TRUE":"FALSE");
    int ret = __vlhal_view_MuteAudio(mute);
    tditrace_ex("@T-vlhal_view_MuteAudio()");

    return ret;
}


extern "C" int vlhal_view_ConnectAndStartVideo(void *decoder, void *disp)
{
    static int(*__vlhal_view_ConnectAndStartVideo)(void *, void *) = NULL;

    if (__vlhal_view_ConnectAndStartVideo==NULL) {
        __vlhal_view_ConnectAndStartVideo = (int (*)(void*, void*))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_ConnectAndStartVideo) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_ConnectAndStartVideo() %x %x", decoder, disp);
    int ret = __vlhal_view_ConnectAndStartVideo(decoder, disp);
    tditrace_ex("@T-vlhal_view_ConnectAndStartVideo()");

    return ret;
}

extern "C" int vlhal_view_DisconnectAndStopVideo(void *disp)
{
    static int (*__vlhal_view_DisconnectAndStopVideo)(void *) = NULL;

    if (__vlhal_view_DisconnectAndStopVideo==NULL) {
        __vlhal_view_DisconnectAndStopVideo = (int (*)(void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_DisconnectAndStopVideo) {
            fprintf(stderr, "Error in `dlsym`: %s %s\n", __func__, dlerror());
        }
    }

    tditrace("@T+vlhal_view_DisconnectAndStopVideo() %x", disp);
    __vlhal_view_DisconnectAndStopVideo(disp);
    tditrace("@T-vlhal_view_DisconnectAndStopVideo()");
}

#if 1
//extern "C" void NEXUS_AudioDecoder_GetDefaultOpenSettings(NEXUS_AudioDecoderOpenSettings *pSettings)
//{
//    static void (*__NEXUS_AudioDecoder_GetDefaultOpenSettings)(NEXUS_AudioDecoderOpenSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_GetDefaultOpenSettings == NULL)
//    {
//       __NEXUS_AudioDecoder_GetDefaultOpenSettings = (void (*)(NEXUS_AudioDecoderOpenSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_GetDefaultOpenSettings)
//        {
//            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_GetDefaultOpenSettings()");
//    __NEXUS_AudioDecoder_GetDefaultOpenSettings(pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_GetDefaultOpenSettings()");
//}
//
//extern "C" NEXUS_AudioDecoderHandle NEXUS_AudioDecoder_Open(unsigned index, const NEXUS_AudioDecoderOpenSettings *pSettings)
//{
//    static NEXUS_AudioDecoderHandle (*__NEXUS_AudioDecoder_Open)(uint32_t, const  NEXUS_AudioDecoderOpenSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_Open == NULL)
//    {
//       __NEXUS_AudioDecoder_Open = (NEXUS_AudioDecoderHandle (*)(uint32_t, const  NEXUS_AudioDecoderOpenSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_Open)
//        {
//            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_Open()");
//    NEXUS_AudioDecoderHandle ret = __NEXUS_AudioDecoder_Open(index, pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_Open()");
//    return ret;
//}
//
//extern "C" void NEXUS_AudioDecoder_GetCodecSettings(NEXUS_AudioDecoderHandle handle, NEXUS_AudioCodec codec, NEXUS_AudioDecoderCodecSettings *pSettings)
//{
//    static void (*__NEXUS_AudioDecoder_GetCodecSettings)(NEXUS_AudioDecoderHandle, NEXUS_AudioCodec, NEXUS_AudioDecoderCodecSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_GetCodecSettings == NULL)
//    {
//        __NEXUS_AudioDecoder_GetCodecSettings = (void (*)(NEXUS_AudioDecoderHandle, NEXUS_AudioCodec, NEXUS_AudioDecoderCodecSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_GetCodecSettings)
//        {
//            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_GetCodecSettings()");
//    __NEXUS_AudioDecoder_GetCodecSettings(handle, codec, pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_GetCodecSettings()");
//}
//
//extern "C" NEXUS_Error NEXUS_AudioDecoder_SetCodecSettings(NEXUS_AudioDecoderHandle handle, const NEXUS_AudioDecoderCodecSettings *pSettings)
//{
//    static NEXUS_Error (*__NEXUS_AudioDecoder_SetCodecSettings)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderCodecSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_SetCodecSettings == NULL)
//    {
//        __NEXUS_AudioDecoder_SetCodecSettings = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderCodecSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_SetCodecSettings)
//        {
//            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_SetCodecSettings()");
//    NEXUS_Error ret = __NEXUS_AudioDecoder_SetCodecSettings(handle, pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_SetCodecSettings()");
//    return ret;
//}
//
//extern "C" void NEXUS_AudioDecoder_Close(NEXUS_AudioDecoderHandle handle)
//{
//    static NEXUS_Error (*__NEXUS_AudioDecoder_Close)(NEXUS_AudioDecoderHandle) = NULL;
//
//    if (__NEXUS_AudioDecoder_Close==NULL) {
//       __NEXUS_AudioDecoder_Close = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_Close) {
//            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_Close() 0x%x", handle);
//    NEXUS_Error ret = __NEXUS_AudioDecoder_Close(handle);
//    tditrace_ex("@T-NEXUS_AudioDecoder_Close()", handle);
//}
//
//extern "C" NEXUS_AudioInput NEXUS_AudioDecoder_GetConnector(NEXUS_AudioDecoderHandle handle, NEXUS_AudioConnectorType type)
//{
//    static NEXUS_AudioInput (*__NEXUS_AudioDecoder_GetConnector)(NEXUS_AudioDecoderHandle, NEXUS_AudioConnectorType) = NULL;
//
//    if (__NEXUS_AudioDecoder_GetConnector==NULL) {
//       __NEXUS_AudioDecoder_GetConnector = (NEXUS_AudioInput (*)(NEXUS_AudioDecoderHandle, NEXUS_AudioConnectorType))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_GetConnector) {
//            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_GetConnector()");
//    NEXUS_AudioInput ret = __NEXUS_AudioDecoder_GetConnector(handle, type);
//    tditrace_ex("@T-NEXUS_AudioDecoder_GetConnector()");
//    return ret;
//}
//
//extern "C" void NEXUS_AudioDecoder_GetSettings(NEXUS_AudioDecoderHandle handle, NEXUS_AudioDecoderSettings *pSettings)
//{
//    static NEXUS_Error (*__NEXUS_AudioDecoder_GetSettings)(NEXUS_AudioDecoderHandle, NEXUS_AudioDecoderSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_GetSettings==NULL) {
//        __NEXUS_AudioDecoder_GetSettings = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle, NEXUS_AudioDecoderSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_GetSettings) {
//            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_GetSettings()");
//    NEXUS_Error ret = __NEXUS_AudioDecoder_GetSettings(handle, pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_GetSettings()");
//}
//
//extern "C" NEXUS_Error NEXUS_AudioDecoder_SetSettings(NEXUS_AudioDecoderHandle handle, const NEXUS_AudioDecoderSettings *pSettings)
//{
//    static NEXUS_Error (*__NEXUS_AudioDecoder_SetSettings)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_SetSettings==NULL) {
//       __NEXUS_AudioDecoder_SetSettings = (NEXUS_Error(*)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_SetSettings) {
//            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_SetSettings()");
//    NEXUS_Error ret = __NEXUS_AudioDecoder_SetSettings(handle, pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_SetSettings()");
//    return ret;
//}
//
//extern "C" void NEXUS_AudioDecoder_GetDefaultStartSettings(NEXUS_AudioDecoderStartSettings *pSettings)
//{
//    static NEXUS_Error (*__NEXUS_AudioDecoder_GetDefaultStartSettings)(NEXUS_AudioDecoderStartSettings *) = NULL;
//
//    if (__NEXUS_AudioDecoder_GetDefaultStartSettings==NULL) {
//       __NEXUS_AudioDecoder_GetDefaultStartSettings = (NEXUS_Error (*)(NEXUS_AudioDecoderStartSettings *))dlsym(RTLD_NEXT, __func__);
//        if (NULL == __NEXUS_AudioDecoder_GetDefaultStartSettings) {
//            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
//        }
//    }
//
//    tditrace_ex("@T+NEXUS_AudioDecoder_GetDefaultStartSettings()");
//    NEXUS_Error ret = __NEXUS_AudioDecoder_GetDefaultStartSettings(pSettings);
//    tditrace_ex("@T-NEXUS_AudioDecoder_GetDefaultStartSettings()");
//}
//


extern "C" NEXUS_Error NEXUS_AudioDecoder_Start(NEXUS_AudioDecoderHandle handle, const NEXUS_AudioDecoderStartSettings *pSettings)
{
    static NEXUS_Error (*__NEXUS_AudioDecoder_Start)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderStartSettings *) = NULL;

    if (__NEXUS_AudioDecoder_Start==NULL) {
       __NEXUS_AudioDecoder_Start = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderStartSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_Start) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_Start()");
    NEXUS_Error ret = __NEXUS_AudioDecoder_Start(handle, pSettings);
    tditrace_ex("@T-NEXUS_AudioDecoder_Start()");

    return ret;
}

extern "C" void NEXUS_AudioDecoder_Stop(NEXUS_AudioDecoderHandle handle)
{
    static void (*__NEXUS_AudioDecoder_Stop)(NEXUS_AudioDecoderHandle) = NULL;

    if (__NEXUS_AudioDecoder_Stop==NULL) {
       __NEXUS_AudioDecoder_Stop = (void (*)(NEXUS_AudioDecoderHandle))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_Stop) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_Stop()");
    __NEXUS_AudioDecoder_Stop(handle);
    tditrace_ex("@T-NEXUS_AudioDecoder_Stop()");
}

extern "C" NEXUS_Error NEXUS_AudioDecoder_Flush(NEXUS_AudioDecoderHandle handle)
{
    static NEXUS_Error (*__NEXUS_AudioDecoder_Flush)(NEXUS_AudioDecoderHandle) = NULL;

    if (__NEXUS_AudioDecoder_Flush==NULL) {
       __NEXUS_AudioDecoder_Flush = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_Flush) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_Flush()");
    NEXUS_Error ret = __NEXUS_AudioDecoder_Flush(handle);
    tditrace_ex("@T-NEXUS_AudioDecoder_Flush()");
    return ret;
}

extern "C" NEXUS_Error NEXUS_AudioDecoder_GetStatus(NEXUS_AudioDecoderHandle handle, NEXUS_AudioDecoderStatus *pStatus)
{
    static NEXUS_Error (*__NEXUS_AudioDecoder_GetStatus)(NEXUS_AudioDecoderHandle, NEXUS_AudioDecoderStatus *) = NULL;

    if (__NEXUS_AudioDecoder_GetStatus==NULL) {
       __NEXUS_AudioDecoder_GetStatus = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle, NEXUS_AudioDecoderStatus *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_GetStatus) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_GetStatus()");
    NEXUS_Error ret = __NEXUS_AudioDecoder_GetStatus(handle, pStatus);
    tditrace_ex("@T-NEXUS_AudioDecoder_GetStatus()");
    return ret;
}

extern "C" void NEXUS_AudioDecoder_GetTrickState(NEXUS_AudioDecoderHandle decoder, NEXUS_AudioDecoderTrickState *pState)
{
    static void (*__NEXUS_AudioDecoder_GetTrickState)(NEXUS_AudioDecoderHandle, NEXUS_AudioDecoderTrickState *) = NULL;

    if (__NEXUS_AudioDecoder_GetTrickState==NULL) {
       __NEXUS_AudioDecoder_GetTrickState = (void (*)(NEXUS_AudioDecoderHandle, NEXUS_AudioDecoderTrickState *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_GetTrickState) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_GetTrickState()");
    __NEXUS_AudioDecoder_GetTrickState(decoder, pState);
    tditrace_ex("@T-NEXUS_AudioDecoder_GetTrickState()");
}

extern "C" NEXUS_Error NEXUS_AudioDecoder_SetTrickState(NEXUS_AudioDecoderHandle decoder, const NEXUS_AudioDecoderTrickState *pTrickState)
{
    static NEXUS_Error (*__NEXUS_AudioDecoder_GetTrickState)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderTrickState *) = NULL;

    if (__NEXUS_AudioDecoder_GetTrickState==NULL) {
       __NEXUS_AudioDecoder_GetTrickState = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle, const NEXUS_AudioDecoderTrickState *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_GetTrickState) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_SetTrickState");
    NEXUS_Error ret = __NEXUS_AudioDecoder_GetTrickState(decoder, pTrickState);
    tditrace_ex("@T-NEXUS_AudioDecoder_SetTrickState");
    return ret;
}

extern "C" NEXUS_Error NEXUS_AudioDecoder_Advance(NEXUS_AudioDecoderHandle decoder, uint32_t targetPts)
{
    static NEXUS_Error (*__NEXUS_AudioDecoder_Advance)(NEXUS_AudioDecoderHandle , uint32_t) = NULL;

    if (__NEXUS_AudioDecoder_Advance==NULL) {
       __NEXUS_AudioDecoder_Advance = (NEXUS_Error (*)(NEXUS_AudioDecoderHandle , uint32_t))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDecoder_Advance) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDecoder_Advance()");
    NEXUS_Error ret = __NEXUS_AudioDecoder_Advance(decoder, targetPts);
    tditrace_ex("@T-NEXUS_AudioDecoder_Advance()");
    return ret;
}
#endif


#if 0
extern "C" int vlhal_ttx_Init()
{
    static int (*__vlhal_ttx_Init)() = NULL;

    if (__vlhal_ttx_Init==NULL) {
        __vlhal_ttx_Init = (int (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_Init) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_ttx_Init()");
    int ret = __vlhal_ttx_Init();
    tditrace_ex("@T-vlhal_ttx_Init()");

    return ret;
}

extern "C" void vlhal_ttx_DecodeStart(unsigned int pid, unsigned int tuner_id)
{
    static void (*__vlhal_ttx_DecodeStart)(uint32_t, uint32_t) = NULL;

    if (__vlhal_ttx_DecodeStart==NULL) {
        __vlhal_ttx_DecodeStart = (void (*)(uint32_t, uint32_t))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_DecodeStart) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }
    tditrace_ex("@T+vlhal_ttx_DecodeStart()");
    __vlhal_ttx_DecodeStart(pid, tuner_id);
    tditrace_ex("@T-vlhal_ttx_DecodeStart()");

}

extern "C" void vlhal_ttx_DecodeStop()
{
    static void (*__vlhal_ttx_DecodeStop)() = NULL;

    if (__vlhal_ttx_DecodeStop==NULL) {
        __vlhal_ttx_DecodeStop = (void (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_DecodeStop) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_ttx_DecodeStop()");
    __vlhal_ttx_DecodeStop();
    tditrace_ex("@T-vlhal_ttx_DecodeStop()");
}

extern "C" void vlhal_ttx_DecodeTrickPlay(int enable)
{
    static void (*__vlhal_ttx_DecodeTrickPlay)(int) = NULL;

    if (__vlhal_ttx_DecodeTrickPlay==NULL) {
        __vlhal_ttx_DecodeTrickPlay = (void (*)(int))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_DecodeTrickPlay) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }
    tditrace_ex("@T+vlhal_ttx_DecodeTrickPlay()");
    __vlhal_ttx_DecodeTrickPlay(enable);
    tditrace_ex("@T-vlhal_ttx_DecodeTrickPlay()");
}

extern "C" void vlhal_ttx_DecodeMute(int enable)
{
    static void (*__vlhal_ttx_DecodeMute)(int) = NULL;

    if (__vlhal_ttx_DecodeMute==NULL) {
        __vlhal_ttx_DecodeMute = (void (*)(int))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_DecodeMute) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }
    tditrace_ex("@T+vlhal_ttx_DecodeMute()");
    __vlhal_ttx_DecodeMute(enable);
    tditrace_ex("@T-vlhal_ttx_DecodeMute()");
}

extern "C" void vlhal_ttx_Start()
{
    static void (*__vlhal_ttx_Start)() = NULL;

    if (__vlhal_ttx_Start==NULL) {
        __vlhal_ttx_Start = (void (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_Start) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_ttx_Start()");
    __vlhal_ttx_Start();
    tditrace_ex("@T-vlhal_ttx_Start()");
}

extern "C" void vlhal_ttx_Stop()
{
    static void (*__vlhal_ttx_Stop)() = NULL;

    if (__vlhal_ttx_Stop==NULL) {
        __vlhal_ttx_Stop = (void (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_ttx_Stop) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_ttx_Stop()");
    __vlhal_ttx_Stop();
    tditrace_ex("@T-vlhal_ttx_Stop()");
}
#endif


#if 0
extern "C" int vlHalSubtitle_Start(vlHalSubtitleDef *driver)
{
    static int (*__vlHalSubtitle_Start)(vlHalSubtitleDef *) = NULL;

    if (__vlHalSubtitle_Start==NULL) {
        __vlHalSubtitle_Start = (int (*)(vlHalSubtitleDef *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlHalSubtitle_Start) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }
    tditrace_ex("@T+vlHalSubtitle_Start()");
    int ret = __vlHalSubtitle_Start(driver);
    tditrace_ex("@T-vlHalSubtitle_Start()");
    return ret;
}

extern "C" void vlHalSubtitle_Stop(vlHalSubtitleDef *driver)
{
    static void (*__vlHalSubtitle_Stop)(vlHalSubtitleDef *) = NULL;

    if (__vlHalSubtitle_Stop==NULL) {
        __vlHalSubtitle_Stop = (void (*)(vlHalSubtitleDef *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlHalSubtitle_Stop) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }
    tditrace_ex("@T+vlHalSubtitle_Stop()");
    __vlHalSubtitle_Stop(driver);
    tditrace_ex("@T-vlHalSubtitle_Stop()");
}

extern "C" void vlHalSubtitle_Mute(vlHalSubtitleDef *driver)
{
    static void (*__vlHalSubtitle_Mute)(vlHalSubtitleDef *) = NULL;

    if (__vlHalSubtitle_Mute==NULL) {
        __vlHalSubtitle_Mute = (void (*)(vlHalSubtitleDef *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlHalSubtitle_Mute) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }
    tditrace_ex("@T+vlHalSubtitle_Mute()");
    __vlHalSubtitle_Mute(driver);
    tditrace_ex("@T-vlHalSubtitle_Mute()");
}
#endif

#if 0
extern "C" void vlhal_refsw_client_Init()
{
    static void (*__vlhal_refsw_client_Init)() = NULL;

    if (__vlhal_refsw_client_Init == NULL) {
        __vlhal_refsw_client_Init = (void (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_Init) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_Init()");
    __vlhal_refsw_client_Init();
    tditrace_ex("@T-vlhal_refsw_client_Init()");
}

extern "C" void *vlhal_refsw_client_GetPcamNode()
{
    static void *(*__vlhal_refsw_client_GetPcamNode)() = NULL;

    if (__vlhal_refsw_client_GetPcamNode == NULL) {
        __vlhal_refsw_client_GetPcamNode = (void *(*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_GetPcamNode) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_GetPcamNode()");
    __vlhal_refsw_client_GetPcamNode();
    tditrace_ex("@T-vlhal_refsw_client_GetPcamNode()");
}

extern "C" void vlhal_refsw_client_GetSecret(char *buffer, unsigned *length)
{
    static void (*__vlhal_refsw_client_GetSecret)(char *, uint32_t *) = NULL;

    if (__vlhal_refsw_client_GetSecret==NULL) {
       __vlhal_refsw_client_GetSecret = (void (*)(char *, uint32_t *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_GetSecret) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_GetSecret()");
     __vlhal_refsw_client_GetSecret(buffer,length);
    tditrace_ex("@T-vlhal_refsw_client_GetSecret()");
}

extern "C" void vlhal_refsw_client_ProxyIsUp()
{
    static void (*__vlhal_refsw_client_ProxyIsUp)() = NULL;

    if (__vlhal_refsw_client_ProxyIsUp == NULL) {
        __vlhal_refsw_client_ProxyIsUp = (void (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_ProxyIsUp) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_ProxyIsUp()");
    __vlhal_refsw_client_ProxyIsUp();
    tditrace_ex("@T-vlhal_refsw_client_ProxyIsUp()");
}

extern "C" void vlhal_refsw_client_SetPowerLevel(int pass, char *level, bool isMocaEnabled, unsigned long timeout)
{
    static void (*__vlhal_refsw_client_SetPowerLevel)(int, char *, bool, unsigned long) = NULL;

    if (__vlhal_refsw_client_SetPowerLevel==NULL) {
       __vlhal_refsw_client_SetPowerLevel = (void (*)(int, char *, bool, unsigned long))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_SetPowerLevel) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_SetPowerLevel()");
    __vlhal_refsw_client_SetPowerLevel(pass, level, isMocaEnabled, timeout);
    tditrace_ex("@T-vlhal_refsw_client_SetPowerLevel()");
}
#endif


#if 0        //causing build fail
extern "C" Pin *get_video_pin(void *disp)
{
    static Pin *(*__get_video_pin)(void *) = NULL;

    if (__get_video_pin==NULL) {
       __get_video_pin = (Pin *(*)(void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __get_video_pin) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+get_video_pin()");
    Pin ret = __get_video_pin(disp);
    tditrace_ex("@T-get_video_pin()");
    return ret;
}
#endif

extern "C" void vlhal_view_EnableDolby(bool enable)
{
    static void (*__vlhal_view_EnableDolby)(bool) = NULL;

    if (__vlhal_view_EnableDolby==NULL) {
        __vlhal_view_EnableDolby = (void (*)(bool))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_EnableDolby) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_EnableDolby() %s", enable ? "TRUE":"FALSE");
    __vlhal_view_EnableDolby(enable);
    tditrace_ex("@T-vlhal_view_EnableDolby()");
}

extern "C" int vlhal_view_SetAudioRate(float rate)
{
    static int (*__vlhal_view_SetAudioRate)(float) = NULL;

    if (__vlhal_view_SetAudioRate==NULL) {
        __vlhal_view_SetAudioRate = (int (*)(float))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_SetAudioRate) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_SetAudioRate() %f", rate);
    int ret = __vlhal_view_SetAudioRate(rate);
    tditrace_ex("@T-vlhal_view_SetAudioRate()");
    return ret;
}

extern "C" int vlhal_view_GetVideoBounds(float *x, float *y, float *w, float *h, void *disp)
{
    static int (*__vlhal_view_GetVideoBounds)(float *, float *, float *, float *, void *) = NULL;

    if (__vlhal_view_GetVideoBounds==NULL) {
        __vlhal_view_GetVideoBounds = (int (*)(float *, float *, float *, float *, void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_GetVideoBounds) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_GetVideoBounds()");
    int ret = __vlhal_view_GetVideoBounds(x, y, w, h, disp);
    tditrace_ex("@T-vlhal_view_GetVideoBounds()");
    return ret;
}

extern "C" int vlhal_view_SetVideoBounds(int x, int y, int w, int h, void *disp)
{
    static int (*__vlhal_view_SetVideoBounds)(int, int, int, int, void *) = NULL;

    if (__vlhal_view_SetVideoBounds==NULL) {
        __vlhal_view_SetVideoBounds = (int (*)(int, int, int, int, void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_SetVideoBounds) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_SetVideoBounds()");
    int ret = __vlhal_view_SetVideoBounds(x, y, w, h, disp);
    tditrace_ex("@T-vlhal_view_SetVideoBounds()");
    return ret;
}

extern "C" int vlhal_view_AnimateVideoBounds(int x, int y, int w, int h, int window_number)
{
    static int (*__vlhal_view_AnimateVideoBounds)(int, int, int, int, int) = NULL;

    if (__vlhal_view_AnimateVideoBounds==NULL) {
        __vlhal_view_AnimateVideoBounds = (int (*)(int, int, int, int, int))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_AnimateVideoBounds) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_AnimateVideoBounds()");
    int ret = __vlhal_view_AnimateVideoBounds(x, y, w, h, window_number);
    tditrace_ex("@T-vlhal_view_AnimateVideoBounds()");
    return ret;
}

extern "C" int vlhal_view_SetVideoAlpha(void *disp, int is_pip)
{
    static int (*__vlhal_view_SetVideoAlpha)(void *, int) = NULL;

    if (__vlhal_view_SetVideoAlpha==NULL) {
        __vlhal_view_SetVideoAlpha = (int (*)(void *, int))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_SetVideoAlpha) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_SetVideoAlpha()");
    int ret = __vlhal_view_SetVideoAlpha(disp, is_pip);
    tditrace_ex("@T-vlhal_view_SetVideoAlpha()");
    return ret;
}

extern "C" void vlhal_view_SetVideoZOrder(void *disp, int zorder)
{
    static void (*__vlhal_view_SetVideoZOrder)(void *, int) = NULL;

    if (__vlhal_view_SetVideoZOrder==NULL) {
        __vlhal_view_SetVideoZOrder = (void (*)(void *, int))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_SetVideoZOrder) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_SetVideoZOrder()");
    __vlhal_view_SetVideoZOrder(disp, zorder);
    tditrace_ex("@T-vlhal_view_SetVideoZOrder()");
}

extern "C" void vlhal_view_SwapVideoZOrder()
{
    static void (*__vlhal_view_SwapVideoZOrder)() = NULL;

    if (__vlhal_view_SwapVideoZOrder==NULL) {
        __vlhal_view_SwapVideoZOrder = (void (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_SwapVideoZOrder) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_SwapVideoZOrder()");
    __vlhal_view_SwapVideoZOrder();
    tditrace_ex("@T-vlhal_view_SwapVideoZOrder()");
}

extern "C" int vlhal_view_ConnectVideo(void *decoder, void *disp)
{
    static int (*__vlhal_view_ConnectVideo)(void *, void *) = NULL;

    if (__vlhal_view_ConnectVideo==NULL) {
        __vlhal_view_ConnectVideo = (int (*)(void *, void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_ConnectVideo) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_ConnectVideo()");
    int ret = __vlhal_view_ConnectVideo(decoder, disp);
    tditrace_ex("@T-vlhal_view_ConnectVideo()");
    return ret;
}

extern "C" int vlhal_view_DisconnectVideo(void *disp)
{
    static int (*__vlhal_view_DisconnectVideo)(void *) = NULL;

    if (__vlhal_view_DisconnectVideo==NULL) {
        __vlhal_view_DisconnectVideo = (int (*)(void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_DisconnectVideo) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_DisconnectVideo()");
    int ret = __vlhal_view_DisconnectVideo(disp);
    tditrace_ex("@T-vlhal_view_DisconnectVideo()");
    return ret;
}

extern "C" int vlhal_view_StartVideo(void *disp)
{
    static int (*__vlhal_view_StartVideo)(void *) = NULL;

    if (__vlhal_view_StartVideo==NULL) {
        __vlhal_view_StartVideo = (int (*)(void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_StartVideo) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_StartVideo()");
    int ret = __vlhal_view_StartVideo(disp);
    tditrace_ex("@T-vlhal_view_StartVideo()");
    return ret;
}

extern "C" int vlhal_view_StopVideo(void *disp)
{
    static int (*__vlhal_view_StopVideo)(void *) = NULL;

    if (__vlhal_view_StopVideo==NULL) {
        __vlhal_view_StopVideo = (int (*)(void *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_StopVideo) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_StopVideo()");
    int ret = __vlhal_view_StopVideo(disp);
    tditrace_ex("@T-vlhal_view_StopVideo()");
    return ret;
}

extern "C" int vlhal_view_EnableAnalogCopy(bool enable)
{
    static int (*__vlhal_view_EnableAnalogCopy)(bool) = NULL;

    if (__vlhal_view_EnableAnalogCopy==NULL) {
        __vlhal_view_EnableAnalogCopy = (int (*)(bool))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_EnableAnalogCopy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_EnableAnalogCopy() %s", enable ? "TRUE":"FALSE");
    int ret = __vlhal_view_EnableAnalogCopy(enable);
    tditrace_ex("@T-vlhal_view_EnableAnalogCopy()");
    return ret;
}

extern "C" bool vlhal_view_GetHDCPEnabled()
{
    static bool (*__vlhal_view_GetHDCPEnabled)() = NULL;

    if (__vlhal_view_GetHDCPEnabled==NULL) {
        __vlhal_view_GetHDCPEnabled = (bool (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_GetHDCPEnabled) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_GetHDCPEnabled() ");
    bool ret = __vlhal_view_GetHDCPEnabled();
    tditrace_ex("@T-vlhal_view_GetHDCPEnabled()");
    return ret;
}

extern "C" bool vlhal_view_GetHDCPAuthenticated()
{
    static bool (*__vlhal_view_GetHDCPAuthenticated)() = NULL;

    if (__vlhal_view_GetHDCPAuthenticated==NULL) {
        __vlhal_view_GetHDCPAuthenticated = (bool (*)())dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_view_GetHDCPAuthenticated) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_view_GetHDCPAuthenticated() ");
    bool ret = __vlhal_view_GetHDCPAuthenticated();
    tditrace_ex("@T-vlhal_view_GetHDCPAuthenticated()");
    return ret;
}

#if 0
extern "C" void vlhal_refsw_client_EnableDolby(bool enable)
{
    static void (*__vlhal_refsw_client_EnableDolby)(bool) = NULL;

    if (__vlhal_refsw_client_EnableDolby==NULL) {
        __vlhal_refsw_client_EnableDolby = (void (*)(bool))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_EnableDolby) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_EnableDolby() ");
     __vlhal_refsw_client_EnableDolby(enable);
    tditrace_ex("@T-vlhal_refsw_client_EnableDolby()");
}

#if 0 //causing crash in box
extern "C" void *vlhal_refsw_client_GetDisplayHandle(const char *id)
{
    static void *(*__vlhal_refsw_client_GetDisplayHandle)(const char *) = NULL;

    if (__vlhal_refsw_client_GetDisplayHandle==NULL) {
        __vlhal_refsw_client_GetDisplayHandle = (void *(*)(const char *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_GetDisplayHandle) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_GetDisplayHandle() ");
     __vlhal_refsw_client_GetDisplayHandle(id);
    tditrace_ex("@T-vlhal_refsw_client_GetDisplayHandle()");
}
#endif

extern "C" void *vlhal_refsw_client_GetAudioDecoderHandle(const char *id)
{
    static void *(*__vlhal_refsw_client_GetAudioDecoderHandle)(const char *) = NULL;

    if (__vlhal_refsw_client_GetAudioDecoderHandle==NULL) {
        __vlhal_refsw_client_GetAudioDecoderHandle = (void *(*)(const char *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __vlhal_refsw_client_GetAudioDecoderHandle) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+vlhal_refsw_client_GetAudioDecoderHandle() ");
     __vlhal_refsw_client_GetAudioDecoderHandle(id);
    tditrace_ex("@T-vlhal_refsw_client_GetAudioDecoderHandle()");
}
#endif

extern "C" NEXUS_AudioOutput NEXUS_AudioDac_GetConnector(NEXUS_AudioDacHandle handle)
{
    static NEXUS_AudioOutput (*__NEXUS_AudioDac_GetConnector)(NEXUS_AudioDacHandle) = NULL;

    if (__NEXUS_AudioDac_GetConnector == NULL)
    {
       __NEXUS_AudioDac_GetConnector = (NEXUS_AudioOutput (*)(NEXUS_AudioDacHandle))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioDac_GetConnector)
        {
            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioDac_GetConnector()");
    NEXUS_AudioOutput ret = __NEXUS_AudioDac_GetConnector(handle);
    tditrace_ex("@T-NEXUS_AudioDac_GetConnector()");
    return ret;
}


extern "C" void NEXUS_AudioOutput_GetSettings(NEXUS_AudioOutput output, NEXUS_AudioOutputSettings *pSettings)
{
    static void (*__NEXUS_AudioOutput_GetSettings)(NEXUS_AudioOutput, NEXUS_AudioOutputSettings *) = NULL;

    if (__NEXUS_AudioOutput_GetSettings==NULL) {
       __NEXUS_AudioOutput_GetSettings = (void (*)(NEXUS_AudioOutput, NEXUS_AudioOutputSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioOutput_GetSettings) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioOutput_GetSettings()");
    __NEXUS_AudioOutput_GetSettings(output, pSettings);
    tditrace_ex("@T-NEXUS_AudioOutput_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_AudioOutput_SetSettings(NEXUS_AudioOutput output, const NEXUS_AudioOutputSettings *pSettings)
{
    static NEXUS_Error (*__NEXUS_AudioOutput_SetSettings)(NEXUS_AudioOutput, const NEXUS_AudioOutputSettings *) = NULL;

    if (__NEXUS_AudioOutput_SetSettings==NULL) {
       __NEXUS_AudioOutput_SetSettings = (NEXUS_Error (*)(NEXUS_AudioOutput, const NEXUS_AudioOutputSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioOutput_SetSettings) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioOutput_SetSettings()");
    NEXUS_Error ret = __NEXUS_AudioOutput_SetSettings(output, pSettings);
    tditrace_ex("@T-NEXUS_AudioOutput_SetSettings()");
    return ret;
}

extern "C" NEXUS_Error NEXUS_AudioOutput_AddInput(NEXUS_AudioOutput output, NEXUS_AudioInput input)
{
    static NEXUS_Error (*__NEXUS_AudioOutput_AddInput)(NEXUS_AudioOutput, NEXUS_AudioInput) = NULL;

    if (__NEXUS_AudioOutput_AddInput==NULL) {
       __NEXUS_AudioOutput_AddInput = (NEXUS_Error (*)(NEXUS_AudioOutput, NEXUS_AudioInput))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioOutput_AddInput) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioOutput_AddInput()");
    NEXUS_Error ret = __NEXUS_AudioOutput_AddInput(output, input);
    tditrace_ex("@T-NEXUS_AudioOutput_AddInput()");
    return ret;
}

extern "C" NEXUS_Error NEXUS_AudioOutput_RemoveInput(NEXUS_AudioOutput output, NEXUS_AudioInput input)
{
    static NEXUS_Error (*__NEXUS_AudioOutput_RemoveInput)(NEXUS_AudioOutput, NEXUS_AudioInput) = NULL;

    if (__NEXUS_AudioOutput_RemoveInput==NULL) {
       __NEXUS_AudioOutput_RemoveInput = (NEXUS_Error (*)(NEXUS_AudioOutput, NEXUS_AudioInput))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_AudioOutput_RemoveInput) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_AudioOutput_RemoveInput()");
    NEXUS_Error ret = __NEXUS_AudioOutput_RemoveInput(output, input);
    tditrace_ex("@T-NEXUS_AudioOutput_RemoveInput()");
    return ret;
}

extern "C" NEXUS_Error NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioOutput output)
{
   static NEXUS_Error (*__NEXUS_AudioOutput_RemoveAllInputs)(NEXUS_AudioOutput) = NULL;

       if (__NEXUS_AudioOutput_RemoveAllInputs==NULL) {
          __NEXUS_AudioOutput_RemoveAllInputs = (NEXUS_Error (*)(NEXUS_AudioOutput))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_AudioOutput_RemoveAllInputs) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_AudioOutput_RemoveAllInputs()");
       NEXUS_Error ret = __NEXUS_AudioOutput_RemoveAllInputs(output);
       tditrace_ex("@T-NEXUS_AudioOutput_RemoveAllInputs()");
       return ret;
}

extern "C" void NEXUS_AudioOutput_Shutdown(NEXUS_AudioOutput output)
{
   static void (*__NEXUS_AudioOutput_Shutdown)(NEXUS_AudioOutput) = NULL;

       if (__NEXUS_AudioOutput_Shutdown==NULL) {
          __NEXUS_AudioOutput_Shutdown = (void (*)(NEXUS_AudioOutput))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_AudioOutput_Shutdown) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_AudioOutput_Shutdown()");
       __NEXUS_AudioOutput_Shutdown(output);
       tditrace_ex("@T-NEXUS_AudioOutput_Shutdown()");
}

extern "C" NEXUS_Error NEXUS_AudioOutput_GetStatus(NEXUS_AudioOutput output, NEXUS_AudioOutputStatus *pStatus)
{
   static NEXUS_Error (*__NEXUS_AudioOutput_GetStatus)(NEXUS_AudioOutput, NEXUS_AudioOutputStatus *) = NULL;

       if (__NEXUS_AudioOutput_GetStatus==NULL) {
          __NEXUS_AudioOutput_GetStatus = (NEXUS_Error (*)(NEXUS_AudioOutput, NEXUS_AudioOutputStatus *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_AudioOutput_GetStatus) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_AudioOutput_GetStatus()");
       NEXUS_Error ret = __NEXUS_AudioOutput_GetStatus(output, pStatus);
       tditrace_ex("@T-NEXUS_AudioOutput_GetStatus()");
       return ret;
}

extern "C" NEXUS_AudioOutput NEXUS_HdmiOutput_GetAudioConnector(NEXUS_HdmiOutputHandle output)
{
   static NEXUS_AudioOutput (*__NEXUS_HdmiOutput_GetAudioConnector)(NEXUS_HdmiOutputHandle) = NULL;

       if (__NEXUS_HdmiOutput_GetAudioConnector==NULL) {
          __NEXUS_HdmiOutput_GetAudioConnector = (NEXUS_AudioOutput (*)(NEXUS_HdmiOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetAudioConnector) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetAudioConnector()");
       NEXUS_AudioOutput ret = __NEXUS_HdmiOutput_GetAudioConnector(output);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetAudioConnector()");
       return ret;
}

extern "C" NEXUS_AudioOutput NEXUS_SpdifOutput_GetConnector(NEXUS_SpdifOutputHandle handle)
{
   static NEXUS_AudioOutput (*__NEXUS_SpdifOutput_GetConnector)(NEXUS_SpdifOutputHandle) = NULL;

       if (__NEXUS_SpdifOutput_GetConnector==NULL) {
          __NEXUS_SpdifOutput_GetConnector = (NEXUS_AudioOutput (*)(NEXUS_SpdifOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SpdifOutput_GetConnector) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SpdifOutput_GetConnector()");
       NEXUS_AudioOutput ret = __NEXUS_SpdifOutput_GetConnector(handle);
       tditrace_ex("@T-NEXUS_SpdifOutput_GetConnector()");
       return ret;
}

#if 0
extern "C" void NEXUS_Cec_GetSettings(NEXUS_CecHandle handle, NEXUS_CecSettings *pSettings)
{
   static void (*__NEXUS_Cec_GetSettings)(NEXUS_CecHandle, NEXUS_CecSettings *) = NULL;

       if (__NEXUS_Cec_GetSettings==NULL) {
          __NEXUS_Cec_GetSettings = (void (*)(NEXUS_CecHandle, NEXUS_CecSettings * ))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Cec_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Cec_GetSettings()");
       __NEXUS_Cec_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_Cec_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_Cec_SetSettings(NEXUS_CecHandle handle, const NEXUS_CecSettings *pSettings)
{
   static NEXUS_Error (*__NEXUS_Cec_SetSettings)(NEXUS_CecHandle, const NEXUS_CecSettings * ) = NULL;

       if (__NEXUS_Cec_SetSettings==NULL) {
          __NEXUS_Cec_SetSettings = (NEXUS_Error (*)(NEXUS_CecHandle, const NEXUS_CecSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Cec_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Cec_SetSettings()");
       NEXUS_Error ret = __NEXUS_Cec_SetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_Cec_SetSettings()");
       return ret;
}

extern "C" NEXUS_CecHandle NEXUS_Cec_Open(unsigned index, const NEXUS_CecSettings *pSettings)
{
   static NEXUS_CecHandle (*__NEXUS_Cec_Open)(uint32_t, const NEXUS_CecSettings * ) = NULL;

       if (__NEXUS_Cec_Open==NULL) {
          __NEXUS_Cec_Open = (NEXUS_CecHandle (*)(uint32_t, const NEXUS_CecSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Cec_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Cec_Open()");
       NEXUS_CecHandle ret = __NEXUS_Cec_Open(index, pSettings);
       tditrace_ex("@T-NEXUS_Cec_Open()");
       return ret;
}

extern "C" void NEXUS_Cec_Close(NEXUS_CecHandle handle)
{
   static void (*__NEXUS_Cec_Close)(NEXUS_CecHandle) = NULL;

       if (__NEXUS_Cec_Close==NULL) {
          __NEXUS_Cec_Close = (void (*)(NEXUS_CecHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Cec_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Cec_Close()");
       __NEXUS_Cec_Close(handle);
       tditrace_ex("@T-NEXUS_Cec_Close()");
}

extern "C" NEXUS_Error NEXUS_Cec_GetStatus(NEXUS_CecHandle handle, NEXUS_CecStatus *pStatus)
{
   static NEXUS_Error (*__NEXUS_Cec_GetStatus)(NEXUS_CecHandle, NEXUS_CecStatus *) = NULL;

       if (__NEXUS_Cec_GetStatus==NULL) {
          __NEXUS_Cec_GetStatus = (NEXUS_Error (*)(NEXUS_CecHandle, NEXUS_CecStatus *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Cec_GetStatus) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Cec_GetStatus()");
       NEXUS_Error ret = __NEXUS_Cec_GetStatus(handle, pStatus);
       tditrace_ex("@T-NEXUS_Cec_GetStatus()");
       return ret;
}


extern "C" NEXUS_Error NEXUS_Cec_ReceiveMessage(NEXUS_CecHandle handle, NEXUS_CecReceivedMessage *pReceivedMessage)
{
   static NEXUS_Error (*__NEXUS_Cec_ReceiveMessage)(NEXUS_CecHandle, NEXUS_CecReceivedMessage *) = NULL;

       if (__NEXUS_Cec_ReceiveMessage==NULL) {
          __NEXUS_Cec_ReceiveMessage = (NEXUS_Error (*)(NEXUS_CecHandle, NEXUS_CecReceivedMessage *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Cec_ReceiveMessage) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Cec_ReceiveMessage()");
       NEXUS_Error ret = __NEXUS_Cec_ReceiveMessage(handle, pReceivedMessage);
       tditrace_ex("@T-NEXUS_Cec_ReceiveMessage()");
       return ret;
}
#endif

#if 0
extern "C" NEXUS_VideoOutput NEXUS_ComponentOutput_GetConnector(NEXUS_ComponentOutputHandle output)
{
   static NEXUS_VideoOutput (*__NEXUS_ComponentOutput_GetConnector)(NEXUS_ComponentOutputHandle) = NULL;

       if (__NEXUS_ComponentOutput_GetConnector==NULL) {
          __NEXUS_ComponentOutput_GetConnector = (NEXUS_VideoOutput (*)(NEXUS_ComponentOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_ComponentOutput_GetConnector) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_ComponentOutput_GetConnector()");
       NEXUS_VideoOutput ret = __NEXUS_ComponentOutput_GetConnector(output);
       tditrace_ex("@T-NEXUS_ComponentOutput_GetConnector()");
       return ret;
}

extern "C" void NEXUS_ComponentOutput_GetSettings(NEXUS_ComponentOutputHandle output, NEXUS_ComponentOutputSettings *pSettings)
{
   static void (*__NEXUS_ComponentOutput_GetSettings)(NEXUS_ComponentOutputHandle, NEXUS_ComponentOutputSettings *) = NULL;

       if (__NEXUS_ComponentOutput_GetSettings==NULL) {
          __NEXUS_ComponentOutput_GetSettings = (void (*)(NEXUS_ComponentOutputHandle, NEXUS_ComponentOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_ComponentOutput_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_ComponentOutput_GetSettings()");
       __NEXUS_ComponentOutput_GetSettings(output, pSettings);
       tditrace_ex("@T-NEXUS_ComponentOutput_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_ComponentOutput_SetSettings(NEXUS_ComponentOutputHandle output, const NEXUS_ComponentOutputSettings *pSettings)
{
   static NEXUS_Error (*__NEXUS_ComponentOutput_SetSettings)(NEXUS_ComponentOutputHandle,const NEXUS_ComponentOutputSettings *) = NULL;

       if (__NEXUS_ComponentOutput_SetSettings==NULL) {
          __NEXUS_ComponentOutput_SetSettings = (NEXUS_Error (*)(NEXUS_ComponentOutputHandle, const NEXUS_ComponentOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_ComponentOutput_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_ComponentOutput_SetSettings()");
       NEXUS_Error ret = __NEXUS_ComponentOutput_SetSettings(output, pSettings);
       tditrace_ex("@T-NEXUS_ComponentOutput_SetSettings()");
       return ret;
}

extern "C" NEXUS_VideoOutput NEXUS_CompositeOutput_GetConnector(NEXUS_CompositeOutputHandle composite)
{
   static NEXUS_VideoOutput (*__NEXUS_CompositeOutput_GetConnector)(NEXUS_CompositeOutputHandle) = NULL;

       if (__NEXUS_CompositeOutput_GetConnector==NULL) {
          __NEXUS_CompositeOutput_GetConnector = (NEXUS_VideoOutput (*)(NEXUS_CompositeOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_CompositeOutput_GetConnector) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_CompositeOutput_GetConnector()");
       NEXUS_VideoOutput ret = __NEXUS_CompositeOutput_GetConnector(composite);
       tditrace_ex("@T-NEXUS_CompositeOutput_GetConnector()");
       return ret;
}
#endif

#if 0
extern "C" void NEXUS_StartCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
{
   static void (*__NEXUS_StartCallbacks_tagged)(void *,const char *, uint32_t, const char *) = NULL;

       if (__NEXUS_StartCallbacks_tagged==NULL) {
          __NEXUS_StartCallbacks_tagged = (void (*)(void *,const char *, uint32_t, const char *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StartCallbacks_tagged) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StartCallbacks_tagged()");
       __NEXUS_StartCallbacks_tagged(interfaceHandle, pFileName, lineNumber, pFunctionName);
       tditrace_ex("@T-NEXUS_StartCallbacks_tagged()");
}

extern "C" void NEXUS_StopCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
{
   static void (*__NEXUS_StopCallbacks_tagged)(void *,const char *, uint32_t, const char *) = NULL;

       if (__NEXUS_StopCallbacks_tagged==NULL) {
          __NEXUS_StopCallbacks_tagged = (void (*)(void *,const char *, uint32_t, const char *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StopCallbacks_tagged) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StopCallbacks_tagged()");
       __NEXUS_StopCallbacks_tagged(interfaceHandle, pFileName, lineNumber, pFunctionName);
       tditrace_ex("@T-NEXUS_StopCallbacks_tagged()");
}
#endif

extern "C" void NEXUS_VideoFormat_GetInfo(NEXUS_VideoFormat videoFmt, NEXUS_VideoFormatInfo *pVideoFmtInfo)
{
   static void (*__NEXUS_VideoFormat_GetInfo)(NEXUS_VideoFormat, NEXUS_VideoFormatInfo *) = NULL;

       if (__NEXUS_VideoFormat_GetInfo==NULL) {
          __NEXUS_VideoFormat_GetInfo = (void (*)(NEXUS_VideoFormat, NEXUS_VideoFormatInfo *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoFormat_GetInfo) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoFormat_GetInfo()");
       __NEXUS_VideoFormat_GetInfo(videoFmt, pVideoFmtInfo);
       tditrace_ex("@T-NEXUS_VideoFormat_GetInfo()");
}


#if 0
extern "C" void NEXUS_Display_GetDefaultSettings(NEXUS_DisplaySettings *pSettings)
{
   static void (*__NEXUS_Display_GetDefaultSettings)(NEXUS_DisplaySettings *) = NULL;

       if (__NEXUS_Display_GetDefaultSettings==NULL) {
          __NEXUS_Display_GetDefaultSettings = (void (*)(NEXUS_DisplaySettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_GetDefaultSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_GetDefaultSettings()");
       __NEXUS_Display_GetDefaultSettings(pSettings);
       tditrace_ex("@T-NEXUS_Display_GetDefaultSettings()");
}

extern "C" NEXUS_DisplayHandle NEXUS_Display_Open(unsigned displayIndex, const NEXUS_DisplaySettings *pSettings)
{
   static NEXUS_DisplayHandle (*__NEXUS_Display_Open)(uint32_t, const NEXUS_DisplaySettings *) = NULL;

       if (__NEXUS_Display_Open==NULL) {
          __NEXUS_Display_Open = (NEXUS_DisplayHandle (*)(uint32_t, const NEXUS_DisplaySettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_Open()");

       NEXUS_DisplayHandle ret = __NEXUS_Display_Open(displayIndex,pSettings);
       tditrace_ex("@T-NEXUS_Display_Open()");
       return ret;
}

extern "C" void NEXUS_Display_Close(NEXUS_DisplayHandle display)
{
   static void (*__NEXUS_Display_Close)(NEXUS_DisplayHandle) = NULL;

       if (__NEXUS_Display_Close==NULL) {
          __NEXUS_Display_Close = (void (*)(NEXUS_DisplayHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_Close()");
       __NEXUS_Display_Close(display);
       tditrace_ex("@T-NEXUS_Display_Close()");
}

extern "C" void NEXUS_Display_GetSettings(NEXUS_DisplayHandle display, NEXUS_DisplaySettings *pSettings)
{
   static void (*__NEXUS_Display_GetSettings)(NEXUS_DisplayHandle, NEXUS_DisplaySettings *) = NULL;

       if (__NEXUS_Display_GetSettings==NULL) {
          __NEXUS_Display_GetSettings = (void (*)(NEXUS_DisplayHandle, NEXUS_DisplaySettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_GetSettings()");
       __NEXUS_Display_GetSettings(display, pSettings);
       tditrace_ex("@T-NEXUS_Display_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_Display_SetSettings(NEXUS_DisplayHandle display, const NEXUS_DisplaySettings *pSettings)
{
   static NEXUS_Error (*__NEXUS_Display_SetSettings)(NEXUS_DisplayHandle, const NEXUS_DisplaySettings *) = NULL;

       if (__NEXUS_Display_SetSettings==NULL) {
          __NEXUS_Display_SetSettings = (NEXUS_Error (*)(NEXUS_DisplayHandle, const NEXUS_DisplaySettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_SetSettings()");
       NEXUS_Error ret = __NEXUS_Display_SetSettings(display, pSettings);
       tditrace_ex("@T-NEXUS_Display_SetSettings()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode updateMode)
{
   static NEXUS_Error (*__NEXUS_DisplayModule_SetUpdateMode)(NEXUS_DisplayUpdateMode) = NULL;

       if (__NEXUS_DisplayModule_SetUpdateMode==NULL) {
          __NEXUS_DisplayModule_SetUpdateMode = (NEXUS_Error (*)(NEXUS_DisplayUpdateMode))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_DisplayModule_SetUpdateMode) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_DisplayModule_SetUpdateMode()");
       NEXUS_Error ret = __NEXUS_DisplayModule_SetUpdateMode(updateMode);
       tditrace_ex("@T-NEXUS_DisplayModule_SetUpdateMode()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Display_AddOutput(NEXUS_DisplayHandle display, NEXUS_VideoOutput output)
{
   static NEXUS_Error (*__NEXUS_Display_AddOutput)(NEXUS_DisplayHandle ,NEXUS_VideoOutput) = NULL;

       if (__NEXUS_Display_AddOutput==NULL) {
          __NEXUS_Display_AddOutput = (NEXUS_Error (*)(NEXUS_DisplayHandle ,NEXUS_VideoOutput))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_AddOutput) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_AddOutput()");
       NEXUS_Error ret = __NEXUS_Display_AddOutput(display, output);
       tditrace_ex("@T-NEXUS_Display_AddOutput()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Display_RemoveOutput(NEXUS_DisplayHandle display, NEXUS_VideoOutput output)
{
   static NEXUS_Error (*__NEXUS_Display_RemoveOutput)(NEXUS_DisplayHandle ,NEXUS_VideoOutput) = NULL;

       if (__NEXUS_Display_RemoveOutput==NULL) {
          __NEXUS_Display_RemoveOutput = (NEXUS_Error (*)(NEXUS_DisplayHandle ,NEXUS_VideoOutput))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_RemoveOutput) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_RemoveOutput()");
       NEXUS_Error ret = __NEXUS_Display_RemoveOutput(display, output);
       tditrace_ex("@T-NEXUS_Display_RemoveOutput()");
       return ret;
}

extern "C" void NEXUS_Display_GetGraphicsSettings(NEXUS_DisplayHandle display, NEXUS_GraphicsSettings *pSettings)
{
   static void (*__NEXUS_Display_GetGraphicsSettings)(NEXUS_DisplayHandle ,NEXUS_GraphicsSettings *) = NULL;

       if (__NEXUS_Display_GetGraphicsSettings==NULL) {
          __NEXUS_Display_GetGraphicsSettings = (void (*)(NEXUS_DisplayHandle ,NEXUS_GraphicsSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_GetGraphicsSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_GetGraphicsSettings()");
       __NEXUS_Display_GetGraphicsSettings(display, pSettings);
       tditrace_ex("@T-NEXUS_Display_GetGraphicsSettings()");
}

extern "C" NEXUS_Error NEXUS_Display_SetGraphicsSettings(NEXUS_DisplayHandle display, const NEXUS_GraphicsSettings *pSettings)
{
   static NEXUS_Error (*__NEXUS_Display_SetGraphicsSettings)(NEXUS_DisplayHandle ,const NEXUS_GraphicsSettings *) = NULL;

       if (__NEXUS_Display_SetGraphicsSettings==NULL) {
          __NEXUS_Display_SetGraphicsSettings = (NEXUS_Error (*)(NEXUS_DisplayHandle ,const NEXUS_GraphicsSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_SetGraphicsSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_SetGraphicsSettings()");
       NEXUS_Error ret = __NEXUS_Display_SetGraphicsSettings(display, pSettings);
       tditrace_ex("@T-NEXUS_Display_SetGraphicsSettings()");
       return ret;
}

extern "C" void NEXUS_Display_GetVbiSettings(NEXUS_DisplayHandle handle, NEXUS_DisplayVbiSettings *pSettings)
{
   static void (*__NEXUS_Display_GetVbiSettings)(NEXUS_DisplayHandle ,NEXUS_DisplayVbiSettings *) = NULL;

       if (__NEXUS_Display_GetVbiSettings==NULL) {
          __NEXUS_Display_GetVbiSettings = (void (*)(NEXUS_DisplayHandle ,NEXUS_DisplayVbiSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_GetVbiSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_GetVbiSettings()");
       __NEXUS_Display_GetVbiSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_Display_GetVbiSettings()");
}

extern "C" NEXUS_Error NEXUS_Display_SetVbiSettings(NEXUS_DisplayHandle handle, const NEXUS_DisplayVbiSettings *pSettings)
{
   static NEXUS_Error (*__NEXUS_Display_SetVbiSettings)(NEXUS_DisplayHandle ,const NEXUS_DisplayVbiSettings *) = NULL;

       if (__NEXUS_Display_SetVbiSettings==NULL) {
          __NEXUS_Display_SetVbiSettings = (NEXUS_Error (*)(NEXUS_DisplayHandle ,const NEXUS_DisplayVbiSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_SetVbiSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_SetVbiSettings()");
       NEXUS_Error ret = __NEXUS_Display_SetVbiSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_Display_SetVbiSettings()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Display_SetMacrovision(NEXUS_DisplayHandle handle, NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable)
{
   static NEXUS_Error (*__NEXUS_Display_SetMacrovision)(NEXUS_DisplayHandle ,NEXUS_DisplayMacrovisionType, const NEXUS_DisplayMacrovisionTables *) = NULL;

       if (__NEXUS_Display_SetMacrovision==NULL) {
          __NEXUS_Display_SetMacrovision = (NEXUS_Error (*)(NEXUS_DisplayHandle ,NEXUS_DisplayMacrovisionType, const NEXUS_DisplayMacrovisionTables *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_SetMacrovision) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_SetMacrovision()");
       NEXUS_Error ret = __NEXUS_Display_SetMacrovision(handle, type, pTable);
       tditrace_ex("@T-NEXUS_Display_SetMacrovision()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Display_SetCgms(NEXUS_DisplayHandle handle, uint32_t cgmsData)
{
   static NEXUS_Error (*__NEXUS_Display_SetCgms)(NEXUS_DisplayHandle ,uint32_t) = NULL;

       if (__NEXUS_Display_SetCgms==NULL) {
          __NEXUS_Display_SetCgms = (NEXUS_Error (*)(NEXUS_DisplayHandle ,uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_SetCgms) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_SetCgms()");
       NEXUS_Error ret = __NEXUS_Display_SetCgms(handle, cgmsData);
       tditrace_ex("@T-NEXUS_Display_SetCgms()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Display_SetWss(NEXUS_DisplayHandle handle, uint16_t wssData)
{
   static NEXUS_Error (*__NEXUS_Display_SetWss)(NEXUS_DisplayHandle ,uint16_t) = NULL;

       if (__NEXUS_Display_SetWss==NULL) {
          __NEXUS_Display_SetWss = (NEXUS_Error (*)(NEXUS_DisplayHandle ,uint16_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Display_SetWss) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Display_SetWss()");
       NEXUS_Error ret = __NEXUS_Display_SetWss(handle, wssData);
       tditrace_ex("@T-NEXUS_Display_SetWss()");
       return ret;
}
#endif

#if 0
extern "C" NEXUS_Error NEXUS_Frontend_GetFastStatus(NEXUS_FrontendHandle handle, NEXUS_FrontendFastStatus *pStatus)
{
   static NEXUS_Error (*__NEXUS_Frontend_GetFastStatus)(NEXUS_FrontendHandle ,NEXUS_FrontendFastStatus *) = NULL;

       if (__NEXUS_Frontend_GetFastStatus==NULL) {
          __NEXUS_Frontend_GetFastStatus = (NEXUS_Error (*)(NEXUS_FrontendHandle ,NEXUS_FrontendFastStatus *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Frontend_GetFastStatus) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Frontend_GetFastStatus()");
       NEXUS_Error ret = __NEXUS_Frontend_GetFastStatus(handle, pStatus);
       tditrace_ex("@T-NEXUS_Frontend_GetFastStatus()");
       return ret;
}
#endif

extern "C" void NEXUS_Frontend_Untune(NEXUS_FrontendHandle handle)
{
   static void (*__NEXUS_Frontend_Untune)(NEXUS_FrontendHandle) = NULL;

       if (__NEXUS_Frontend_Untune==NULL) {
          __NEXUS_Frontend_Untune = (void (*)(NEXUS_FrontendHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Frontend_Untune) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Frontend_Untune()");
       __NEXUS_Frontend_Untune(handle);
       tditrace_ex("@T-NEXUS_Frontend_Untune()");
}

#if 0
extern "C" void NEXUS_Frontend_GetCapabilities(NEXUS_FrontendHandle handle, NEXUS_FrontendCapabilities *pCapabilities)
{
   static void (*__NEXUS_Frontend_GetCapabilities)(NEXUS_FrontendHandle, NEXUS_FrontendCapabilities *) = NULL;

       if (__NEXUS_Frontend_GetCapabilities==NULL) {
          __NEXUS_Frontend_GetCapabilities = (void (*)(NEXUS_FrontendHandle, NEXUS_FrontendCapabilities *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Frontend_GetCapabilities) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Frontend_GetCapabilities()");
       __NEXUS_Frontend_GetCapabilities(handle, pCapabilities);
       tditrace_ex("@T-NEXUS_Frontend_GetCapabilities()");
}
#endif


#if 0
extern "C" void NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType type, NEXUS_GpioSettings *pSettings)
{
   static void (*__NEXUS_Gpio_GetDefaultSettings)(NEXUS_GpioType, NEXUS_GpioSettings *) = NULL;

       if (__NEXUS_Gpio_GetDefaultSettings==NULL) {
          __NEXUS_Gpio_GetDefaultSettings = (void (*)(NEXUS_GpioType, NEXUS_GpioSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Gpio_GetDefaultSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Gpio_GetDefaultSettings()");
       __NEXUS_Gpio_GetDefaultSettings(type, pSettings);
       tditrace_ex("@T-NEXUS_Gpio_GetDefaultSettings()");
}

extern "C" NEXUS_GpioHandle NEXUS_Gpio_OpenAux(unsigned typeAndPin, const NEXUS_GpioSettings *pSettings)
{
   static NEXUS_GpioHandle (*__NEXUS_Gpio_OpenAux)(uint32_t ,const NEXUS_GpioSettings *) = NULL;

       if (__NEXUS_Gpio_OpenAux==NULL) {
          __NEXUS_Gpio_OpenAux = (NEXUS_GpioHandle (*)(uint32_t ,const NEXUS_GpioSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Gpio_OpenAux) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Gpio_OpenAux()");
       NEXUS_GpioHandle ret = __NEXUS_Gpio_OpenAux(typeAndPin, pSettings);
       tditrace_ex("@T-NEXUS_Gpio_OpenAux()");
       return ret;
}

extern "C" void NEXUS_Gpio_Close(NEXUS_GpioHandle handle)
{
   static void (*__NEXUS_Gpio_Close)(NEXUS_GpioHandle) = NULL;

       if (__NEXUS_Gpio_Close==NULL) {
          __NEXUS_Gpio_Close = (void (*)(NEXUS_GpioHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Gpio_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Gpio_Close()");
       __NEXUS_Gpio_Close(handle);
       tditrace_ex("@T-NEXUS_Gpio_Close()");
}
#endif

#if 0
extern "C" void NEXUS_Graphics2D_GetDefaultFillSettings(NEXUS_Graphics2DFillSettings *pSettings)
{
   static void (*__NEXUS_Graphics2D_GetDefaultFillSettings)(NEXUS_Graphics2DFillSettings *) = NULL;

       if (__NEXUS_Graphics2D_GetDefaultFillSettings==NULL) {
          __NEXUS_Graphics2D_GetDefaultFillSettings = (void (*)(NEXUS_Graphics2DFillSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_GetDefaultFillSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_GetDefaultFillSettings()");
       __NEXUS_Graphics2D_GetDefaultFillSettings(pSettings);
       tditrace_ex("@T-NEXUS_Graphics2D_GetDefaultFillSettings()");
}

extern "C" NEXUS_Graphics2DHandle NEXUS_Graphics2D_Open(unsigned index, const NEXUS_Graphics2DOpenSettings *pSettings)
{
   static NEXUS_Graphics2DHandle (*__NEXUS_Graphics2D_Open)(uint32_t, const NEXUS_Graphics2DOpenSettings *) = NULL;

       if (__NEXUS_Graphics2D_Open==NULL) {
          __NEXUS_Graphics2D_Open = (NEXUS_Graphics2DHandle (*)(uint32_t, const NEXUS_Graphics2DOpenSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_Open()");
       NEXUS_Graphics2DHandle ret = __NEXUS_Graphics2D_Open(index, pSettings);
       tditrace_ex("@T-NEXUS_Graphics2D_Open()");
       return ret;
}

extern "C" void NEXUS_Graphics2D_Close(NEXUS_Graphics2DHandle handle)
{
   static void(*__NEXUS_Graphics2D_Close)(NEXUS_Graphics2DHandle) = NULL;

       if (__NEXUS_Graphics2D_Close==NULL) {
          __NEXUS_Graphics2D_Close = (void (*)(NEXUS_Graphics2DHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_Close()");
       __NEXUS_Graphics2D_Close(handle);
       tditrace_ex("@T-NEXUS_Graphics2D_Close()");
}

extern "C" void NEXUS_Graphics2D_GetSettings(NEXUS_Graphics2DHandle handle, NEXUS_Graphics2DSettings *pSettings)
{
   static void (*__NEXUS_Graphics2D_GetSettings)(NEXUS_Graphics2DHandle, NEXUS_Graphics2DSettings *) = NULL;

       if (__NEXUS_Graphics2D_GetSettings==NULL) {
          __NEXUS_Graphics2D_GetSettings = (void (*)(NEXUS_Graphics2DHandle, NEXUS_Graphics2DSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_GetSettings()");
       __NEXUS_Graphics2D_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_Graphics2D_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_Graphics2D_SetSettings(NEXUS_Graphics2DHandle handle, const NEXUS_Graphics2DSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_Graphics2D_SetSettings)(NEXUS_Graphics2DHandle, const NEXUS_Graphics2DSettings *) = NULL;

       if (__NEXUS_Graphics2D_SetSettings==NULL) {
          __NEXUS_Graphics2D_SetSettings = (NEXUS_Error (*)(NEXUS_Graphics2DHandle, const NEXUS_Graphics2DSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_SetSettings()");
       NEXUS_Error ret = __NEXUS_Graphics2D_SetSettings(handle,pSettings);
       tditrace_ex("@T-NEXUS_Graphics2D_SetSettings()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Graphics2D_Fill(NEXUS_Graphics2DHandle handle, const NEXUS_Graphics2DFillSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_Graphics2D_Fill)(NEXUS_Graphics2DHandle, const NEXUS_Graphics2DFillSettings *) = NULL;

       if (__NEXUS_Graphics2D_Fill==NULL) {
          __NEXUS_Graphics2D_Fill = (NEXUS_Error (*)(NEXUS_Graphics2DHandle, const NEXUS_Graphics2DFillSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_Fill) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_Fill()");
       NEXUS_Error ret = __NEXUS_Graphics2D_Fill(handle,pSettings);
       tditrace_ex("@T-NEXUS_Graphics2D_Fill()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Graphics2D_Blit(NEXUS_Graphics2DHandle handle, const NEXUS_Graphics2DBlitSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_Graphics2D_Blit)(NEXUS_Graphics2DHandle, const NEXUS_Graphics2DBlitSettings *) = NULL;

       if (__NEXUS_Graphics2D_Blit==NULL) {
          __NEXUS_Graphics2D_Blit = (NEXUS_Error (*)(NEXUS_Graphics2DHandle, const NEXUS_Graphics2DBlitSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_Blit) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_Blit()");
       NEXUS_Error ret = __NEXUS_Graphics2D_Blit(handle,pSettings);
       tditrace_ex("@T-NEXUS_Graphics2D_Blit()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Graphics2D_Checkpoint(NEXUS_Graphics2DHandle handle, const NEXUS_CallbackDesc *pLegacyCallback)
{
   static NEXUS_Error(*__NEXUS_Graphics2D_Checkpoint)(NEXUS_Graphics2DHandle, const NEXUS_CallbackDesc *) = NULL;

       if (__NEXUS_Graphics2D_Checkpoint==NULL) {
          __NEXUS_Graphics2D_Checkpoint = (NEXUS_Error (*)(NEXUS_Graphics2DHandle, const NEXUS_CallbackDesc *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Graphics2D_Checkpoint) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Graphics2D_Checkpoint()");
       NEXUS_Error ret = __NEXUS_Graphics2D_Checkpoint(handle,pLegacyCallback);
       tditrace_ex("@T-NEXUS_Graphics2D_Checkpoint()");
       return ret;
}
#endif

#if 0
extern "C" NEXUS_VideoOutput NEXUS_HdmiOutput_GetVideoConnector(NEXUS_HdmiOutputHandle output)
{
   static NEXUS_VideoOutput(*__NEXUS_HdmiOutput_GetVideoConnector)(NEXUS_HdmiOutputHandle) = NULL;

       if (__NEXUS_HdmiOutput_GetVideoConnector==NULL) {
          __NEXUS_HdmiOutput_GetVideoConnector = (NEXUS_VideoOutput (*)(NEXUS_HdmiOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetVideoConnector) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetVideoConnector()");
       NEXUS_VideoOutput ret = __NEXUS_HdmiOutput_GetVideoConnector(output);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetVideoConnector()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_GetBasicEdidData(NEXUS_HdmiOutputHandle output, NEXUS_HdmiOutputBasicEdidData *pData)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_GetBasicEdidData)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputBasicEdidData *) = NULL;

       if (__NEXUS_HdmiOutput_GetBasicEdidData==NULL) {
          __NEXUS_HdmiOutput_GetBasicEdidData = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputBasicEdidData *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetBasicEdidData) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetBasicEdidData()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_GetBasicEdidData(output,pData);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetBasicEdidData()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_GetEdidBlock(NEXUS_HdmiOutputHandle output, unsigned blockNum, NEXUS_HdmiOutputEdidBlock *pBlock)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_GetEdidBlock)(NEXUS_HdmiOutputHandle, uint32_t, NEXUS_HdmiOutputEdidBlock *) = NULL;

       if (__NEXUS_HdmiOutput_GetEdidBlock==NULL) {
          __NEXUS_HdmiOutput_GetEdidBlock = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle,uint32_t, NEXUS_HdmiOutputEdidBlock *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetEdidBlock) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetEdidBlock()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_GetEdidBlock(output,blockNum,pBlock);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetEdidBlock()");
       return ret;
}

extern "C" void NEXUS_HdmiOutput_GetSettings(NEXUS_HdmiOutputHandle output, NEXUS_HdmiOutputSettings *pSettings)
{
   static void (*__NEXUS_HdmiOutput_GetSettings)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputSettings *) = NULL;

       if (__NEXUS_HdmiOutput_GetSettings==NULL) {
          __NEXUS_HdmiOutput_GetSettings = (void (*)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetSettings()");
       __NEXUS_HdmiOutput_GetSettings(output, pSettings);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_SetSettings(NEXUS_HdmiOutputHandle output, const NEXUS_HdmiOutputSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_SetSettings)(NEXUS_HdmiOutputHandle, const NEXUS_HdmiOutputSettings *) = NULL;

       if (__NEXUS_HdmiOutput_SetSettings==NULL) {
          __NEXUS_HdmiOutput_SetSettings = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle, const NEXUS_HdmiOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_SetSettings()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_SetSettings(output,pSettings);
       tditrace_ex("@T-NEXUS_HdmiOutput_SetSettings()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_GetStatus(NEXUS_HdmiOutputHandle output, NEXUS_HdmiOutputStatus *pStatus)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_GetStatus)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputStatus *) = NULL;

       if (__NEXUS_HdmiOutput_GetStatus==NULL) {
          __NEXUS_HdmiOutput_GetStatus = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputStatus *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetStatus) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetStatus()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_GetStatus(output,pStatus);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetStatus()");
       return ret;
}

extern "C" void NEXUS_HdmiOutput_GetHdcpSettings(NEXUS_HdmiOutputHandle handle, NEXUS_HdmiOutputHdcpSettings *pSettings)
{
   static void(*__NEXUS_HdmiOutput_GetHdcpSettings)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputHdcpSettings *) = NULL;

       if (__NEXUS_HdmiOutput_GetHdcpSettings==NULL) {
          __NEXUS_HdmiOutput_GetHdcpSettings = (void (*)(NEXUS_HdmiOutputHandle, NEXUS_HdmiOutputHdcpSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_GetHdcpSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_GetHdcpSettings()");
       __NEXUS_HdmiOutput_GetHdcpSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_HdmiOutput_GetHdcpSettings()");
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_SetHdcpSettings(NEXUS_HdmiOutputHandle handle, const NEXUS_HdmiOutputHdcpSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_SetHdcpSettings)(NEXUS_HdmiOutputHandle, const NEXUS_HdmiOutputHdcpSettings *) = NULL;

       if (__NEXUS_HdmiOutput_SetHdcpSettings==NULL) {
          __NEXUS_HdmiOutput_SetHdcpSettings = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle, const NEXUS_HdmiOutputHdcpSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_SetHdcpSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_SetHdcpSettings()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_SetHdcpSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_HdmiOutput_SetHdcpSettings()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_SetHdcpRevokedKsvs(NEXUS_HdmiOutputHandle handle, const NEXUS_HdmiOutputHdcpKsv *pRevokedKsvs, uint16_t numKsvs)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_SetHdcpRevokedKsvs)(NEXUS_HdmiOutputHandle, const NEXUS_HdmiOutputHdcpKsv *, uint16_t) = NULL;

       if (__NEXUS_HdmiOutput_SetHdcpRevokedKsvs==NULL) {
          __NEXUS_HdmiOutput_SetHdcpRevokedKsvs = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle, const NEXUS_HdmiOutputHdcpKsv *, uint16_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_SetHdcpRevokedKsvs) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_SetHdcpRevokedKsvs()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_SetHdcpRevokedKsvs(handle, pRevokedKsvs, numKsvs);
       tditrace_ex("@T-NEXUS_HdmiOutput_SetHdcpRevokedKsvs()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_StartHdcpAuthentication(NEXUS_HdmiOutputHandle handle)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_StartHdcpAuthentication)(NEXUS_HdmiOutputHandle) = NULL;

       if (__NEXUS_HdmiOutput_StartHdcpAuthentication==NULL) {
          __NEXUS_HdmiOutput_StartHdcpAuthentication = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_StartHdcpAuthentication) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_StartHdcpAuthentication()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_StartHdcpAuthentication(handle);
       tditrace_ex("@T-NEXUS_HdmiOutput_StartHdcpAuthentication()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_HdmiOutput_DisableHdcpAuthentication(NEXUS_HdmiOutputHandle handle)
{
   static NEXUS_Error(*__NEXUS_HdmiOutput_DisableHdcpAuthentication)(NEXUS_HdmiOutputHandle) = NULL;

       if (__NEXUS_HdmiOutput_DisableHdcpAuthentication==NULL) {
          __NEXUS_HdmiOutput_DisableHdcpAuthentication = (NEXUS_Error (*)(NEXUS_HdmiOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_HdmiOutput_DisableHdcpAuthentication) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_HdmiOutput_DisableHdcpAuthentication()");
       NEXUS_Error ret = __NEXUS_HdmiOutput_DisableHdcpAuthentication(handle);
       tditrace_ex("@T-NEXUS_HdmiOutput_DisableHdcpAuthentication()");
       return ret;
}
#endif

#if 0

extern "C" void NEXUS_MemoryBlock_GetDefaultAllocationSettings(NEXUS_MemoryBlockAllocationSettings *pSettings)
{
   static void (*__NEXUS_MemoryBlock_GetDefaultAllocationSettings)(NEXUS_MemoryBlockAllocationSettings *) = NULL;

       if (__NEXUS_MemoryBlock_GetDefaultAllocationSettings==NULL) {
          __NEXUS_MemoryBlock_GetDefaultAllocationSettings = (void (*)(NEXUS_MemoryBlockAllocationSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MemoryBlock_GetDefaultAllocationSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MemoryBlock_GetDefaultAllocationSettings()");
       __NEXUS_MemoryBlock_GetDefaultAllocationSettings(pSettings);
       tditrace_ex("@T-NEXUS_MemoryBlock_GetDefaultAllocationSettings()");
}

extern "C" NEXUS_Error NEXUS_Memory_Allocate(size_t numBytes, const NEXUS_MemoryAllocationSettings *pSettings, void **ppMemory)
{
   static NEXUS_Error(*__NEXUS_Memory_Allocate)(uint32_t, const NEXUS_MemoryAllocationSettings *, void **) = NULL;

       if (__NEXUS_Memory_Allocate==NULL) {
          __NEXUS_Memory_Allocate = (NEXUS_Error (*)(uint32_t, const NEXUS_MemoryAllocationSettings *, void **))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Memory_Allocate) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Memory_Allocate");
       NEXUS_Error ret = __NEXUS_Memory_Allocate(numBytes, pSettings, ppMemory);
       tditrace_ex("@T-NEXUS_Memory_Allocate");
       return ret;
}

extern "C" void NEXUS_Memory_Free(void *pMemory)
{
   static void (*__NEXUS_Memory_Free)(void *) = NULL;

       if (__NEXUS_Memory_Free==NULL) {
          __NEXUS_Memory_Free = (void (*)(void *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Memory_Free) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Memory_Free()");
       __NEXUS_Memory_Free(pMemory);
       tditrace_ex("@T-NEXUS_Memory_Free()");
}

extern "C" void NEXUS_Memory_FlushCache(void *pMemory, size_t numBytes)
{
   static void(*__NEXUS_Memory_FlushCache)(void *, uint32_t) = NULL;

       if (__NEXUS_Memory_FlushCache==NULL) {
          __NEXUS_Memory_FlushCache = (void (*)(void *, uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Memory_FlushCache) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Memory_FlushCache()");
       __NEXUS_Memory_FlushCache(pMemory, numBytes);
       tditrace_ex("@T-NEXUS_Memory_FlushCache()");
}

extern "C" NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_Allocate(NEXUS_HeapHandle heap, size_t numBytes, size_t alignment, const NEXUS_MemoryBlockAllocationSettings *pSettings)
{
   static NEXUS_MemoryBlockHandle(*__NEXUS_MemoryBlock_Allocate)(NEXUS_HeapHandle, uint32_t, uint32_t, const NEXUS_MemoryBlockAllocationSettings *) = NULL;

       if (__NEXUS_MemoryBlock_Allocate==NULL) {
          __NEXUS_MemoryBlock_Allocate = (NEXUS_MemoryBlockHandle (*)(NEXUS_HeapHandle, uint32_t, uint32_t, const NEXUS_MemoryBlockAllocationSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MemoryBlock_Allocate) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MemoryBlock_Allocate()");
       NEXUS_MemoryBlockHandle ret = __NEXUS_MemoryBlock_Allocate(heap, numBytes, alignment, pSettings);
       tditrace_ex("@T-NEXUS_MemoryBlock_Allocate()");
       return ret;
}

extern "C" void NEXUS_MemoryBlock_Free(NEXUS_MemoryBlockHandle memoryBlock)
{
   static void (*__NEXUS_MemoryBlock_Free)(NEXUS_MemoryBlockHandle) = NULL;

       if (__NEXUS_MemoryBlock_Free==NULL) {
          __NEXUS_MemoryBlock_Free = (void (*)(NEXUS_MemoryBlockHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MemoryBlock_Free) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MemoryBlock_Free()");
       __NEXUS_MemoryBlock_Free(memoryBlock);
       tditrace_ex("@T-NEXUS_MemoryBlock_Free()");
}

extern "C" NEXUS_Error NEXUS_MemoryBlock_Lock(NEXUS_MemoryBlockHandle memoryBlock, void **ppMemory)
{
   static NEXUS_Error(*__NEXUS_MemoryBlock_Lock)(NEXUS_MemoryBlockHandle, void **) = NULL;

       if (__NEXUS_MemoryBlock_Lock==NULL) {
          __NEXUS_MemoryBlock_Lock = (NEXUS_Error (*)(NEXUS_MemoryBlockHandle, void **))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MemoryBlock_Lock) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MemoryBlock_Lock()");
       NEXUS_Error ret = __NEXUS_MemoryBlock_Lock(memoryBlock, ppMemory);
       tditrace_ex("@T-NEXUS_MemoryBlock_Lock()");
       return ret;
}

extern "C" void NEXUS_MemoryBlock_Unlock(NEXUS_MemoryBlockHandle memoryBlock)
{
   static void(*__NEXUS_MemoryBlock_Unlock)(NEXUS_MemoryBlockHandle) = NULL;

       if (__NEXUS_MemoryBlock_Unlock==NULL) {
          __NEXUS_MemoryBlock_Unlock = (void (*)(NEXUS_MemoryBlockHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MemoryBlock_Unlock) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MemoryBlock_Unlock()");
       __NEXUS_MemoryBlock_Unlock(memoryBlock);
       tditrace_ex("@T-NEXUS_MemoryBlock_Unlock()");
}

extern "C" NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_FromAddress(void *pMemory)
{
   static NEXUS_MemoryBlockHandle(*__NEXUS_MemoryBlock_FromAddress)(void *) = NULL;

       if (__NEXUS_MemoryBlock_FromAddress==NULL) {
          __NEXUS_MemoryBlock_FromAddress = (NEXUS_MemoryBlockHandle (*)(void *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MemoryBlock_FromAddress) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MemoryBlock_FromAddress()");
       NEXUS_MemoryBlockHandle ret = __NEXUS_MemoryBlock_FromAddress(pMemory);
       tditrace_ex("@T-NEXUS_MemoryBlock_FromAddress()");
       return ret;
}
#endif

#if 0
extern "C" void NEXUS_Message_GetDefaultSettings(NEXUS_MessageSettings *pSettings)
{
   static void(*__NEXUS_Message_GetDefaultSettings)(NEXUS_MessageSettings *) = NULL;

       if (__NEXUS_Message_GetDefaultSettings==NULL) {
          __NEXUS_Message_GetDefaultSettings = (void (*)(NEXUS_MessageSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_GetDefaultSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_GetDefaultSettings()");
       __NEXUS_Message_GetDefaultSettings(pSettings);
       tditrace_ex("@T-NEXUS_Message_GetDefaultSettings()");
}

extern "C" NEXUS_MessageHandle NEXUS_Message_Open(const NEXUS_MessageSettings *pSettings)
{
   static NEXUS_MessageHandle(*__NEXUS_Message_Open)(const NEXUS_MessageSettings *) = NULL;

       if (__NEXUS_Message_Open==NULL) {
          __NEXUS_Message_Open = (NEXUS_MessageHandle (*)(const NEXUS_MessageSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_Open()");
       NEXUS_MessageHandle ret = __NEXUS_Message_Open(pSettings);
       tditrace_ex("@T-NEXUS_Message_Open()");
       return ret;
}

extern "C" void NEXUS_Message_Close(NEXUS_MessageHandle handle)
{
   static void (*__NEXUS_Message_Close)(NEXUS_MessageHandle) = NULL;

       if (__NEXUS_Message_Close==NULL) {
          __NEXUS_Message_Close = (void (*)(NEXUS_MessageHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_Close()");
       __NEXUS_Message_Close(handle);
       tditrace_ex("@T-NEXUS_Message_Close()");
}

extern "C" void NEXUS_Message_GetDefaultStartSettings(NEXUS_MessageHandle handle, NEXUS_MessageStartSettings *pStartSettings)
{
   static void(*__NEXUS_Message_GetDefaultStartSettings)(NEXUS_MessageHandle, NEXUS_MessageStartSettings *) = NULL;

       if (__NEXUS_Message_GetDefaultStartSettings==NULL) {
          __NEXUS_Message_GetDefaultStartSettings = (void (*)(NEXUS_MessageHandle, NEXUS_MessageStartSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_GetDefaultStartSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_GetDefaultStartSettings()");
       __NEXUS_Message_GetDefaultStartSettings(handle, pStartSettings);
       tditrace_ex("@T-NEXUS_Message_GetDefaultStartSettings()");
}

extern "C" NEXUS_Error NEXUS_Message_Start(NEXUS_MessageHandle handle, const NEXUS_MessageStartSettings *pStartSettings)
{
   static NEXUS_Error(*__NEXUS_Message_Start)(NEXUS_MessageHandle, const NEXUS_MessageStartSettings *) = NULL;

       if (__NEXUS_Message_Start==NULL) {
          __NEXUS_Message_Start = (NEXUS_Error (*)(NEXUS_MessageHandle, const NEXUS_MessageStartSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_Start) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_Start()");
       NEXUS_Error ret = __NEXUS_Message_Start(handle, pStartSettings);
       tditrace_ex("@T-NEXUS_Message_Start()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Message_GetBufferWithWrap(NEXUS_MessageHandle handle, const void **pBuffer, size_t *pLength, const void **pBuffer2, size_t *pLength2)
{
   static NEXUS_Error(*__NEXUS_Message_GetBufferWithWrap)(NEXUS_MessageHandle, const void **, uint32_t *, const void **, uint32_t *) = NULL;

       if (__NEXUS_Message_GetBufferWithWrap==NULL) {
          __NEXUS_Message_GetBufferWithWrap = (NEXUS_Error (*)(NEXUS_MessageHandle,NEXUS_MessageHandle, const void **, uint32_t *, const void **, uint32_t *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_GetBufferWithWrap) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_GetBufferWithWrap()");
       NEXUS_Error ret = __NEXUS_Message_GetBufferWithWrap(handle, pBuffer, pLength, pBuffer2, pLength2);
       tditrace_ex("@T-NEXUS_Message_GetBufferWithWrap()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Message_ReadComplete(NEXUS_MessageHandle handle, size_t amountConsumed)
{
   static NEXUS_Error(*__NEXUS_Message_ReadComplete)(NEXUS_MessageHandle, uint32_t) = NULL;

       if (__NEXUS_Message_ReadComplete==NULL) {
          __NEXUS_Message_ReadComplete = (NEXUS_Error (*)(NEXUS_MessageHandle, uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_ReadComplete) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_ReadComplete()");
       NEXUS_Error ret = __NEXUS_Message_ReadComplete(handle, amountConsumed);
       tditrace_ex("@T-NEXUS_Message_ReadComplete()");
       return ret;
}

extern "C" void NEXUS_Message_Stop(NEXUS_MessageHandle handle)
{
   static void(*__NEXUS_Message_Stop)(NEXUS_MessageHandle) = NULL;

       if (__NEXUS_Message_Stop==NULL) {
          __NEXUS_Message_Stop = (void (*)(NEXUS_MessageHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Message_Stop) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Message_Stop()");
       __NEXUS_Message_Stop(handle);
       tditrace_ex("@T-NEXUS_Message_Stop()");
}

#endif

#if 0

extern "C" NEXUS_MpodInputHandle NEXUS_MpodInput_Open(NEXUS_MpodHandle mpod, const NEXUS_MpodInputSettings *pMpodInputSettings)
{
   static NEXUS_MpodInputHandle(*__NEXUS_MpodInput_Open)(NEXUS_MpodHandle, const NEXUS_MpodInputSettings *) = NULL;

       if (__NEXUS_MpodInput_Open==NULL) {
          __NEXUS_MpodInput_Open = (NEXUS_MpodInputHandle (*)(NEXUS_MpodHandle, const NEXUS_MpodInputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MpodInput_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MpodInput_Open()");
       NEXUS_MpodInputHandle ret = __NEXUS_MpodInput_Open(mpod, pMpodInputSettings);
       tditrace_ex("@T-NEXUS_MpodInput_Open()");
       return ret;
}

extern "C" void NEXUS_MpodInput_Close(NEXUS_MpodInputHandle mpod)
{
   static void(*__NEXUS_MpodInput_Close)(NEXUS_MpodInputHandle) = NULL;

       if (__NEXUS_MpodInput_Close==NULL) {
          __NEXUS_MpodInput_Close = (void (*)(NEXUS_MpodInputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_MpodInput_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_MpodInput_Close()");
       __NEXUS_MpodInput_Close(mpod);
       tditrace_ex("@T-NEXUS_MpodInput_Close()");
}
#endif


#if 0
extern "C" NEXUS_Error NEXUS_Platform_AuthenticatedJoin(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    static NEXUS_Error (*__NEXUS_Platform_AuthenticatedJoin)(const NEXUS_ClientAuthenticationSettings *) = NULL;

    if (__NEXUS_Platform_AuthenticatedJoin == NULL)
    {
        __NEXUS_Platform_AuthenticatedJoin = (NEXUS_Error (*)(const NEXUS_ClientAuthenticationSettings *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __NEXUS_Platform_AuthenticatedJoin)
        {
            fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
        }
    }

    tditrace_ex("@T+NEXUS_Platform_AuthenticatedJoin()");
    NEXUS_Error ret = __NEXUS_Platform_AuthenticatedJoin(pSettings);
    tditrace_ex("@T-NEXUS_Platform_AuthenticatedJoin()");

    return ret;
}


extern "C" void NEXUS_Platform_GetConfiguration(NEXUS_PlatformConfiguration *pConfiguration)
{
   static void(*__NEXUS_Platform_GetConfiguration)(NEXUS_PlatformConfiguration *) = NULL;

       if (__NEXUS_Platform_GetConfiguration==NULL) {
          __NEXUS_Platform_GetConfiguration = (void (*)(NEXUS_PlatformConfiguration *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_GetConfiguration) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_GetConfiguration()");
       __NEXUS_Platform_GetConfiguration(pConfiguration);
       tditrace_ex("@T-NEXUS_Platform_GetConfiguration()");
}

extern "C" NEXUS_HeapHandle NEXUS_Platform_GetFramebufferHeap(unsigned displayIndex)
{
   static NEXUS_HeapHandle(*__NEXUS_Platform_GetFramebufferHeap)(uint32_t) = NULL;

       if (__NEXUS_Platform_GetFramebufferHeap==NULL) {
          __NEXUS_Platform_GetFramebufferHeap = (NEXUS_HeapHandle (*)( uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_GetFramebufferHeap) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_GetFramebufferHeap()");
       NEXUS_HeapHandle ret = __NEXUS_Platform_GetFramebufferHeap(displayIndex);
       tditrace_ex("@T-NEXUS_Platform_GetFramebufferHeap()");
       return ret;
}

extern "C" void NEXUS_Platform_GetDefaultStartServerSettings(NEXUS_PlatformStartServerSettings *pSettings)
{
   static void(*__NEXUS_Platform_GetDefaultStartServerSettings)(NEXUS_PlatformStartServerSettings *) = NULL;

       if (__NEXUS_Platform_GetDefaultStartServerSettings==NULL) {
          __NEXUS_Platform_GetDefaultStartServerSettings = (void (*)(NEXUS_PlatformStartServerSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_GetDefaultStartServerSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_GetDefaultStartServerSettings()");
       __NEXUS_Platform_GetDefaultStartServerSettings(pSettings);
       tditrace_ex("@T-NEXUS_Platform_GetDefaultStartServerSettings()");
}

extern "C" NEXUS_Error NEXUS_Platform_StartServer(const NEXUS_PlatformStartServerSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_Platform_StartServer)(const NEXUS_PlatformStartServerSettings *) = NULL;

       if (__NEXUS_Platform_StartServer==NULL) {
          __NEXUS_Platform_StartServer = (NEXUS_Error (*)(const NEXUS_PlatformStartServerSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_StartServer) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_StartServer()");
       NEXUS_Error ret = __NEXUS_Platform_StartServer(pSettings);
       tditrace_ex("@T-NEXUS_Platform_StartServer()");
}

extern "C" void NEXUS_Platform_GetDefaultClientSettings(NEXUS_ClientSettings *pSettings)
{
   static void(*__NEXUS_Platform_GetDefaultClientSettings)(NEXUS_ClientSettings *) = NULL;

       if (__NEXUS_Platform_GetDefaultClientSettings==NULL) {
          __NEXUS_Platform_GetDefaultClientSettings = (void (*)(NEXUS_ClientSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_GetDefaultClientSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_GetDefaultClientSettings()");
       __NEXUS_Platform_GetDefaultClientSettings(pSettings);
       tditrace_ex("@T-NEXUS_Platform_GetDefaultClientSettings()");
}

extern "C" NEXUS_ClientHandle NEXUS_Platform_RegisterClient(const NEXUS_ClientSettings *pSettings)
{
   static NEXUS_ClientHandle(*__NEXUS_Platform_RegisterClient)(const NEXUS_ClientSettings *) = NULL;

       if (__NEXUS_Platform_RegisterClient==NULL) {
          __NEXUS_Platform_RegisterClient = (NEXUS_ClientHandle (*)(const NEXUS_ClientSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_RegisterClient) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_RegisterClient()");
       NEXUS_ClientHandle ret = __NEXUS_Platform_RegisterClient(pSettings);
       tditrace_ex("@T-NEXUS_Platform_RegisterClient()");
       return ret;
}

extern "C" void NEXUS_Platform_UnregisterClient(NEXUS_ClientHandle client)
{
   static void(*__NEXUS_Platform_UnregisterClient)(NEXUS_ClientHandle) = NULL;

       if (__NEXUS_Platform_UnregisterClient==NULL) {
          __NEXUS_Platform_UnregisterClient = (void (*)(NEXUS_ClientHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_UnregisterClient) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_UnregisterClient()");
       __NEXUS_Platform_UnregisterClient(client);
       tditrace_ex("@T-NEXUS_Platform_UnregisterClient()");
}

extern "C" void NEXUS_Platform_GetDefaultSettings(NEXUS_PlatformSettings *pSettings)
{
   static void(*__NEXUS_Platform_GetDefaultSettings)(NEXUS_PlatformSettings *) = NULL;

       if (__NEXUS_Platform_GetDefaultSettings==NULL) {
          __NEXUS_Platform_GetDefaultSettings = (void (*)(NEXUS_PlatformSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_GetDefaultSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_GetDefaultSettings()");
       __NEXUS_Platform_GetDefaultSettings(pSettings);
       tditrace_ex("@T-NEXUS_Platform_GetDefaultSettings()");
}

extern "C" NEXUS_Error NEXUS_Platform_Init_tagged(const NEXUS_PlatformSettings *pSettings, unsigned platformCheck, unsigned versionCheck, unsigned structSizeCheck)
{
   static NEXUS_Error(*__NEXUS_Platform_Init_tagged)(const NEXUS_PlatformSettings *, uint32_t, uint32_t, uint32_t) = NULL;

       if (__NEXUS_Platform_Init_tagged==NULL) {
          __NEXUS_Platform_Init_tagged = (NEXUS_Error (*)(const NEXUS_PlatformSettings *, uint32_t, uint32_t, uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_Init_tagged) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_Init_tagged()");
       NEXUS_Error ret = __NEXUS_Platform_Init_tagged(pSettings, platformCheck, versionCheck, structSizeCheck);
       tditrace_ex("@T-NEXUS_Platform_Init_tagged()");
       return ret;
}

extern "C" void NEXUS_Platform_Uninit(void)
{
   static void(*__NEXUS_Platform_Uninit)(void) = NULL;

       if (__NEXUS_Platform_Uninit==NULL) {
          __NEXUS_Platform_Uninit = (void (*)(void))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_Uninit) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_Uninit()");
       __NEXUS_Platform_Uninit();
       tditrace_ex("@T-NEXUS_Platform_Uninit()");
}

extern "C" void NEXUS_Platform_GetStandbySettings(NEXUS_PlatformStandbySettings *pSettings)
{
   static void (*__NEXUS_Platform_GetStandbySettings)(NEXUS_PlatformStandbySettings *) = NULL;

       if (__NEXUS_Platform_GetStandbySettings==NULL) {
          __NEXUS_Platform_GetStandbySettings = (void (*)(NEXUS_PlatformStandbySettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Platform_GetStandbySettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Platform_GetStandbySettings()");
       __NEXUS_Platform_GetStandbySettings(pSettings);
       tditrace_ex("@T-NEXUS_Platform_GetStandbySettings()");
}
#endif


#if 1
extern "C" NEXUS_Error NEXUS_Recpump_Start(NEXUS_RecpumpHandle handle)
{
   static NEXUS_Error(*__NEXUS_Recpump_Start)(NEXUS_RecpumpHandle) = NULL;

       if (__NEXUS_Recpump_Start==NULL) {
          __NEXUS_Recpump_Start = (NEXUS_Error (*)(NEXUS_RecpumpHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_Start) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_Start()");
       NEXUS_Error ret = __NEXUS_Recpump_Start(handle);
       tditrace_ex("@T-NEXUS_Recpump_Start()");
       return ret;
}

extern "C" void NEXUS_Recpump_Stop(NEXUS_RecpumpHandle handle)
{
   static void (*__NEXUS_Recpump_Stop)(NEXUS_RecpumpHandle) = NULL;

       if (__NEXUS_Recpump_Stop==NULL) {
          __NEXUS_Recpump_Stop = (void (*)(NEXUS_RecpumpHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_Stop) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_Stop()");
       __NEXUS_Recpump_Stop(handle);
       tditrace_ex("@T-NEXUS_Recpump_Stop()");
}

extern "C" NEXUS_Error NEXUS_Recpump_GetDataBuffer(NEXUS_RecpumpHandle handle, const void **pBuffer, size_t *pAmount)
{
   static NEXUS_Error(*__NEXUS_Recpump_GetDataBuffer)(NEXUS_RecpumpHandle, const void **, uint32_t *) = NULL;

       if (__NEXUS_Recpump_GetDataBuffer==NULL) {
          __NEXUS_Recpump_GetDataBuffer = (NEXUS_Error (*)(NEXUS_RecpumpHandle, const void **, uint32_t *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_GetDataBuffer) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_GetDataBuffer()");
       NEXUS_Error ret = __NEXUS_Recpump_GetDataBuffer(handle, pBuffer, pAmount);
       tditrace_ex("@T-NEXUS_Recpump_GetDataBuffer()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Recpump_DataReadComplete(NEXUS_RecpumpHandle handle, size_t amount)
{
   static NEXUS_Error(*__NEXUS_Recpump_DataReadComplete)(NEXUS_RecpumpHandle, uint32_t) = NULL;

       if (__NEXUS_Recpump_DataReadComplete==NULL) {
          __NEXUS_Recpump_DataReadComplete = (NEXUS_Error (*)(NEXUS_RecpumpHandle, uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_DataReadComplete) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_DataReadComplete()");
       NEXUS_Error ret = __NEXUS_Recpump_DataReadComplete(handle, amount);
       tditrace_ex("@T-NEXUS_Recpump_DataReadComplete()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Recpump_GetIndexBuffer(NEXUS_RecpumpHandle handle, const void **pBuffer, size_t *pAmount)
{
   static NEXUS_Error(*__NEXUS_Recpump_GetIndexBuffer)(NEXUS_RecpumpHandle, const void **, uint32_t *) = NULL;

       if (__NEXUS_Recpump_GetIndexBuffer==NULL) {
          __NEXUS_Recpump_GetIndexBuffer = (NEXUS_Error (*)(NEXUS_RecpumpHandle, const void **, uint32_t *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_GetIndexBuffer) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_GetIndexBuffer()");
       NEXUS_Error ret = __NEXUS_Recpump_GetIndexBuffer(handle, pBuffer, pAmount);
       tditrace_ex("@T-NEXUS_Recpump_GetIndexBuffer()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Recpump_IndexReadComplete(NEXUS_RecpumpHandle handle, size_t amount)
{
   static NEXUS_Error(*__NEXUS_Recpump_IndexReadComplete)(NEXUS_RecpumpHandle, uint32_t) = NULL;

       if (__NEXUS_Recpump_IndexReadComplete==NULL) {
          __NEXUS_Recpump_IndexReadComplete = (NEXUS_Error (*)(NEXUS_RecpumpHandle, uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_IndexReadComplete) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_IndexReadComplete()");
       NEXUS_Error ret = __NEXUS_Recpump_IndexReadComplete(handle, amount);
       tditrace_ex("@T-NEXUS_Recpump_IndexReadComplete()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Recpump_GetStatus(NEXUS_RecpumpHandle handle, NEXUS_RecpumpStatus *pStatus)
{
   static NEXUS_Error(*__NEXUS_Recpump_GetStatus)(NEXUS_RecpumpHandle, NEXUS_RecpumpStatus *) = NULL;

       if (__NEXUS_Recpump_GetStatus==NULL) {
          __NEXUS_Recpump_GetStatus = (NEXUS_Error (*)(NEXUS_RecpumpHandle, NEXUS_RecpumpStatus *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_GetStatus) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_GetStatus()");
       NEXUS_Error ret = __NEXUS_Recpump_GetStatus(handle, pStatus);
       tditrace_ex("@T-NEXUS_Recpump_GetStatus()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_Recpump_AddPidChannel(NEXUS_RecpumpHandle handle, NEXUS_PidChannelHandle pidChannel, const NEXUS_RecpumpAddPidChannelSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_Recpump_AddPidChannel)(NEXUS_RecpumpHandle, NEXUS_PidChannelHandle, const NEXUS_RecpumpAddPidChannelSettings *) = NULL;

       if (__NEXUS_Recpump_AddPidChannel==NULL) {
          __NEXUS_Recpump_AddPidChannel = (NEXUS_Error (*)(NEXUS_RecpumpHandle, NEXUS_PidChannelHandle, const NEXUS_RecpumpAddPidChannelSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_AddPidChannel) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_AddPidChannel()");
       NEXUS_Error ret = __NEXUS_Recpump_AddPidChannel(handle, pidChannel, pSettings);
       tditrace_ex("@T-NEXUS_Recpump_AddPidChannel()");
       return ret;
}

extern "C" void NEXUS_Recpump_RemoveAllPidChannels(NEXUS_RecpumpHandle handle)
{
   static void (*__NEXUS_Recpump_RemoveAllPidChannels)(NEXUS_RecpumpHandle) = NULL;

       if (__NEXUS_Recpump_RemoveAllPidChannels==NULL) {
          __NEXUS_Recpump_RemoveAllPidChannels = (void (*)(NEXUS_RecpumpHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_RemoveAllPidChannels) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_RemoveAllPidChannels()");
       __NEXUS_Recpump_RemoveAllPidChannels(handle);
       tditrace_ex("@T-NEXUS_Recpump_RemoveAllPidChannels()");
}

extern "C" NEXUS_Error NEXUS_Recpump_RemovePidChannel(NEXUS_RecpumpHandle handle, NEXUS_PidChannelHandle pidChannel)
{
   static NEXUS_Error(*__NEXUS_Recpump_RemovePidChannel)(NEXUS_RecpumpHandle, NEXUS_PidChannelHandle) = NULL;

       if (__NEXUS_Recpump_RemovePidChannel==NULL) {
          __NEXUS_Recpump_RemovePidChannel = (NEXUS_Error (*)(NEXUS_RecpumpHandle, NEXUS_PidChannelHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Recpump_RemovePidChannel) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Recpump_RemovePidChannel()");
       NEXUS_Error ret = __NEXUS_Recpump_RemovePidChannel(handle, pidChannel);
       tditrace_ex("@T-NEXUS_Recpump_RemovePidChannel()");
       return ret;
}
#endif

#if 1
extern "C" NEXUS_SpdifOutputHandle NEXUS_SpdifOutput_Open(unsigned index, const NEXUS_SpdifOutputSettings *pSettings)
{
   static NEXUS_SpdifOutputHandle(*__NEXUS_SpdifOutput_Open)(uint32_t, const NEXUS_SpdifOutputSettings *) = NULL;

       if (__NEXUS_SpdifOutput_Open==NULL) {
          __NEXUS_SpdifOutput_Open = (NEXUS_SpdifOutputHandle (*)(uint32_t, const NEXUS_SpdifOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SpdifOutput_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SpdifOutput_Open()");
       NEXUS_SpdifOutputHandle ret = __NEXUS_SpdifOutput_Open(index, pSettings);
       tditrace_ex("@T-NEXUS_SpdifOutput_Open()");
       return ret;
}

extern "C" void NEXUS_SpdifOutput_Close(NEXUS_SpdifOutputHandle handle)
{
   static void (*__NEXUS_SpdifOutput_Close)(NEXUS_SpdifOutputHandle) = NULL;

       if (__NEXUS_SpdifOutput_Close==NULL) {
          __NEXUS_SpdifOutput_Close = (void (*)(NEXUS_SpdifOutputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SpdifOutput_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SpdifOutput_Close()");
       __NEXUS_SpdifOutput_Close(handle);
       tditrace_ex("@T-NEXUS_SpdifOutput_Close()");
}

extern "C" void NEXUS_SpdifOutput_GetSettings(NEXUS_SpdifOutputHandle handle, NEXUS_SpdifOutputSettings *pSettings)
{
   static void (*__NEXUS_SpdifOutput_GetSettings)(NEXUS_SpdifOutputHandle, NEXUS_SpdifOutputSettings *) = NULL;

       if (__NEXUS_SpdifOutput_GetSettings==NULL) {
          __NEXUS_SpdifOutput_GetSettings = (void (*)(NEXUS_SpdifOutputHandle, NEXUS_SpdifOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SpdifOutput_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SpdifOutput_GetSettings()");
       __NEXUS_SpdifOutput_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SpdifOutput_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_SpdifOutput_SetSettings(NEXUS_SpdifOutputHandle handle, const NEXUS_SpdifOutputSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_SpdifOutput_SetSettings)(NEXUS_SpdifOutputHandle, const NEXUS_SpdifOutputSettings *) = NULL;

       if (__NEXUS_SpdifOutput_SetSettings==NULL) {
          __NEXUS_SpdifOutput_SetSettings = (NEXUS_Error (*)(NEXUS_SpdifOutputHandle, const NEXUS_SpdifOutputSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SpdifOutput_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SpdifOutput_SetSettings()");
       NEXUS_Error ret = __NEXUS_SpdifOutput_SetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SpdifOutput_SetSettings()");
       return ret;
}
#endif

extern "C" NEXUS_StcChannelHandle NEXUS_StcChannel_Open(unsigned index, const NEXUS_StcChannelSettings *pSettings)
{
   static NEXUS_StcChannelHandle(*__NEXUS_StcChannel_Open)(uint32_t, const NEXUS_StcChannelSettings *) = NULL;

       if (__NEXUS_StcChannel_Open==NULL) {
          __NEXUS_StcChannel_Open = (NEXUS_StcChannelHandle (*)(uint32_t, const NEXUS_StcChannelSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StcChannel_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StcChannel_Open()");
       NEXUS_StcChannelHandle ret = __NEXUS_StcChannel_Open(index, pSettings);
       tditrace_ex("@T-NEXUS_StcChannel_Open()");
       return ret;
}

extern "C" void NEXUS_StcChannel_Close(NEXUS_StcChannelHandle handle)
{
   static void(*__NEXUS_StcChannel_Close)(NEXUS_StcChannelHandle) = NULL;

       if (__NEXUS_StcChannel_Close==NULL) {
          __NEXUS_StcChannel_Close = (void (*)(NEXUS_StcChannelHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StcChannel_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StcChannel_Close()");
       __NEXUS_StcChannel_Close(handle);
       tditrace_ex("@T-NEXUS_StcChannel_Close()");
}

extern "C" NEXUS_Error NEXUS_StcChannel_Invalidate(NEXUS_StcChannelHandle handle)
{
   static NEXUS_Error (*__NEXUS_StcChannel_Invalidate)(NEXUS_StcChannelHandle) = NULL;

       if (__NEXUS_StcChannel_Invalidate==NULL) {
          __NEXUS_StcChannel_Invalidate = (NEXUS_Error (*)(NEXUS_StcChannelHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StcChannel_Invalidate) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StcChannel_Invalidate()");
       NEXUS_Error ret = __NEXUS_StcChannel_Invalidate(handle);
       tditrace_ex("@T-NEXUS_StcChannel_Invalidate()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_StcChannel_SetRate(NEXUS_StcChannelHandle stcChannel, unsigned increment, unsigned prescale)
{
   static NEXUS_Error (*__NEXUS_StcChannel_SetRate)(NEXUS_StcChannelHandle, uint32_t, uint32_t) = NULL;

       if (__NEXUS_StcChannel_SetRate==NULL) {
          __NEXUS_StcChannel_SetRate = (NEXUS_Error (*)(NEXUS_StcChannelHandle, uint32_t, uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StcChannel_SetRate) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StcChannel_SetRate()");
       NEXUS_Error ret = __NEXUS_StcChannel_SetRate(stcChannel, increment, prescale);
       tditrace_ex("@T-NEXUS_StcChannel_SetRate()");
       return ret;
}

extern "C" void NEXUS_StcChannel_GetStc(NEXUS_StcChannelHandle handle, uint32_t *pStc)
{
   static void (*__NEXUS_StcChannel_GetStc)(NEXUS_StcChannelHandle, uint32_t *) = NULL;

       if (__NEXUS_StcChannel_GetStc==NULL) {
          __NEXUS_StcChannel_GetStc = (void (*)(NEXUS_StcChannelHandle, uint32_t *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_StcChannel_GetStc) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_StcChannel_GetStc()");
       __NEXUS_StcChannel_GetStc(handle, pStc);
       tditrace_ex("@T-NEXUS_StcChannel_GetStc()");
}

#if 0
extern "C" void NEXUS_Surface_GetMemory(NEXUS_SurfaceHandle surface, NEXUS_SurfaceMemory *pMemory)
{
   static void (*__NEXUS_Surface_GetMemory)(NEXUS_SurfaceHandle, NEXUS_SurfaceMemory *) = NULL;

       if (__NEXUS_Surface_GetMemory==NULL) {
          __NEXUS_Surface_GetMemory = (void (*)(NEXUS_SurfaceHandle, NEXUS_SurfaceMemory *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Surface_GetMemory) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Surface_GetMemory()");
       __NEXUS_Surface_GetMemory(surface, pMemory);
       tditrace_ex("@T-NEXUS_Surface_GetMemory()");
}
#endif

extern "C" void NEXUS_Surface_Flush(NEXUS_SurfaceHandle surface)
{
   static void (*__NEXUS_Surface_Flush)(NEXUS_SurfaceHandle) = NULL;

       if (__NEXUS_Surface_Flush==NULL) {
          __NEXUS_Surface_Flush = (void (*)(NEXUS_SurfaceHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Surface_Flush) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Surface_Flush()");
       __NEXUS_Surface_Flush(surface);
       tditrace_ex("@T-NEXUS_Surface_Flush()");
}

extern "C" void NEXUS_Surface_GetDefaultCreateSettings(NEXUS_SurfaceCreateSettings *pCreateSettings)
{
   static void (*__NEXUS_Surface_GetDefaultCreateSettings)(NEXUS_SurfaceCreateSettings *) = NULL;

       if (__NEXUS_Surface_GetDefaultCreateSettings==NULL) {
          __NEXUS_Surface_GetDefaultCreateSettings = (void (*)(NEXUS_SurfaceCreateSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Surface_GetDefaultCreateSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Surface_GetDefaultCreateSettings()");
       __NEXUS_Surface_GetDefaultCreateSettings(pCreateSettings);
       tditrace_ex("@T-NEXUS_Surface_GetDefaultCreateSettings()");
}

extern "C" NEXUS_SurfaceHandle NEXUS_Surface_Create(const NEXUS_SurfaceCreateSettings *pCreateSettings)
{
   static NEXUS_SurfaceHandle(*__NEXUS_Surface_Create)(const NEXUS_SurfaceCreateSettings *) = NULL;

       if (__NEXUS_Surface_Create==NULL) {
          __NEXUS_Surface_Create = (NEXUS_SurfaceHandle (*)(const NEXUS_SurfaceCreateSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Surface_Create) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Surface_Create()");
       NEXUS_SurfaceHandle ret = __NEXUS_Surface_Create(pCreateSettings);
       tditrace_ex("@T-NEXUS_Surface_Create()");
       return ret;
}

extern "C" void NEXUS_Surface_Destroy(NEXUS_SurfaceHandle surface)
{
   static void (*__NEXUS_Surface_Destroy)(NEXUS_SurfaceHandle) = NULL;

       if (__NEXUS_Surface_Destroy==NULL) {
          __NEXUS_Surface_Destroy = (void (*)(NEXUS_SurfaceHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Surface_Destroy) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Surface_Destroy()");
       __NEXUS_Surface_Destroy(surface);
       tditrace_ex("@T-NEXUS_Surface_Destroy()");
}

extern "C" NEXUS_Error NEXUS_SurfaceClient_UpdateSurface(NEXUS_SurfaceClientHandle handle, const NEXUS_Rect *pUpdateRect)
{
   static NEXUS_Error(*__NEXUS_SurfaceClient_UpdateSurface)(NEXUS_SurfaceClientHandle, const NEXUS_Rect *) = NULL;

       if (__NEXUS_SurfaceClient_UpdateSurface==NULL) {
          __NEXUS_SurfaceClient_UpdateSurface = (NEXUS_Error (*)(NEXUS_SurfaceClientHandle, const NEXUS_Rect *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceClient_UpdateSurface) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceClient_UpdateSurface()");
       NEXUS_Error ret = __NEXUS_SurfaceClient_UpdateSurface(handle, pUpdateRect);
       tditrace_ex("@T-NEXUS_SurfaceClient_UpdateSurface()");
}

extern "C" void NEXUS_SurfaceClient_GetSettings(NEXUS_SurfaceClientHandle handle, NEXUS_SurfaceClientSettings *pSettings)
{
   static void(*__NEXUS_SurfaceClient_GetSettings)(NEXUS_SurfaceClientHandle, NEXUS_SurfaceClientSettings *) = NULL;

       if (__NEXUS_SurfaceClient_GetSettings==NULL) {
          __NEXUS_SurfaceClient_GetSettings = (void (*)(NEXUS_SurfaceClientHandle, NEXUS_SurfaceClientSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceClient_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceClient_GetSettings()");
       __NEXUS_SurfaceClient_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SurfaceClient_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_SurfaceClient_SetSettings(NEXUS_SurfaceClientHandle handle, const NEXUS_SurfaceClientSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_SurfaceClient_SetSettings)(NEXUS_SurfaceClientHandle, const NEXUS_SurfaceClientSettings *) = NULL;

       if (__NEXUS_SurfaceClient_SetSettings==NULL) {
          __NEXUS_SurfaceClient_SetSettings = (NEXUS_Error (*)(NEXUS_SurfaceClientHandle, const NEXUS_SurfaceClientSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceClient_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceClient_SetSettings()");
       NEXUS_Error ret = __NEXUS_SurfaceClient_SetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SurfaceClient_SetSettings()");
}

extern "C" NEXUS_Error NEXUS_SurfaceClient_SetSurface(NEXUS_SurfaceClientHandle handle, NEXUS_SurfaceHandle surface)
{
   static NEXUS_Error(*__NEXUS_SurfaceClient_SetSurface)(NEXUS_SurfaceClientHandle, NEXUS_SurfaceHandle) = NULL;

       if (__NEXUS_SurfaceClient_SetSurface==NULL) {
          __NEXUS_SurfaceClient_SetSurface = (NEXUS_Error (*)(NEXUS_SurfaceClientHandle, NEXUS_SurfaceHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceClient_SetSurface) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceClient_SetSurface()");
       NEXUS_Error ret = __NEXUS_SurfaceClient_SetSurface(handle, surface);
       tditrace_ex("@T-NEXUS_SurfaceClient_SetSurface()");
}

extern "C" NEXUS_SurfaceCompositorHandle NEXUS_SurfaceCompositor_Create(unsigned server_id)
{
   static NEXUS_SurfaceCompositorHandle(*__NEXUS_SurfaceCompositor_Create)(uint32_t) = NULL;

       if (__NEXUS_SurfaceCompositor_Create==NULL) {
          __NEXUS_SurfaceCompositor_Create = (NEXUS_SurfaceCompositorHandle (*)(uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_Create) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_Create()");
       NEXUS_SurfaceCompositorHandle ret = __NEXUS_SurfaceCompositor_Create(server_id);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_Create()");
       return ret;
}

extern "C" NEXUS_SurfaceClientHandle NEXUS_SurfaceCompositor_CreateClient(NEXUS_SurfaceCompositorHandle handle, NEXUS_SurfaceCompositorClientId client_id)
{
   static NEXUS_SurfaceClientHandle(*__NEXUS_SurfaceCompositor_CreateClient)(NEXUS_SurfaceCompositorHandle, NEXUS_SurfaceCompositorClientId) = NULL;

       if (__NEXUS_SurfaceCompositor_CreateClient==NULL) {
          __NEXUS_SurfaceCompositor_CreateClient = (NEXUS_SurfaceClientHandle (*)(NEXUS_SurfaceCompositorHandle, NEXUS_SurfaceCompositorClientId))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_CreateClient) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_CreateClient()");
       NEXUS_SurfaceClientHandle ret = __NEXUS_SurfaceCompositor_CreateClient(handle, client_id);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_CreateClient()");
       return ret;
}

extern "C" void NEXUS_SurfaceCompositor_Destroy(NEXUS_SurfaceCompositorHandle handle)
{
   static void (*__NEXUS_SurfaceCompositor_Destroy)(NEXUS_SurfaceCompositorHandle) = NULL;

       if (__NEXUS_SurfaceCompositor_Destroy==NULL) {
          __NEXUS_SurfaceCompositor_Destroy = (void (*)(NEXUS_SurfaceCompositorHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_Destroy) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_Destroy()");
       __NEXUS_SurfaceCompositor_Destroy(handle);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_Destroy()");
}

extern "C" NEXUS_Error NEXUS_SurfaceCompositor_SetSettings(NEXUS_SurfaceCompositorHandle handle, const NEXUS_SurfaceCompositorSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_SurfaceCompositor_SetSettings)(NEXUS_SurfaceCompositorHandle, const NEXUS_SurfaceCompositorSettings *) = NULL;

       if (__NEXUS_SurfaceCompositor_SetSettings==NULL) {
          __NEXUS_SurfaceCompositor_SetSettings = (NEXUS_Error (*)(NEXUS_SurfaceCompositorHandle, const NEXUS_SurfaceCompositorSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_SetSettings()");
       NEXUS_Error ret = __NEXUS_SurfaceCompositor_SetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_SetSettings()");
       return ret;
}

extern "C" void NEXUS_SurfaceCompositor_DestroyClient(NEXUS_SurfaceClientHandle client)
{
   static void (*__NEXUS_SurfaceCompositor_DestroyClient)(NEXUS_SurfaceClientHandle) = NULL;

       if (__NEXUS_SurfaceCompositor_DestroyClient==NULL) {
          __NEXUS_SurfaceCompositor_DestroyClient = (void (*)(NEXUS_SurfaceClientHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_DestroyClient) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_DestroyClient()");
       __NEXUS_SurfaceCompositor_DestroyClient(client);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_DestroyClient()");
}

extern "C" void NEXUS_SurfaceCompositor_GetClientSettings(NEXUS_SurfaceCompositorHandle handle, NEXUS_SurfaceClientHandle client, NEXUS_SurfaceCompositorClientSettings *pSettings)
{
   static void (*__NEXUS_SurfaceCompositor_GetClientSettings)(NEXUS_SurfaceCompositorHandle,NEXUS_SurfaceClientHandle, NEXUS_SurfaceCompositorClientSettings *) = NULL;

       if (__NEXUS_SurfaceCompositor_GetClientSettings==NULL) {
          __NEXUS_SurfaceCompositor_GetClientSettings = (void (*)(NEXUS_SurfaceCompositorHandle,NEXUS_SurfaceClientHandle, NEXUS_SurfaceCompositorClientSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_GetClientSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_GetClientSettings()");
       __NEXUS_SurfaceCompositor_GetClientSettings(handle,client, pSettings);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_GetClientSettings()");
}

extern "C" NEXUS_Error NEXUS_SurfaceCompositor_SetClientSettings(NEXUS_SurfaceCompositorHandle handle, NEXUS_SurfaceClientHandle client, const NEXUS_SurfaceCompositorClientSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_SurfaceCompositor_SetClientSettings)(NEXUS_SurfaceCompositorHandle,NEXUS_SurfaceClientHandle, const NEXUS_SurfaceCompositorClientSettings *) = NULL;

       if (__NEXUS_SurfaceCompositor_SetClientSettings==NULL) {
          __NEXUS_SurfaceCompositor_SetClientSettings = (NEXUS_Error (*)(NEXUS_SurfaceCompositorHandle,NEXUS_SurfaceClientHandle, const NEXUS_SurfaceCompositorClientSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_SetClientSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_SetClientSettings()");
       NEXUS_Error ret = __NEXUS_SurfaceCompositor_SetClientSettings(handle,client, pSettings);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_SetClientSettings()");
       return ret;
}

extern "C" void NEXUS_SurfaceCompositor_GetSettings(NEXUS_SurfaceCompositorHandle handle, NEXUS_SurfaceCompositorSettings *pSettings)
{
   static void (*__NEXUS_SurfaceCompositor_GetSettings)(NEXUS_SurfaceCompositorHandle, NEXUS_SurfaceCompositorSettings *) = NULL;

       if (__NEXUS_SurfaceCompositor_GetSettings==NULL) {
          __NEXUS_SurfaceCompositor_GetSettings = (void (*)(NEXUS_SurfaceCompositorHandle, NEXUS_SurfaceCompositorSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SurfaceCompositor_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SurfaceCompositor_GetSettings()");
       __NEXUS_SurfaceCompositor_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SurfaceCompositor_GetSettings()");
}

extern "C" NEXUS_SyncChannelHandle NEXUS_SyncChannel_Create(const NEXUS_SyncChannelSettings *pSettings)
{
   static NEXUS_SyncChannelHandle(*__NEXUS_SyncChannel_Create)(const NEXUS_SyncChannelSettings *) = NULL;

       if (__NEXUS_SyncChannel_Create==NULL) {
          __NEXUS_SyncChannel_Create = (NEXUS_SyncChannelHandle (*)(const NEXUS_SyncChannelSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SyncChannel_Create) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SyncChannel_Create()");
       NEXUS_SyncChannelHandle ret = __NEXUS_SyncChannel_Create(pSettings);
       tditrace_ex("@T-NEXUS_SyncChannel_Create()");
       return ret;
}

extern "C" void NEXUS_SyncChannel_GetSettings(NEXUS_SyncChannelHandle handle, NEXUS_SyncChannelSettings *pSettings)
{
   static void (*__NEXUS_SyncChannel_GetSettings)(NEXUS_SyncChannelHandle, NEXUS_SyncChannelSettings *) = NULL;

       if (__NEXUS_SyncChannel_GetSettings==NULL) {
          __NEXUS_SyncChannel_GetSettings = (void (*)(NEXUS_SyncChannelHandle, NEXUS_SyncChannelSettings * ))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SyncChannel_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SyncChannel_GetSettings()");
       __NEXUS_SyncChannel_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SyncChannel_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_SyncChannel_SetSettings(NEXUS_SyncChannelHandle handle, const NEXUS_SyncChannelSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_SyncChannel_SetSettings)(NEXUS_SyncChannelHandle, const NEXUS_SyncChannelSettings *) = NULL;

       if (__NEXUS_SyncChannel_SetSettings==NULL) {
          __NEXUS_SyncChannel_SetSettings = (NEXUS_Error (*)(NEXUS_SyncChannelHandle, const NEXUS_SyncChannelSettings * ))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SyncChannel_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SyncChannel_SetSettings()");
       NEXUS_Error ret = __NEXUS_SyncChannel_SetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_SyncChannel_SetSettings()");
       return ret;
}

extern "C" void NEXUS_SyncChannel_Destroy(NEXUS_SyncChannelHandle handle)
{
   static void (*__NEXUS_SyncChannel_Destroy)(NEXUS_SyncChannelHandle) = NULL;

       if (__NEXUS_SyncChannel_Destroy==NULL) {
          __NEXUS_SyncChannel_Destroy = (void (*)(NEXUS_SyncChannelHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_SyncChannel_Destroy) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_SyncChannel_Destroy()");
       __NEXUS_SyncChannel_Destroy(handle);
       tditrace_ex("@T-NEXUS_SyncChannel_Destroy()");
}

extern "C" NEXUS_Timebase NEXUS_Timebase_Open(unsigned index)
{
   static NEXUS_Timebase(*__NEXUS_Timebase_Open)(uint32_t) = NULL;

       if (__NEXUS_Timebase_Open==NULL) {
          __NEXUS_Timebase_Open = (NEXUS_Timebase (*)(uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Timebase_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Timebase_Open()");
       NEXUS_Timebase ret = __NEXUS_Timebase_Open(index);
       tditrace_ex("@T-NEXUS_Timebase_Open()");
       return ret;
}

extern "C" void NEXUS_Timebase_Close(NEXUS_Timebase timebase)
{
   static void(*__NEXUS_Timebase_Close)(NEXUS_Timebase) = NULL;

       if (__NEXUS_Timebase_Close==NULL) {
          __NEXUS_Timebase_Close = (void (*)(NEXUS_Timebase))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_Timebase_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_Timebase_Close()");
       __NEXUS_Timebase_Close(timebase);
       tditrace_ex("@T-NEXUS_Timebase_Close()");
}

extern "C" NEXUS_Error NEXUS_VideoWindow_SetScalerSettings(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowScalerSettings *pSettings)
{
    static NEXUS_Error(*__NEXUS_VideoWindow_SetScalerSettings)(NEXUS_VideoWindowHandle,const NEXUS_VideoWindowScalerSettings *) = NULL;

       if (__NEXUS_VideoWindow_SetScalerSettings==NULL) {
          __NEXUS_VideoWindow_SetScalerSettings = (NEXUS_Error (*)(NEXUS_VideoWindowHandle,const NEXUS_VideoWindowScalerSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_SetScalerSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_SetScalerSettings()");
       NEXUS_Error ret = __NEXUS_VideoWindow_SetScalerSettings(window, pSettings);
       tditrace_ex("@T-NEXUS_VideoWindow_SetScalerSettings()");
       return ret;
}

extern "C" NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_Open(unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
   static NEXUS_VideoDecoderHandle(*__NEXUS_VideoDecoder_Open)(uint32_t ,const NEXUS_VideoDecoderOpenSettings *) = NULL;

       if (__NEXUS_VideoDecoder_Open==NULL) {
          __NEXUS_VideoDecoder_Open = (NEXUS_VideoDecoderHandle (*)(uint32_t ,const NEXUS_VideoDecoderOpenSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_Open()");
       NEXUS_VideoDecoderHandle ret = __NEXUS_VideoDecoder_Open(index, pOpenSettings);
       tditrace_ex("@T-NEXUS_VideoDecoder_Open()");
       return ret;
}

extern "C" void NEXUS_VideoDecoder_Close(NEXUS_VideoDecoderHandle handle)
{
   static void (*__NEXUS_VideoDecoder_Close)(NEXUS_VideoDecoderHandle) = NULL;

       if (__NEXUS_VideoDecoder_Close==NULL) {
          __NEXUS_VideoDecoder_Close = (void (*)(NEXUS_VideoDecoderHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_Close()");
       __NEXUS_VideoDecoder_Close(handle);
       tditrace_ex("@T-NEXUS_VideoDecoder_Close()");
}

extern "C" NEXUS_VideoInputHandle NEXUS_VideoDecoder_GetConnector(NEXUS_VideoDecoderHandle handle)
{
    static NEXUS_VideoInputHandle(*__NEXUS_VideoDecoder_GetConnector)(NEXUS_VideoDecoderHandle) = NULL;

       if (__NEXUS_VideoDecoder_GetConnector==NULL) {
          __NEXUS_VideoDecoder_GetConnector = (NEXUS_VideoInputHandle (*)(NEXUS_VideoDecoderHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_GetConnector) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_GetConnector()");
       NEXUS_VideoInputHandle ret = __NEXUS_VideoDecoder_GetConnector(handle);
       tditrace_ex("@T-NEXUS_VideoDecoder_GetConnector()");
       return ret;
}

extern "C" void NEXUS_VideoDecoder_Flush(NEXUS_VideoDecoderHandle handle)
{
   static void (*__NEXUS_VideoDecoder_Flush)(NEXUS_VideoDecoderHandle) = NULL;

       if (__NEXUS_VideoDecoder_Flush==NULL) {
          __NEXUS_VideoDecoder_Flush = (void (*)(NEXUS_VideoDecoderHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_Flush) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_Flush()");
       __NEXUS_VideoDecoder_Flush(handle);
       tditrace_ex("@T-NEXUS_VideoDecoder_Flush()");
}

extern "C" void NEXUS_VideoDecoder_GetSettings(NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderSettings *pSettings)
{
   static void (*__NEXUS_VideoDecoder_GetSettings)(NEXUS_VideoDecoderHandle, NEXUS_VideoDecoderSettings *) = NULL;

       if (__NEXUS_VideoDecoder_GetSettings==NULL) {
          __NEXUS_VideoDecoder_GetSettings = (void (*)(NEXUS_VideoDecoderHandle, NEXUS_VideoDecoderSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_GetSettings()");
       __NEXUS_VideoDecoder_GetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_VideoDecoder_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_VideoDecoder_SetSettings(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_VideoDecoder_SetSettings)(NEXUS_VideoDecoderHandle, const NEXUS_VideoDecoderSettings *) = NULL;

       if (__NEXUS_VideoDecoder_SetSettings==NULL) {
          __NEXUS_VideoDecoder_SetSettings = (NEXUS_Error (*)(NEXUS_VideoDecoderHandle, const NEXUS_VideoDecoderSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_SetSettings()");
       NEXUS_Error ret = __NEXUS_VideoDecoder_SetSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_VideoDecoder_SetSettings()");
       return ret;
}

//video primer

extern "C" NEXUS_Error NEXUS_VideoDecoderPrimer_SetSettings ( NEXUS_VideoDecoderPrimerHandle primer ,  const NEXUS_VideoDecoderPrimerSettings * pSettings )
{
   static NEXUS_Error (* __NEXUS_VideoDecoderPrimer_SetSettings) ( NEXUS_VideoDecoderPrimerHandle ,  const NEXUS_VideoDecoderPrimerSettings * ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_SetSettings == NULL) {
      __NEXUS_VideoDecoderPrimer_SetSettings = (NEXUS_Error (*) ( NEXUS_VideoDecoderPrimerHandle ,  const NEXUS_VideoDecoderPrimerSettings * )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_SetSettings");
      if (__NEXUS_VideoDecoderPrimer_SetSettings == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_SetSettings()");
   NEXUS_Error ret = __NEXUS_VideoDecoderPrimer_SetSettings ( primer ,  pSettings );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_SetSettings()");
   return ret;
}

extern "C" void NEXUS_VideoDecoderPrimer_Close ( NEXUS_VideoDecoderPrimerHandle primer )
{
   static void (* __NEXUS_VideoDecoderPrimer_Close) ( NEXUS_VideoDecoderPrimerHandle ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_Close == NULL) {
      __NEXUS_VideoDecoderPrimer_Close = (void (*) ( NEXUS_VideoDecoderPrimerHandle )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_Close");
      if (__NEXUS_VideoDecoderPrimer_Close == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_Close()");
   __NEXUS_VideoDecoderPrimer_Close ( primer );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_Close()");
}

extern "C" void NEXUS_VideoDecoderPrimer_Stop ( NEXUS_VideoDecoderPrimerHandle primer )
{
   static void (* __NEXUS_VideoDecoderPrimer_Stop) ( NEXUS_VideoDecoderPrimerHandle ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_Stop == NULL) {
      __NEXUS_VideoDecoderPrimer_Stop = (void (*) ( NEXUS_VideoDecoderPrimerHandle )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_Stop");
      if (__NEXUS_VideoDecoderPrimer_Stop == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_Stop()");
   __NEXUS_VideoDecoderPrimer_Stop ( primer );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_Stop()");
}

extern "C" void NEXUS_VideoDecoderPrimer_GetSettings ( NEXUS_VideoDecoderPrimerHandle primer ,  NEXUS_VideoDecoderPrimerSettings * pSettings )
{
   static void (* __NEXUS_VideoDecoderPrimer_GetSettings) ( NEXUS_VideoDecoderPrimerHandle ,  NEXUS_VideoDecoderPrimerSettings * ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_GetSettings == NULL) {
      __NEXUS_VideoDecoderPrimer_GetSettings = (void (*) ( NEXUS_VideoDecoderPrimerHandle ,  NEXUS_VideoDecoderPrimerSettings * )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_GetSettings");
      if (__NEXUS_VideoDecoderPrimer_GetSettings == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_GetSettings()");
   __NEXUS_VideoDecoderPrimer_GetSettings ( primer ,  pSettings );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_GetSettings()");
}

extern "C" NEXUS_VideoDecoderPrimerHandle NEXUS_VideoDecoderPrimer_Open ( NEXUS_VideoDecoderHandle videoDecoder )
{
   static NEXUS_VideoDecoderPrimerHandle (* __NEXUS_VideoDecoderPrimer_Open) ( NEXUS_VideoDecoderHandle ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_Open == NULL) {
      __NEXUS_VideoDecoderPrimer_Open = (NEXUS_VideoDecoderPrimerHandle (*) ( NEXUS_VideoDecoderHandle )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_Open");
      if (__NEXUS_VideoDecoderPrimer_Open == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_Open()");
   NEXUS_VideoDecoderPrimerHandle ret = __NEXUS_VideoDecoderPrimer_Open ( videoDecoder );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_Open()");
   return ret;
}

extern "C" NEXUS_Error NEXUS_VideoDecoderPrimer_Start ( NEXUS_VideoDecoderPrimerHandle primer ,  const NEXUS_VideoDecoderStartSettings * pStartSettings )
{
   static NEXUS_Error (* __NEXUS_VideoDecoderPrimer_Start) ( NEXUS_VideoDecoderPrimerHandle ,  const NEXUS_VideoDecoderStartSettings * ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_Start == NULL) {
      __NEXUS_VideoDecoderPrimer_Start = (NEXUS_Error (*) ( NEXUS_VideoDecoderPrimerHandle ,  const NEXUS_VideoDecoderStartSettings * )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_Start");
      if (__NEXUS_VideoDecoderPrimer_Start == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_Start()");
   NEXUS_Error ret = __NEXUS_VideoDecoderPrimer_Start ( primer ,  pStartSettings );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_Start()");
   return ret;
}

extern "C" NEXUS_Error NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode ( NEXUS_VideoDecoderPrimerHandle primer ,  NEXUS_VideoDecoderHandle videoDecoder )
{
   static NEXUS_Error (* __NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode) ( NEXUS_VideoDecoderPrimerHandle ,  NEXUS_VideoDecoderHandle ) = NULL;
   if (__NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode == NULL) {
      __NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode = (NEXUS_Error (*) ( NEXUS_VideoDecoderPrimerHandle ,  NEXUS_VideoDecoderHandle )) dlsym(RTLD_NEXT, "NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode");
      if (__NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode()");
   NEXUS_Error ret = __NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode ( primer ,  videoDecoder );
   tditrace_ex("@T-NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode()");
   return ret;
}


//audio primer

extern "C" NEXUS_Error NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer ( NEXUS_AudioDecoderPrimerHandle primer ,  NEXUS_AudioDecoderHandle audioDecoder )
{
   static NEXUS_Error (* __NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer) ( NEXUS_AudioDecoderPrimerHandle ,  NEXUS_AudioDecoderHandle ) = NULL;
   if (__NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer == NULL) {
      __NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer = (NEXUS_Error (*) ( NEXUS_AudioDecoderPrimerHandle ,  NEXUS_AudioDecoderHandle )) dlsym(RTLD_NEXT, "NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer");
      if (__NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer()");
   NEXUS_Error ret = __NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer ( primer ,  audioDecoder );
   tditrace_ex("@T-NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer()");
   return ret;
}

extern "C" NEXUS_Error NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode ( NEXUS_AudioDecoderPrimerHandle primer ,  NEXUS_AudioDecoderHandle audioDecoder )
{
   static NEXUS_Error (* __NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode) ( NEXUS_AudioDecoderPrimerHandle ,  NEXUS_AudioDecoderHandle ) = NULL;
   if (__NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode == NULL) {
      __NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode = (NEXUS_Error (*) ( NEXUS_AudioDecoderPrimerHandle ,  NEXUS_AudioDecoderHandle )) dlsym(RTLD_NEXT, "NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode");
      if (__NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode()");
   NEXUS_Error ret = __NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode ( primer ,  audioDecoder );
   tditrace_ex("@T-NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode()");
   return ret;
}

extern "C" void NEXUS_AudioDecoderPrimer_Stop ( NEXUS_AudioDecoderPrimerHandle primer )
{
   static void (* __NEXUS_AudioDecoderPrimer_Stop) ( NEXUS_AudioDecoderPrimerHandle ) = NULL;
   if (__NEXUS_AudioDecoderPrimer_Stop == NULL) {
      __NEXUS_AudioDecoderPrimer_Stop = (void (*) ( NEXUS_AudioDecoderPrimerHandle )) dlsym(RTLD_NEXT, "NEXUS_AudioDecoderPrimer_Stop");
      if (__NEXUS_AudioDecoderPrimer_Stop == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_AudioDecoderPrimer_Stop()");
   __NEXUS_AudioDecoderPrimer_Stop ( primer );
   tditrace_ex("@T-NEXUS_AudioDecoderPrimer_Stop()");
}

extern "C" NEXUS_Error NEXUS_AudioDecoderPrimer_Start ( NEXUS_AudioDecoderPrimerHandle primer ,  const NEXUS_AudioDecoderStartSettings * pStartSettings )
{
   static NEXUS_Error (* __NEXUS_AudioDecoderPrimer_Start) ( NEXUS_AudioDecoderPrimerHandle ,  const NEXUS_AudioDecoderStartSettings * ) = NULL;
   if (__NEXUS_AudioDecoderPrimer_Start == NULL) {
      __NEXUS_AudioDecoderPrimer_Start = (NEXUS_Error (*) ( NEXUS_AudioDecoderPrimerHandle ,  const NEXUS_AudioDecoderStartSettings * )) dlsym(RTLD_NEXT, "NEXUS_AudioDecoderPrimer_Start");
      if (__NEXUS_AudioDecoderPrimer_Start == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_AudioDecoderPrimer_Start()");
   NEXUS_Error ret = __NEXUS_AudioDecoderPrimer_Start ( primer ,  pStartSettings );
   tditrace_ex("@T-NEXUS_AudioDecoderPrimer_Start()");
   return ret;
}

extern "C" void NEXUS_AudioDecoderPrimer_Close ( NEXUS_AudioDecoderPrimerHandle primer )
{
   static void (* __NEXUS_AudioDecoderPrimer_Close) ( NEXUS_AudioDecoderPrimerHandle ) = NULL;
   if (__NEXUS_AudioDecoderPrimer_Close == NULL) {
      __NEXUS_AudioDecoderPrimer_Close = (void (*) ( NEXUS_AudioDecoderPrimerHandle )) dlsym(RTLD_NEXT, "NEXUS_AudioDecoderPrimer_Close");
      if (__NEXUS_AudioDecoderPrimer_Close == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_AudioDecoderPrimer_Close()");
   __NEXUS_AudioDecoderPrimer_Close ( primer );
   tditrace_ex("@T-NEXUS_AudioDecoderPrimer_Close()");
}

extern "C" NEXUS_Error NEXUS_AudioDecoder_SetPrimerSettings ( NEXUS_AudioDecoderPrimerHandle primer ,  const NEXUS_AudioDecoderPrimerSettings * pSettings )
{
   static NEXUS_Error (* __NEXUS_AudioDecoder_SetPrimerSettings) ( NEXUS_AudioDecoderPrimerHandle ,  const NEXUS_AudioDecoderPrimerSettings * ) = NULL;
   if (__NEXUS_AudioDecoder_SetPrimerSettings == NULL) {
      __NEXUS_AudioDecoder_SetPrimerSettings = (NEXUS_Error (*) ( NEXUS_AudioDecoderPrimerHandle ,  const NEXUS_AudioDecoderPrimerSettings * )) dlsym(RTLD_NEXT, "NEXUS_AudioDecoder_SetPrimerSettings");
      if (__NEXUS_AudioDecoder_SetPrimerSettings == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_AudioDecoder_SetPrimerSettings()");
   NEXUS_Error ret = __NEXUS_AudioDecoder_SetPrimerSettings ( primer ,  pSettings );
   tditrace_ex("@T-NEXUS_AudioDecoder_SetPrimerSettings()");
   return ret;
}


extern "C" NEXUS_Error NEXUS_VideoDecoder_SetUserDataFormatFilter(NEXUS_VideoDecoderHandle handle, NEXUS_UserDataFormat format)
{
   static NEXUS_Error(*__NEXUS_VideoDecoder_SetUserDataFormatFilter)(NEXUS_VideoDecoderHandle, NEXUS_UserDataFormat) = NULL;

       if (__NEXUS_VideoDecoder_SetUserDataFormatFilter==NULL) {
          __NEXUS_VideoDecoder_SetUserDataFormatFilter = (NEXUS_Error (*)(NEXUS_VideoDecoderHandle, NEXUS_UserDataFormat))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoDecoder_SetUserDataFormatFilter) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoDecoder_SetUserDataFormatFilter()");
       NEXUS_Error ret = __NEXUS_VideoDecoder_SetUserDataFormatFilter(handle, format);
       tditrace_ex("@T-NEXUS_VideoDecoder_SetUserDataFormatFilter()");
       return ret;
}

extern "C" void NEXUS_VideoInput_Shutdown(NEXUS_VideoInputHandle input)
{
   static void(*__NEXUS_VideoInput_Shutdown)(NEXUS_VideoInputHandle) = NULL;

       if (__NEXUS_VideoInput_Shutdown==NULL) {
          __NEXUS_VideoInput_Shutdown = (void (*)(NEXUS_VideoInputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoInput_Shutdown) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoInput_Shutdown()");
       __NEXUS_VideoInput_Shutdown(input);
       tditrace_ex("@T-NEXUS_VideoInput_Shutdown()");
}

extern "C" NEXUS_Error NEXUS_VideoInput_GetStatus(NEXUS_VideoInputHandle input, NEXUS_VideoInputStatus *pStatus)
{
   static NEXUS_Error(*__NEXUS_VideoInput_GetStatus)(NEXUS_VideoInputHandle, NEXUS_VideoInputStatus *) = NULL;

       if (__NEXUS_VideoInput_GetStatus==NULL) {
          __NEXUS_VideoInput_GetStatus = (NEXUS_Error (*)(NEXUS_VideoInputHandle, NEXUS_VideoInputStatus *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoInput_GetStatus) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoInput_GetStatus()");
       NEXUS_Error ret = __NEXUS_VideoInput_GetStatus(input, pStatus);
       tditrace_ex("@T-NEXUS_VideoInput_GetStatus()");
       return ret;
}

extern "C" void NEXUS_VideoInput_GetVbiSettings(NEXUS_VideoInputHandle handle, NEXUS_VideoInputVbiSettings *pSettings)
{
   static void (*__NEXUS_VideoInput_GetVbiSettings)(NEXUS_VideoInputHandle, NEXUS_VideoInputVbiSettings *) = NULL;

       if (__NEXUS_VideoInput_GetVbiSettings==NULL) {
          __NEXUS_VideoInput_GetVbiSettings = (void (*)(NEXUS_VideoInputHandle, NEXUS_VideoInputVbiSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoInput_GetVbiSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoInput_GetVbiSettings()");
       __NEXUS_VideoInput_GetVbiSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_VideoInput_GetVbiSettings()");
}

extern "C" NEXUS_Error NEXUS_VideoInput_SetVbiSettings(NEXUS_VideoInputHandle handle, const NEXUS_VideoInputVbiSettings *pSettings)
{
   static NEXUS_Error(*__NEXUS_VideoInput_SetVbiSettings)(NEXUS_VideoInputHandle, const NEXUS_VideoInputVbiSettings *) = NULL;

       if (__NEXUS_VideoInput_SetVbiSettings==NULL) {
          __NEXUS_VideoInput_SetVbiSettings = (NEXUS_Error (*)(NEXUS_VideoInputHandle, const NEXUS_VideoInputVbiSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoInput_SetVbiSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoInput_SetVbiSettings()");
       NEXUS_Error ret = __NEXUS_VideoInput_SetVbiSettings(handle, pSettings);
       tditrace_ex("@T-NEXUS_VideoInput_SetVbiSettings()");
       return ret;
}


#if 0
extern "C" NEXUS_VideoWindowHandle NEXUS_VideoWindow_Open(NEXUS_DisplayHandle display, unsigned windowIndex)
{
   static NEXUS_VideoWindowHandle(*__NEXUS_VideoWindow_Open)(NEXUS_DisplayHandle, uint32_t) = NULL;

       if (__NEXUS_VideoWindow_Open==NULL) {
          __NEXUS_VideoWindow_Open = (NEXUS_VideoWindowHandle (*)(NEXUS_DisplayHandle , uint32_t))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_Open) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_Open()");
       NEXUS_VideoWindowHandle ret = __NEXUS_VideoWindow_Open(display, windowIndex);
       tditrace_ex("@T-NEXUS_VideoWindow_Open()");
       return ret;
}

extern "C" void NEXUS_VideoWindow_GetSettings(NEXUS_VideoWindowHandle window, NEXUS_VideoWindowSettings *pSettings)
{
   static void (*__NEXUS_VideoWindow_GetSettings)(NEXUS_VideoWindowHandle, NEXUS_VideoWindowSettings *) = NULL;

       if (__NEXUS_VideoWindow_GetSettings==NULL) {
          __NEXUS_VideoWindow_GetSettings = (void (*)(NEXUS_VideoWindowHandle , NEXUS_VideoWindowSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_GetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_GetSettings()");
       __NEXUS_VideoWindow_GetSettings(window, pSettings);
       tditrace_ex("@T-NEXUS_VideoWindow_GetSettings()");
}

extern "C" NEXUS_Error NEXUS_VideoWindow_SetSettings(NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowSettings *pSettings)
{
   static NEXUS_Error (*__NEXUS_VideoWindow_SetSettings)(NEXUS_VideoWindowHandle, const NEXUS_VideoWindowSettings *) = NULL;

       if (__NEXUS_VideoWindow_SetSettings==NULL) {
          __NEXUS_VideoWindow_SetSettings = (NEXUS_Error (*)(NEXUS_VideoWindowHandle , const NEXUS_VideoWindowSettings *))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_SetSettings) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_SetSettings()");
       NEXUS_Error ret = __NEXUS_VideoWindow_SetSettings(window, pSettings);
       tditrace_ex("@T-NEXUS_VideoWindow_SetSettings()");
       return ret;
}

extern "C" void NEXUS_VideoWindow_RemoveAllInputs(NEXUS_VideoWindowHandle window)
{
   static void (*__NEXUS_VideoWindow_RemoveAllInputs)(NEXUS_VideoWindowHandle) = NULL;

       if (__NEXUS_VideoWindow_RemoveAllInputs==NULL) {
          __NEXUS_VideoWindow_RemoveAllInputs = (void (*)(NEXUS_VideoWindowHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_RemoveAllInputs) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_RemoveAllInputs()");
       __NEXUS_VideoWindow_RemoveAllInputs(window);
       tditrace_ex("@T-NEXUS_VideoWindow_RemoveAllInputs()");
}

extern "C" void NEXUS_VideoWindow_Close(NEXUS_VideoWindowHandle window)
{
   static void (*__NEXUS_VideoWindow_Close)(NEXUS_VideoWindowHandle) = NULL;

       if (__NEXUS_VideoWindow_Close==NULL) {
          __NEXUS_VideoWindow_Close = (void (*)(NEXUS_VideoWindowHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_Close) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_Close()");
       __NEXUS_VideoWindow_Close(window);
       tditrace_ex("@T-NEXUS_VideoWindow_Close()");
}

extern "C" NEXUS_Error NEXUS_VideoWindow_AddInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInputHandle input)
{
   static NEXUS_Error(*__NEXUS_VideoWindow_AddInput)(NEXUS_VideoWindowHandle, NEXUS_VideoInputHandle) = NULL;

       if (__NEXUS_VideoWindow_AddInput==NULL) {
          __NEXUS_VideoWindow_AddInput = (NEXUS_Error (*)(NEXUS_VideoWindowHandle, NEXUS_VideoInputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_AddInput) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_AddInput()");
       NEXUS_Error ret = __NEXUS_VideoWindow_AddInput(window, input);
       tditrace_ex("@T-NEXUS_VideoWindow_AddInput()");
       return ret;
}

extern "C" NEXUS_Error NEXUS_VideoWindow_RemoveInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInputHandle input)
{
   static NEXUS_Error(*__NEXUS_VideoWindow_RemoveInput)(NEXUS_VideoWindowHandle, NEXUS_VideoInputHandle) = NULL;

       if (__NEXUS_VideoWindow_RemoveInput==NULL) {
          __NEXUS_VideoWindow_RemoveInput = (NEXUS_Error (*)(NEXUS_VideoWindowHandle, NEXUS_VideoInputHandle))dlsym(RTLD_NEXT, __func__);
           if (NULL == __NEXUS_VideoWindow_RemoveInput) {
               fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
           }
       }

       tditrace_ex("@T+NEXUS_VideoWindow_RemoveInput()");
       NEXUS_Error ret = __NEXUS_VideoWindow_RemoveInput(window, input);
       tditrace_ex("@T-NEXUS_VideoWindow_RemoveInput()");
       return ret;
}
#endif


extern "C" NEXUS_VideoInputHandle NEXUS_VideoInput_Create (void)
{
   static NEXUS_VideoInputHandle (* __NEXUS_VideoInput_Create) (void) = NULL;
   if (__NEXUS_VideoInput_Create == NULL) {
      __NEXUS_VideoInput_Create = (NEXUS_VideoInputHandle (*) (void)) dlsym(RTLD_NEXT, "NEXUS_VideoInput_Create");
      if (__NEXUS_VideoInput_Create == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoInput_Create()");
   NEXUS_VideoInputHandle ret = __NEXUS_VideoInput_Create ();
   tditrace_ex("@T-NEXUS_VideoInput_Create()");
   return ret;
}

extern "C" void NEXUS_PidChannel_ConsumerStarted ( NEXUS_PidChannelHandle pidChannel )
{
   static void (* __NEXUS_PidChannel_ConsumerStarted) ( NEXUS_PidChannelHandle ) = NULL;
   if (__NEXUS_PidChannel_ConsumerStarted == NULL) {
      __NEXUS_PidChannel_ConsumerStarted = (void (*) ( NEXUS_PidChannelHandle )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_ConsumerStarted");
      if (__NEXUS_PidChannel_ConsumerStarted == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_ConsumerStarted()");
   __NEXUS_PidChannel_ConsumerStarted ( pidChannel );
   tditrace_ex("@T-NEXUS_PidChannel_ConsumerStarted()");
}

extern "C" void NEXUS_PidChannel_RemoveAllSplicePidChannels ( NEXUS_PidChannelHandle pidChannel )
{
   static void (* __NEXUS_PidChannel_RemoveAllSplicePidChannels) ( NEXUS_PidChannelHandle ) = NULL;
   if (__NEXUS_PidChannel_RemoveAllSplicePidChannels == NULL) {
      __NEXUS_PidChannel_RemoveAllSplicePidChannels = (void (*) ( NEXUS_PidChannelHandle )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_RemoveAllSplicePidChannels");
      if (__NEXUS_PidChannel_RemoveAllSplicePidChannels == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_RemoveAllSplicePidChannels()");
   __NEXUS_PidChannel_RemoveAllSplicePidChannels ( pidChannel );
   tditrace_ex("@T-NEXUS_PidChannel_RemoveAllSplicePidChannels()");
}

extern "C" void NEXUS_PidChannel_GetDefaultSettings ( NEXUS_PidChannelSettings * pSettings )
{
   static void (* __NEXUS_PidChannel_GetDefaultSettings) ( NEXUS_PidChannelSettings * ) = NULL;
   if (__NEXUS_PidChannel_GetDefaultSettings == NULL) {
      __NEXUS_PidChannel_GetDefaultSettings = (void (*) ( NEXUS_PidChannelSettings * )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_GetDefaultSettings");
      if (__NEXUS_PidChannel_GetDefaultSettings == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_GetDefaultSettings()");
   __NEXUS_PidChannel_GetDefaultSettings ( pSettings );
   tditrace_ex("@T-NEXUS_PidChannel_GetDefaultSettings()");
}

extern "C" NEXUS_Error NEXUS_PidChannel_RemoveSplicePidChannel ( NEXUS_PidChannelHandle pidChannel ,  NEXUS_PidChannelHandle splicePidChannel )
{
   static NEXUS_Error (* __NEXUS_PidChannel_RemoveSplicePidChannel) ( NEXUS_PidChannelHandle ,  NEXUS_PidChannelHandle ) = NULL;
   if (__NEXUS_PidChannel_RemoveSplicePidChannel == NULL) {
      __NEXUS_PidChannel_RemoveSplicePidChannel = (NEXUS_Error (*) ( NEXUS_PidChannelHandle ,  NEXUS_PidChannelHandle )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_RemoveSplicePidChannel");
      if (__NEXUS_PidChannel_RemoveSplicePidChannel == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_RemoveSplicePidChannel()");
   NEXUS_Error ret = __NEXUS_PidChannel_RemoveSplicePidChannel ( pidChannel ,  splicePidChannel );
   tditrace_ex("@T-NEXUS_PidChannel_RemoveSplicePidChannel()");
   return ret;
}

extern "C" NEXUS_PidChannelHandle NEXUS_PidChannel_Open ( NEXUS_ParserBand parserBand ,  uint16_t pid ,  const NEXUS_PidChannelSettings * pSettings )
{
   static NEXUS_PidChannelHandle (* __NEXUS_PidChannel_Open) ( NEXUS_ParserBand ,  uint16_t ,  const NEXUS_PidChannelSettings * ) = NULL;
   if (__NEXUS_PidChannel_Open == NULL) {
      __NEXUS_PidChannel_Open = (NEXUS_PidChannelHandle (*) ( NEXUS_ParserBand ,  uint16_t ,  const NEXUS_PidChannelSettings * )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_Open");
      if (__NEXUS_PidChannel_Open == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_Open()");
   NEXUS_PidChannelHandle ret = __NEXUS_PidChannel_Open ( parserBand ,  pid ,  pSettings );
   tditrace_ex("@T-NEXUS_PidChannel_Open()");
   return ret;
}

extern "C" NEXUS_Error NEXUS_PidChannel_AddSplicePidChannel ( NEXUS_PidChannelHandle pidChannel ,  NEXUS_PidChannelHandle splicePidChannel )
{
   static NEXUS_Error (* __NEXUS_PidChannel_AddSplicePidChannel) ( NEXUS_PidChannelHandle ,  NEXUS_PidChannelHandle ) = NULL;
   if (__NEXUS_PidChannel_AddSplicePidChannel == NULL) {
      __NEXUS_PidChannel_AddSplicePidChannel = (NEXUS_Error (*) ( NEXUS_PidChannelHandle ,  NEXUS_PidChannelHandle )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_AddSplicePidChannel");
      if (__NEXUS_PidChannel_AddSplicePidChannel == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_AddSplicePidChannel()");
   NEXUS_Error ret = __NEXUS_PidChannel_AddSplicePidChannel ( pidChannel ,  splicePidChannel );
   tditrace_ex("@T-NEXUS_PidChannel_AddSplicePidChannel()");
   return ret;
}

extern "C" NEXUS_Error NEXUS_PidChannel_GetStatus ( NEXUS_PidChannelHandle pidChannel ,  NEXUS_PidChannelStatus * pStatus )
{
   static NEXUS_Error (* __NEXUS_PidChannel_GetStatus) ( NEXUS_PidChannelHandle ,  NEXUS_PidChannelStatus * ) = NULL;
   if (__NEXUS_PidChannel_GetStatus == NULL) {
      __NEXUS_PidChannel_GetStatus = (NEXUS_Error (*) ( NEXUS_PidChannelHandle ,  NEXUS_PidChannelStatus * )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_GetStatus");
      if (__NEXUS_PidChannel_GetStatus == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_GetStatus()");
   NEXUS_Error ret = __NEXUS_PidChannel_GetStatus ( pidChannel ,  pStatus );
   tditrace_ex("@T-NEXUS_PidChannel_GetStatus()");
   return ret;
}

extern "C" NEXUS_Error NEXUS_PidChannel_SetRemapSettings ( NEXUS_PidChannelHandle pidChannel ,  const NEXUS_PidChannelRemapSettings * pSettings )
{
   static NEXUS_Error (* __NEXUS_PidChannel_SetRemapSettings) ( NEXUS_PidChannelHandle ,  const NEXUS_PidChannelRemapSettings * ) = NULL;
   if (__NEXUS_PidChannel_SetRemapSettings == NULL) {
      __NEXUS_PidChannel_SetRemapSettings = (NEXUS_Error (*) ( NEXUS_PidChannelHandle ,  const NEXUS_PidChannelRemapSettings * )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_SetRemapSettings");
      if (__NEXUS_PidChannel_SetRemapSettings == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_SetRemapSettings()");
   NEXUS_Error ret = __NEXUS_PidChannel_SetRemapSettings ( pidChannel ,  pSettings );
   tditrace_ex("@T-NEXUS_PidChannel_SetRemapSettings()");
   return ret;
}

extern "C" void NEXUS_PidChannel_CloseAll ( NEXUS_ParserBand parserBand )
{
   static void (* __NEXUS_PidChannel_CloseAll) ( NEXUS_ParserBand ) = NULL;
   if (__NEXUS_PidChannel_CloseAll == NULL) {
      __NEXUS_PidChannel_CloseAll = (void (*) ( NEXUS_ParserBand )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_CloseAll");
      if (__NEXUS_PidChannel_CloseAll == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_CloseAll()");
   __NEXUS_PidChannel_CloseAll ( parserBand );
   tditrace_ex("@T-NEXUS_PidChannel_CloseAll()");
}

extern "C" NEXUS_Error NEXUS_PidChannel_SetEnabled ( NEXUS_PidChannelHandle pidChannel ,  bool enabled )
{
   static NEXUS_Error (* __NEXUS_PidChannel_SetEnabled) ( NEXUS_PidChannelHandle ,  bool ) = NULL;
   if (__NEXUS_PidChannel_SetEnabled == NULL) {
      __NEXUS_PidChannel_SetEnabled = (NEXUS_Error (*) ( NEXUS_PidChannelHandle ,  bool )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_SetEnabled");
      if (__NEXUS_PidChannel_SetEnabled == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_SetEnabled()");
   NEXUS_Error ret = __NEXUS_PidChannel_SetEnabled ( pidChannel ,  enabled );
   tditrace_ex("@T-NEXUS_PidChannel_SetEnabled()");
   return ret;
}

extern "C" void NEXUS_VideoInput_Destroy ( NEXUS_VideoInputHandle videoInput )
{
   static void (* __NEXUS_VideoInput_Destroy) ( NEXUS_VideoInputHandle ) = NULL;
   if (__NEXUS_VideoInput_Destroy == NULL) {
      __NEXUS_VideoInput_Destroy = (void (*) ( NEXUS_VideoInputHandle )) dlsym(RTLD_NEXT, "NEXUS_VideoInput_Destroy");
      if (__NEXUS_VideoInput_Destroy == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_VideoInput_Destroy()");
   __NEXUS_VideoInput_Destroy ( videoInput );
   tditrace_ex("@T-NEXUS_VideoInput_Destroy()");
}

extern "C" void NEXUS_PidChannel_Close ( NEXUS_PidChannelHandle pidChannel )
{
   static void (* __NEXUS_PidChannel_Close) ( NEXUS_PidChannelHandle ) = NULL;
   if (__NEXUS_PidChannel_Close == NULL) {
      __NEXUS_PidChannel_Close = (void (*) ( NEXUS_PidChannelHandle )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_Close");
      if (__NEXUS_PidChannel_Close == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_Close()");
   __NEXUS_PidChannel_Close ( pidChannel );
   tditrace_ex("@T-NEXUS_PidChannel_Close()");
}

extern "C" NEXUS_Error NEXUS_PidChannel_ResetStatus ( NEXUS_PidChannelHandle pidChannel )
{
   static NEXUS_Error (* __NEXUS_PidChannel_ResetStatus) ( NEXUS_PidChannelHandle ) = NULL;
   if (__NEXUS_PidChannel_ResetStatus == NULL) {
      __NEXUS_PidChannel_ResetStatus = (NEXUS_Error (*) ( NEXUS_PidChannelHandle )) dlsym(RTLD_NEXT, "NEXUS_PidChannel_ResetStatus");
      if (__NEXUS_PidChannel_ResetStatus == NULL) {
         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
      }
   }
   tditrace_ex("@T+NEXUS_PidChannel_ResetStatus()");
   NEXUS_Error ret = __NEXUS_PidChannel_ResetStatus ( pidChannel );
   tditrace_ex("@T-NEXUS_PidChannel_ResetStatus()");
   return ret;
}
