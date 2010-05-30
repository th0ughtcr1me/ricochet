/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#include "ContactInfoPage.h"
#include "core/ContactUser.h"
#include "utils/DateUtil.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QAction>
#include <QToolButton>
#include <QApplication>
#include <QTextStream>
#include <QDateTime>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>

ContactInfoPage::ContactInfoPage(ContactUser *u, QWidget *parent)
    : QWidget(parent), user(u)
{
    createActions();

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->setMargin(0);
    mainLayout->addLayout(infoLayout);

    createAvatar();
    infoLayout->addWidget(avatar, Qt::AlignTop | Qt::AlignLeft);

    infoLayout->addLayout(createInfo(), 1);
    infoLayout->addStretch();
    infoLayout->addLayout(createButtons());

    /* Notes */
    createNotes(mainLayout);
}

ContactInfoPage::~ContactInfoPage()
{
    saveNotes();
}

void ContactInfoPage::createActions()
{
    renameAction = new QAction(QIcon(QLatin1String(":/icons/user--pencil.png")), tr("Change Nickname"), this);
    renameAction->setIconVisibleInMenu(false);

    /* Show a grayscale icon normally, and the full color (red) icon when hovered */
    QIcon deleteIcon;
    deleteIcon.addFile(QLatin1String(":/icons/cross.png"), QSize(), QIcon::Active);
    deleteIcon.addPixmap(deleteIcon.pixmap(24, QIcon::Disabled));

    deleteAction = new QAction(deleteIcon, tr("Delete Contact"), this);
    deleteAction->setIconVisibleInMenu(false);
}

#include <QPainter>

/* 6x6 */
static const quint32 topLeftData[] =
{
    0x1000000u, 0x2000000u, 0x4000000u, 0x7000000u, 0x9000000u, 0xa000000u,
    0x2000000u, 0x6000000u, 0x0u, 0x0u, 0x0u, 0x0u,
    0x4000000u, 0xc000000u, 0x0u, 0x0u, 0x0u, 0x0u,
    0x7000000u, 0x14000000u, 0x0u, 0x0u, 0x0u, 0x0u,
    0x9000000u, 0x1a000000u, 0x0u, 0x0u, 0x0u, 0x0u,
    0xa000000u, 0x1e000000u, 0x0u, 0x0u, 0x0u, 0x0u
};
/* 6x6 */
static const quint32 topRightData[] =
{
    0xa000000u, 0x9000000u, 0x7000000u, 0x4000000u, 0x2000000u, 0x1000000u,
    0x0u, 0x0u, 0x14000000u, 0xc000000u, 0x6000000u, 0x2000000u,
    0x0u, 0x0u, 0x28000000u, 0x18000000u, 0xc000000u, 0x4000000u,
    0x0u, 0x0u, 0x42000000u, 0x28000000u, 0x14000000u, 0x7000000u,
    0x0u, 0x0u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x9000000u,
    0x0u, 0x0u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0xa000000u
};
/* 6x6 */
static const quint32 bottomRightData[] =
{
    0x0u, 0x0u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0xa000000u,
    0x80000000u, 0x6f000000u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x9000000u,
    0x62000000u, 0x55000000u, 0x42000000u, 0x28000000u, 0x14000000u, 0x7000000u,
    0x3b000000u, 0x33000000u, 0x28000000u, 0x18000000u, 0xc000000u, 0x4000000u,
    0x1e000000u, 0x1a000000u, 0x14000000u, 0xc000000u, 0x6000000u, 0x2000000u,
    0xa000000u, 0x9000000u, 0x7000000u, 0x4000000u, 0x2000000u, 0x1000000u
};
/* 6x6 */
static const quint32 bottomLeftData[] =
{
    0xa000000u, 0x1e000000u, 0x0u, 0x0u, 0x0u, 0x0u,
    0x9000000u, 0x1a000000u, 0x33000000u, 0x55000000u, 0x6f000000u, 0x80000000u,
    0x7000000u, 0x14000000u, 0x28000000u, 0x42000000u, 0x55000000u, 0x62000000u,
    0x4000000u, 0xc000000u, 0x18000000u, 0x28000000u, 0x33000000u, 0x3b000000u,
    0x2000000u, 0x6000000u, 0xc000000u, 0x14000000u, 0x1a000000u, 0x1e000000u,
    0x1000000u, 0x2000000u, 0x4000000u, 0x7000000u, 0x9000000u, 0xa000000u
};
/* 1x5 */
static const quint32 bottomData[] =
{
    0x88000000u,
    0x69000000u,
    0x3f000000u,
    0x20000000u,
    0xb000000u
};

