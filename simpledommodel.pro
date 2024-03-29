HEADERS     = domitem.h \
              dommodel.h \
              mainwindow.h \
    fieldtype.h
SOURCES     = domitem.cpp \
              dommodel.cpp \
              main.cpp \
              mainwindow.cpp
QT      += xml widgets
requires(qtConfig(filedialog))

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/itemviews/simpledommodel
INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    uml_classdiagram.qmodel

