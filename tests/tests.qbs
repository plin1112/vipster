import qbs 1.0

CppApplication {
    name: "test_libvipster"
    condition: !project.webBuild
    files: ["*.cpp"]
    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.defines: "DO_NOT_USE_WMAIN"
    }
    Depends { name: "libvipster" }
}
