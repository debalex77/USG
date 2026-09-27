CONFIG += use_lld_linker

win32 {
    # Necesare pentru simbolurile Qt/LimeReport exportate pe Windows.
    HEADERS += \
        $$USG_ROOT/3rdparty/LimeReport/include/lrcallbackdatasourceintf.h \
        $$USG_ROOT/3rdparty/LimeReport/include/lrpreviewreportwidget.h \
        $$USG_ROOT/3rdparty/LimeReport/include/lrreportengine.h
}

macx {
    CONFIG += app_bundle
}

# LimeReport
INCLUDEPATH += $$USG_ROOT/3rdparty/LimeReport/include
DEPENDPATH  += $$USG_ROOT/3rdparty/LimeReport/include
INCLUDEPATH += $$USG_ROOT/3rdparty/LimeReport/debug
INCLUDEPATH += $$USG_ROOT/3rdparty/LimeReport/release

unix:!macx {
    QMAKE_LFLAGS += -Wl,--disable-new-dtags
    QMAKE_RPATHDIR += $$USG_ROOT/3rdparty/LimeReport/debug
    QMAKE_RPATHDIR += $$USG_ROOT/3rdparty/LimeReport/release
}

CONFIG(debug, debug|release) {
    LIBS += -L$$USG_ROOT/3rdparty/LimeReport/debug/ -llimereportd
} else {
    LIBS += -L$$USG_ROOT/3rdparty/LimeReport/release/ -llimereport
}

CONFIG(debug, debug|release) {
    unix:!macx {
        LIBS += -Wl,--no-as-needed -L$$USG_ROOT/3rdparty/LimeReport/debug/ -lQtZintd -Wl,--as-needed
    } else {
        LIBS += -L$$USG_ROOT/3rdparty/LimeReport/debug/ -lQtZintd
    }
} else {
    unix:!macx {
        LIBS += -Wl,--no-as-needed -L$$USG_ROOT/3rdparty/LimeReport/release/ -lQtZint -Wl,--as-needed
    } else {
        LIBS += -L$$USG_ROOT/3rdparty/LimeReport/release/ -lQtZint
    }
}

macx {
    LIBS += -L$$USG_ROOT/3rdparty/LimeReport/debug/ -llimereportd -lQtZintd
    LIBS += -L$$USG_ROOT/3rdparty/LimeReport/release/ -llimereport -lQtZint
}

!CONFIG(static_build):CONFIG(zint) {
    isEmpty(DEST_LIBS): DEST_LIBS = $$USG_ROOT/3rdparty/LimeReport/release
    CONFIG(debug, debug|release) {
        LIBS += -L$${DEST_LIBS} -lQtZintd
    } else {
        LIBS += -L$${DEST_LIBS} -lQtZint
    }
}

# OpenSSL
OPENSSL_DIR = $$USG_ROOT/3rdparty/openssl
INCLUDEPATH += $$OPENSSL_DIR

win32 {
    INCLUDEPATH += $$OPENSSL_DIR/include

    LIBS += /LIBPATH:$$OPENSSL_DIR/lib \
            libssl.lib \
            libcrypto.lib \
            crypt32.lib \
            ws2_32.lib \
            advapi32.lib \
            user32.lib
}

unix:!macx {
    LIBS += -L$$OPENSSL_DIR -lssl -lcrypto
}

macx {
    LIBS += -L$$OPENSSL_DIR -lssl -lcrypto
}

win32 {
    QMAKE_PROJECT_DEPTH = 0
}
