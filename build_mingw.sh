#!/bin/bash

# 设置路径，与 CMakeLists.txt 保持一致
MPV_SOURCE_DIR="/d/github/qt_mpv/mpv"
MPV_BUILD_DIR="/d/github/qt_mpv/build/mpv-build"
MPV_INSTALL_DIR="/d/github/qt_mpv/build/mpv-install"
MINGW_PREFIX="/d/msys64/mingw64"

echo "开始编译 mpv..."
echo "源目录: ${MPV_SOURCE_DIR}"
echo "构建目录: ${MPV_BUILD_DIR}"
echo "安装目录: ${MPV_INSTALL_DIR}"

# 检查源码目录是否存在
if [ ! -d "${MPV_SOURCE_DIR}" ]; then
    echo "错误: mpv 源码目录不存在: ${MPV_SOURCE_DIR}"
    exit 1
fi

# 清理并创建构建目录
echo "清理旧构建..."
rm -rf "${MPV_BUILD_DIR}"
rm -rf "${MPV_INSTALL_DIR}"
mkdir -p "${MPV_BUILD_DIR}"
mkdir -p "${MPV_INSTALL_DIR}"

# 进入构建目录
cd "${MPV_BUILD_DIR}"

# 配置 mpv
echo "配置 mpv..."
meson setup . "${MPV_SOURCE_DIR}" \
    --prefix="${MPV_INSTALL_DIR}" \
    --buildtype=release \
    -Dlibmpv=true \
    -Dcplayer=false \
    -Dbuild-date=false \
    -Dmanpage-build=disabled \
    -Dpdf-build=disabled \
    -Dlua=disabled \
    -Djavascript=disabled \
    -Dlibarchive=disabled \
    -Dlibbluray=disabled \
    -Ddvdnav=disabled \
    -Duchardet=disabled \
    -Drubberband=disabled \
    -Dlcms2=disabled \
    -Dopenal=disabled \
    -Dspirv-cross=disabled \
    -Dvulkan=disabled \
    -Dshaderc=disabled \
    -Dgl=enabled \
    -Dvdpau=disabled \
    -Dvaapi=disabled \
    -Dwayland=disabled \
    -Dxv=disabled \
    -Dsdl2=disabled \
    -Degl-angle=disabled \
    -Degl-angle-win32=disabled \
    -Dplain-gl=enabled \
    -Dtests=false

# 编译
echo "编译 mpv..."
ninja

# 安装
echo "安装 mpv..."
ninja install

echo "编译完成!"
echo "库文件位置: ${MPV_INSTALL_DIR}/lib/libmpv.dll.a"
echo "DLL 位置: ${MPV_INSTALL_DIR}/bin/libmpv-2.dll"

# 复制 DLL 到构建目录（与 CMakeLists.txt 一致）
echo "复制 DLL 到构建目录..."
cp "${MPV_INSTALL_DIR}/bin/libmpv-2.dll" "/d/github/qt_mpv/build/"

echo "mpv 编译和安装完成!"