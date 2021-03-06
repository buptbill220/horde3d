#!/bin/bash

APPNAME="Chicago"

# options

buildexternalsfromsource=

usage(){
cat << EOF
usage: $0 [options]

Build C/C++ code for $APPNAME using Android NDK

OPTIONS:
-s	Build externals from source
-h	this help
EOF
}

while getopts "sh" OPTION; do
case "$OPTION" in
s)
buildexternalsfromsource=1
;;
h)
usage
exit 0
;;
esac
done

# paths

if [ -z "${NDK_ROOT+aaa}" ];then
echo "please define NDK_ROOT"
exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# ... use paths relative to current directory
ASSETS_ROOT="$DIR/../../../Binaries/Content"
APP_ANDROID_ROOT="$DIR"
HORDE3D_ROOT="$DIR/../../../.."

echo "NDK_ROOT = $NDK_ROOT"
echo "HORDE3D_ROOT = $HORDE3D_ROOT"
echo "ASSETS_ROOT = $ASSETS_ROOT"
echo "APP_ANDROID_ROOT = $APP_ANDROID_ROOT"

# make sure assets is exist
if [ -d "$APP_ANDROID_ROOT"/assets ]; then
    rm -rf "$APP_ANDROID_ROOT"/assets
fi

mkdir "$APP_ANDROID_ROOT"/assets
mkdir "$APP_ANDROID_ROOT"/assets/Content

# copy resources
for file in "$ASSETS_ROOT"/*
do
if [ -d "$file" ]; then
    cp -rf "$file" "$APP_ANDROID_ROOT"/assets/Content
fi

if [ -f "$file" ]; then
    cp "$file" "$APP_ANDROID_ROOT"/assets/Content
fi
done

# run ndk-build
    "$NDK_ROOT"/ndk-build V=1 -C "$APP_ANDROID_ROOT" $* \
	   "NDK_MODULE_PATH=${HORDE3D_ROOT}"
#        "NDK_MODULE_PATH=${COCOS2DX_ROOT}:${COCOS2DX_ROOT}/cocos2dx/platform/third_party/android/prebuilt"
