SUMMARY = "DISCO 2 Camera control software"
SECTION = "camera"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://github.com/ivaroli/DiscoCameraController.git;branch=main;rev=ed5bd826418185b8ddfd307b3d492ee15795588f;protocol=https"

SRC_URI += " \
    git://github.com/spaceinventor/libcsp.git;protocol=https;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=544635f292b7a15ea46b95cd2861102129c329e7 \
    git://github.com/spaceinventor/libparam.git;protocol=https;destsuffix=git/lib/param;name=libparam;branch=master;rev=fdf62e155a965df99a1012174677c6f2958a7e4f \
"

S = "${WORKDIR}/git"

inherit meson pkgconfig

RDEPENDS:${PN} += "zeromq can-utils libcsp libbsd pkgconfig meson-native"
DEPENDS += "curl openssl libsocketcan can-utils zeromq libyaml meson-native ninja-native pkgconfig python3-pip-native elfutils libbsd glibc"

PROVIDES += " libVmbCPP"
RPROVIDES_${PN} += " libVmbCPP.so()(64bit)"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_configure:prepend() {
    cat > ${WORKDIR}/cross.txt <<EOF
[binaries]
c = '${TARGET_PREFIX}gcc'
cpp = '${TARGET_PREFIX}g++'
ar = '${TARGET_PREFIX}ar'
strip = '${TARGET_PREFIX}strip'
pkgconfig = 'pkg-config'
[properties]
needs_exe_wrapper = true
EOF
}



do_install(){
    install -d ${D}${libdir}
    install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/lib
    install -d 644 ${D}${sysconfdir}/profile.d

    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64/api/lib/*.so ${D}${libdir}
    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64/api/lib/GenICam/*.so ${D}${libdir}
    install ${WORKDIR}/build/Disco2CameraControl ${D}${bindir}

    cp -r ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64 ${D}${sysconfdir}/lib/VimbaX_2023-4-ARM64
    chmod 447 ${D}${sysconfdir}/lib/VimbaX_2023-4-ARM64/cti/VimbaUSBTL_Install.sh
}
INHIBIT_PACKAGE_STRIP = "1"
INSANE_SKIP:${PN} += "already-stripped"

FILES_${PN} += "${libdir}/libVmbCPP.so"
FILES_${PN} += "${libdir}/libVmbC.so"
FILES:${PN} += "/usr/csp /usr/csp/csp_autoconfig.h"

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"
