#ifndef UI_SETTINGSWINDOW_H
#define UI_SETTINGSWINDOW_H
#include <QtCore/QVariant>
#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include "config.h"

class CustomSpinbox : public QSpinBox
{
    Q_OBJECT
public:
    CustomSpinbox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        lineEdit()->setReadOnly(true);
    }
};

class Ui_SettingsWindow
{
public:
    QLabel *label;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QSpinBox *fpsVal;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    CustomSpinbox *packetVal;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QSpinBox *qualityVal;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    QPushButton *saveButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *SettingsWindow)
    {
        if (SettingsWindow->objectName().isEmpty())
            SettingsWindow->setObjectName(QString::fromUtf8("SettingsWindow"));
        SettingsWindow->resize(800, 600);
        label = new QLabel(SettingsWindow);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(330, 20, 101, 41));
        QFont font;
        font.setPointSize(15);
        font.setBold(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        verticalLayoutWidget = new QWidget(SettingsWindow);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(70, 160, 271, 241));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QFont font1;
        font1.setPointSize(12);
        label_2->setFont(font1);

        horizontalLayout->addWidget(label_2);

        fpsVal = new QSpinBox(verticalLayoutWidget);
        fpsVal->setObjectName(QString::fromUtf8("fpsVal"));
        fpsVal->setMinimum(1);
        fpsVal->setMaximum(60);

        horizontalLayout->addWidget(fpsVal);

        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(verticalLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font1);

        horizontalLayout_2->addWidget(label_3);

        packetVal = new CustomSpinbox(verticalLayoutWidget);
        packetVal->setObjectName(QString::fromUtf8("packetVal"));
        packetVal->setMinimum(1024);
        packetVal->setMaximum(60416);
        packetVal->setSingleStep(1024);

        horizontalLayout_2->addWidget(packetVal);

        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_4 = new QLabel(verticalLayoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font1);

        horizontalLayout_3->addWidget(label_4);

        qualityVal = new QSpinBox(verticalLayoutWidget);
        qualityVal->setObjectName(QString::fromUtf8("qualityVal"));
        qualityVal->setMinimum(1);
        qualityVal->setMaximum(100);

        horizontalLayout_3->addWidget(qualityVal);

        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayoutWidget_4 = new QWidget(SettingsWindow);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(-1, 510, 801, 80));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        saveButton = new QPushButton(horizontalLayoutWidget_4);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(saveButton->sizePolicy().hasHeightForWidth());
        saveButton->setSizePolicy(sizePolicy);
        saveButton->setMinimumSize(QSize(200, 35));

        horizontalLayout_4->addWidget(saveButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        retranslateUi(SettingsWindow);

        QMetaObject::connectSlotsByName(SettingsWindow);
    } // setupUi

    void retranslateUi(QWidget *SettingsWindow)
    {
        SettingsWindow->setWindowTitle(QCoreApplication::translate("SettingsWindow", "Settings", nullptr));
        label->setText(QCoreApplication::translate("SettingsWindow", "Settings", nullptr));
        label_2->setText(QCoreApplication::translate("SettingsWindow", "FPS", nullptr));
        label_3->setText(QCoreApplication::translate("SettingsWindow", "Packet size", nullptr));
        label_4->setText(QCoreApplication::translate("SettingsWindow", "Quality", nullptr));
        saveButton->setText(QCoreApplication::translate("SettingsWindow", "Save changes", nullptr));
    } // retranslateUi
};

class SettingsWindow : public QWidget, public Ui_SettingsWindow
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUi(this);
        QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
        fpsVal->setValue(settings.value("fps", FPS).toInt());
        packetVal->setValue(settings.value("packet", PACK_SIZE).toInt());
        qualityVal->setValue(settings.value("quality", ENCODE_QUALITY).toInt());
    }

    void saveSettings()
    {
        QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
        settings.setValue("fps", fpsVal->value());
        settings.setValue("packet", packetVal->value());
        settings.setValue("quality", qualityVal->value());
    }
};

#endif