void ContactInfoPage::createAvatar()
{
    avatar = new QLabel;
    avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QPixmap image = user->avatar(ContactUser::FullAvatar);
    if (!image.isNull())
    {
        QImage topLeft(reinterpret_cast<const uchar*>(topLeftData), 6, 6, QImage::Format_ARGB32);
        QImage topRight(reinterpret_cast<const uchar*>(topRightData), 6, 6, QImage::Format_ARGB32);
        QImage bottomRight(reinterpret_cast<const uchar*>(bottomRightData), 6, 6, QImage::Format_ARGB32);
        QImage bottomLeft(reinterpret_cast<const uchar*>(bottomLeftData), 6, 6, QImage::Format_ARGB32);

        static const quint32 rightData[] = { 0x69000000u, 0x3f000000u, 0x20000000u, 0x0b000000u };
        QImage right(reinterpret_cast<const uchar*>(rightData), 4, 1, QImage::Format_ARGB32);

        QImage bottom(reinterpret_cast<const uchar*>(bottomData), 1, 5, QImage::Format_ARGB32);

        /* Shadowed image */
        QImage shadowAvatar(image.width() + 6, image.height() + 6, QImage::Format_ARGB32_Premultiplied);
        shadowAvatar.fill(qRgb(240, 240, 240));
        QPainter p(&shadowAvatar);

        QRect imageRect(2, 1, image.width(), image.height());

        /* Draw avatar */
        p.drawPixmap(imageRect.topLeft(), image);

        /* Draw top-left corner */
        p.drawImage(0, 0, topLeft);

        /* Draw top */
        p.setPen(QColor(0, 0, 0, 11));
        p.drawLine(6, 0, imageRect.right() - 2, 0);

        /* Draw left */
        p.drawLine(0, 6, 0, imageRect.bottom() - 1);

        p.setPen(QColor(0, 0, 0, 32));
        p.drawLine(1, 6, 1, imageRect.bottom() - 1);

        /* Draw top-right corner */
        p.drawImage(imageRect.right() - 1, 0, topRight);

        /* Draw right-side repeat */
        p.drawTiledPixmap(QRect(imageRect.right()+1, 6, 4, imageRect.height() - 6), QPixmap::fromImage(right));

        /* Draw bottom-right corner */
        p.drawImage(imageRect.right() - 1, imageRect.bottom(), bottomRight);

        /* Draw bottom repeat */
        p.drawTiledPixmap(QRect(6, imageRect.bottom()+1, image.width() - 6, 5), QPixmap::fromImage(bottom));

        /* Draw bottom-left corner */
        p.drawImage(0, imageRect.bottom(), bottomLeft);

        p.end();

        avatar->setPixmap(QPixmap::fromImage(shadowAvatar));
    }
    else
        avatar->setPixmap(QPixmap(QLatin1String(":/graphics/avatar-placeholder.png")));
}

class IDLineEdit : public QLineEdit
{
public:
    IDLineEdit(QWidget *parent = 0)
        : QLineEdit(parent), blockMousePress(false)
    {
    }

protected:
    virtual void focusInEvent(QFocusEvent *ev)
    {
        QLineEdit::focusInEvent(ev);
        selectAll();

        if (ev->reason() == Qt::MouseFocusReason)
            blockMousePress = true;
    }

    virtual void mousePressEvent(QMouseEvent *ev)
    {
        if (blockMousePress)
        {
            blockMousePress = false;
            ev->accept();
            return;
        }

        QLineEdit::mousePressEvent(ev);
    }

    virtual void mouseDoubleClickEvent(QMouseEvent *ev)
    {
        selectAll();
        copy();

        ev->accept();
    }

private:
    bool blockMousePress;
};

