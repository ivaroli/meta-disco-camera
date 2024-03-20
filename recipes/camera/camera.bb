SUMMARY = "DISCO 2 Camera control software"
SECTION = "camera"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://github.com/ivaroli/DiscoCameraController.git;branch=main;rev=fc150f1a69c8b10e5f2fc5e76d1a9bcb8a9cc47d"

SRC_URI += " \
    git://github.com/spaceinventor/libcsp.git;protocol=https;destsuffix=git/lib/csp;name=libcsp;branch=master;rev=544635f292b7a15ea46b95cd2861102129c329e7 \
"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = "-DCMAKE_BUILD_TYPE=Release"

RDEPENDS:${PN} += "libcsp"
DEPENDS += "libcsp"
do_configure[depends] += "libcsp:do_populate_sysroot"

OECMAKE_FIND_ROOT_PATH_MODE_PROGRAM = "BOTH"

PROVIDES = " libVmbCPP"
RPROVIDES_${PN} += " libVmbCPP.so()(64bit)"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_install(){
    install -d ${D}${libdir}
    install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/lib
    install -d 644 ${D}${sysconfdir}/profile.d

    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64/api/lib/*.so ${D}${libdir}
    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64/api/lib/GenICam/*.so ${D}${libdir}
    install ${WORKDIR}/git/build/Disco2CameraControl ${D}${bindir}

    cp -r ${WORKDIR}/git/lib/VimbaX_2023-4-ARM64 ${D}${sysconfdir}/lib/VimbaX_2023-4-ARM64
    chmod 447 ${D}${sysconfdir}/lib/VimbaX_2023-4-ARM64/cti/VimbaUSBTL_Install.sh
}

FILES_${PN} += "${libdir}/libVmbCPP.so"
FILES_${PN} += "${libdir}/libVmbC.so"
FILES:${PN} += "/usr/csp /usr/csp/csp_autoconfig.h"

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"