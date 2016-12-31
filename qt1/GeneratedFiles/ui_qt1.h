/********************************************************************************
** Form generated from reading UI file 'qt1.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QT1_H
#define UI_QT1_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qt1Class
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *qt1Class)
    {
        if (qt1Class->objectName().isEmpty())
            qt1Class->setObjectName(QStringLiteral("qt1Class"));
        qt1Class->resize(600, 400);
        menuBar = new QMenuBar(qt1Class);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        qt1Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(qt1Class);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        qt1Class->addToolBar(mainToolBar);
        centralWidget = new QWidget(qt1Class);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        qt1Class->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(qt1Class);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        qt1Class->setStatusBar(statusBar);

        retranslateUi(qt1Class);

        QMetaObject::connectSlotsByName(qt1Class);
    } // setupUi

    void retranslateUi(QMainWindow *qt1Class)
    {
        qt1Class->setWindowTitle(QApplication::translate("qt1Class", "qt1", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class qt1Class: public Ui_qt1Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QT1_H