QLayout *ContactInfoPage::createInfo()
{
    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(5);

    int row = 0;

    /* Nickname */
    nickname = new QLabel;
    nickname->setTextFormat(Qt::PlainText);
    nickname->setText(user->nickname());
    nickname->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    nickname->addAction(renameAction);
    nickname->setContextMenuPolicy(Qt::ActionsContextMenu);

    QFont font = nickname->font();
    font.setPointSize(11);
    nickname->setFont(font);

    QPalette p = nickname->palette();
    p.setColor(QPalette::WindowText, QColor(0x00, 0x66, 0xaa));
    nickname->setPalette(p);

    layout->addWidget(nickname, row++, 0, 1, 2);

    layout->setRowMinimumHeight(row++, 6);

    /* ID */
    p.setColor(QPalette::WindowText, QColor(0x80, 0x80, 0x80));

    QLabel *label = new QLabel(tr("ID:"));
    label->setContentsMargins(0, 0, 0, 0);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setPalette(p);
    layout->addWidget(label, row, 0);

    QLineEdit *id = new IDLineEdit;
    id->setText(user->contactID());
    id->setReadOnly(true);
    id->setFrame(false);
    id->setTextMargins(-2, 0, 0, 0);

    font = QFont(QLatin1String("Consolas, \"Courier New\""), 9);
    font.setStyleHint(QFont::TypeWriter);
    id->setFont(font);

    QPalette idPalette = id->palette();
    idPalette.setBrush(QPalette::Base, idPalette.window());
    idPalette.setBrush(QPalette::Text, idPalette.windowText());
    id->setPalette(idPalette);

    layout->addWidget(id, row++, 1);

    /* Connected date */
    label = new QLabel(tr("Connected:"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setPalette(p);
    layout->addWidget(label, row, 0);

    QLabel *connected = new QLabel;
    layout->addWidget(connected, row++, 1);

    QDateTime lastConnect = user->readSetting(QLatin1String("lastConnected")).toDateTime();
    if (user->isConnected())
        connected->setText(tr("Yes"));
    else if (lastConnect.isNull())
        connected->setText(tr("Never"));
    else
    {
        connected->setText(timeDifferenceString(lastConnect, QDateTime::currentDateTime()));
        connected->setToolTip(lastConnect.toString(Qt::DefaultLocaleLongDate));
    }

    layout->setRowStretch(row, 1);
    return layout;
}

QLayout *ContactInfoPage::createButtons()
{
    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    QPalette p = QApplication::palette();
    p.setColor(QPalette::ButtonText, QColor(104, 104, 104));

    QToolButton *btn = new QToolButton;
    btn->setFixedHeight(23);
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setDefaultAction(renameAction);
    btn->setPalette(p);
    layout->addWidget(btn);

    btn = new QToolButton;
    btn->setFixedHeight(23);
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setDefaultAction(deleteAction);
    btn->setPalette(p);
    layout->addWidget(btn);

    layout->addStretch();
    return layout;
}

void ContactInfoPage::createNotes(QBoxLayout *layout)
{
    /* Header */
    QLabel *notesHeader = new QLabel(tr("Private notes:"));
    notesHeader->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    notesHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    notesHeader->setContentsMargins(1, 0, 0, 0);

    QFont font = notesHeader->font();
    font.setBold(true);
    notesHeader->setFont(font);

    QPalette p = notesHeader->palette();
    p.setColor(QPalette::WindowText, Qt::darkGray);
    notesHeader->setPalette(p);

    layout->addWidget(notesHeader);

    /* Edit */
    notesEdit = new QTextEdit;
    notesEdit->insertPlainText(user->notesText());
    notesEdit->setFont(QFont(QLatin1String("Helvetica"), 9));
    layout->addWidget(notesEdit, 1);
}

void ContactInfoPage::saveNotes()
{
    if (!notesEdit->document()->isModified())
        return;

    user->setNotesText(notesEdit->document()->toPlainText());
    notesEdit->document()->setModified(false);
}

void ContactInfoPage::hideEvent(QHideEvent *ev)
{
    saveNotes();
    QWidget::hideEvent(ev);
}
