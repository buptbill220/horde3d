LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
./egAnimatables.cpp \
./egAnimation.cpp \
./egCamera.cpp \
./egCom.cpp \
./egExtensions.cpp \
./egGeometry.cpp \
./egLight.cpp \
./egMain.cpp \
./egMaterial.cpp \
./egModel.cpp \
./egModules.cpp \
./egParticle.cpp \
./egPipeline.cpp \
./egPrimitives.cpp \
./egRenderer.cpp \
./egRendererBase.cpp \
./egResource.cpp \
./egScene.cpp \
./egSceneGraphRes.cpp \
./egShader.cpp \
./egTexture.cpp \
./utImage.cpp \
./utTexture.cpp \
./GLES2/egRendererBaseGLES2.cpp \
./GLES2/utOpenGLES2.cpp \
./egBackground.cpp


LOCAL_EXPORT_C_INCLUDES :=  $(LOCAL_PATH)/

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/GLES2 \
                    $(LOCAL_PATH)/../Shared \
                    $(LOCAL_PATH)/../Terrain

LOCAL_CFLAGS := -Wno-psabi -D__ANDROID__ -D__DEBUG__ -DHORDE3D_GLES2 -DPLATFORM_ANDROID
LLOCAL_EXPORT_CFLAGS:= -Wno-psabi -D__ANDROID__ -D__DEBUG__ -DHORDE3D_GLES2 -DPLATFORM_ANDROID

LOCAL_LDLIBS := -lGLESv3 \
                -lGLESv2 \
                -llog \
                -lz

LOCAL_MODULE := horde3d

include $(BUILD_STATIC_LIBRARY)
