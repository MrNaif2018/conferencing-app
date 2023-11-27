#ifndef STARTWINDOW_H
#define STARTWINDOW_H
#include <QWidget>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class Ui_StartWindow
{
public:
    QLabel *label;
    QLabel *label_2;
    QLineEdit *ipLabel;
    QPushButton *connectButton;
    QPushButton *settingsButton;

    void setupUi(QWidget *StartWindow)
    {
        if (StartWindow->objectName().isEmpty())
            StartWindow->setObjectName(QString::fromUtf8("StartWindow"));
        StartWindow->resize(1280, 720);
        label = new QLabel(StartWindow);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(510, 10, 241, 101));
        QFont font;
        font.setPointSize(15);
        font.setBold(true);
        label->setFont(font);
        label->setTextFormat(Qt::AutoText);
        label->setAlignment(Qt::AlignCenter);
        label_2 = new QLabel(StartWindow);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(510, 270, 251, 51));
        QFont font1;
        font1.setPointSize(15);
        label_2->setFont(font1);
        label_2->setAlignment(Qt::AlignCenter);
        ipLabel = new QLineEdit(StartWindow);
        ipLabel->setObjectName(QString::fromUtf8("ipLabel"));
        ipLabel->setGeometry(QRect(520, 350, 231, 41));
        ipLabel->setAlignment(Qt::AlignCenter);
        connectButton = new QPushButton(StartWindow);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));
        connectButton->setGeometry(QRect(600, 420, 80, 23));
        connectButton->setStyleSheet(QString::fromUtf8("background-color: rgb(87, 227, 137);"));
        settingsButton = new QPushButton(StartWindow);
        settingsButton->setObjectName(QString::fromUtf8("settingsButton"));
        settingsButton->setGeometry(QRect(1170, 640, 61, 51));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/config.png"), QSize(), QIcon::Normal, QIcon::Off);
        settingsButton->setIcon(icon);
        settingsButton->setIconSize(QSize(32, 32));

        retranslateUi(StartWindow);

        QMetaObject::connectSlotsByName(StartWindow);
    } // setupUi

    void retranslateUi(QWidget *StartWindow)
    {
        StartWindow->setWindowTitle(QCoreApplication::translate("StartWindow", "Video conferencing", nullptr));
        label->setText(QCoreApplication::translate("StartWindow", "P2P Videoconferencing", nullptr));
        label_2->setText(QCoreApplication::translate("StartWindow", "Connect to server:", nullptr));
        ipLabel->setPlaceholderText(QCoreApplication::translate("StartWindow", "IP address", nullptr));
        connectButton->setText(QCoreApplication::translate("StartWindow", "Connect", nullptr));
        settingsButton->setText(QString());
    } // retranslateUi
};

class StartWindow : public QWidget, public Ui_StartWindow
{
    Q_OBJECT

public:
    StartWindow(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUi(this);
    }
};
#endif