/********************************************************************************
** Form generated from reading UI file 'zoomui.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ZOOMUI_H
#define UI_ZOOMUI_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsPixmapItem>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGraphicsScene *graphicsScene;
    QGraphicsView *graphicsView;
    QGraphicsPixmapItem *pixmap;
    QPixmap imgpix;
    QFrame *frame;
    QPushButton *pushButton;
    QPushButton *endButton;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1280, 720);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        graphicsScene = new QGraphicsScene;
        pixmap = new QGraphicsPixmapItem;
        graphicsScene->addItem(pixmap);
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->setGeometry(QRect(0, 0, 1280, 640));
        graphicsView->setScene(graphicsScene);
        frame = new QFrame(centralwidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(0, 639, 1281, 102));
        frame->setStyleSheet(QString::fromUtf8("background-color: gray"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        pushButton = new QPushButton(frame);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(0, 0, 91, 81));
        pushButton->setAutoFillBackground(false);
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("audio-input-microphone")));
        pushButton->setIcon(icon);
        pushButton->setIconSize(QSize(32, 32));
        endButton = new QPushButton(frame);
        endButton->setObjectName(QString::fromUtf8("endButton"));
        endButton->setGeometry(QRect(1150, 20, 101, 41));
        endButton->setStyleSheet(QString::fromUtf8("background-color: red;\n"
                                                   "color: rgb(255, 255, 255);"));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton->setText(QString());
        endButton->setText(QCoreApplication::translate("MainWindow", "End", nullptr));
    } // retranslateUi
};

namespace Ui
{
    class MainWindow : public Ui_MainWindow
    {
    };
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ZOOMUI_H
