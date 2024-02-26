SUMMARY = "DISCO 2 Camera control software"
SECTION = "camera"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS += "opencv"

SRCREV = "${AUTOREV}"
SRC_URI = "git://github.com/ivaroli/meta-disco-camera-files.git;branch=main"

S = "${WORKDIR}"

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

    install -m 0644 ${WORKDIR}/lib/VimbaX_2023-1/api/lib/*.so ${D}${libdir}
    install -m 0644 ${WORKDIR}/lib/VimbaX_2023-1/api/lib/GenICam/*.so ${D}${libdir}
    install ${WORKDIR}/build/Disco2CameraControl ${D}${bindir}

    cp -r ${WORKDIR}/lib/VimbaX_2023-1 ${D}${sysconfdir}/lib/VimbaX_2023-1
    chmod 447 ${D}${sysconfdir}/lib/VimbaX_2023-1/cti/VimbaUSBTL_Install.sh
}

FILES_${PN} += "${libdir}/libVmbCPP.so"
FILES_${PN} += "${libdir}/libVmbC.so"

do_package_qa[noexec] = "1"
EXCLUDE_FROM_SHLIBS = "1"