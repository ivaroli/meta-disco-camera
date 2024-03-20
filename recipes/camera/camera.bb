SUMMARY = "DISCO 2 Camera control software"
SECTION = "camera"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS = "libcsp"

SRC_URI = "git://github.com/ivaroli/DiscoCameraController.git;branch=main;rev=27f36cecc82f766c933d1a202fc0a6f221c07bb9"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = "-DCMAKE_BUILD_TYPE=Release"

PROVIDES = " libVmbCPP"
RPROVIDES_${PN} += " libVmbCPP.so()(64bit)"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_install(){
    install -d ${D}${libdir}
    install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/lib
    install -d 644 ${D}${sysconfdir}/profile.d

    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-1/api/lib/*.so ${D}${libdir}
    install -m 0644 ${WORKDIR}/git/lib/VimbaX_2023-1/api/lib/GenICam/*.so ${D}${libdir}
    install ${WORKDIR}/git/build/Disco2CameraControl ${D}${bindir}

    cp -r ${WORKDIR}/git/lib/VimbaX_2023-1 ${D}${sysconfdir}/lib/VimbaX_2023-1
    chmod 447 ${D}${sysconfdir}/lib/VimbaX_2023-1/cti/VimbaUSBTL_Install.sh
}

FILES_${PN} += "${libdir}/libVmbCPP.so"
FILES_${PN} += "${libdir}/libVmbC.so"

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"