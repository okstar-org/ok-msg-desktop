/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifndef FILETRANSFERWIDGET_H
#define FILETRANSFERWIDGET_H

#include <QTime>
#include <QWidget>

#include "src/chatlog/chatlinecontent.h"
#include "src/chatlog/toxfileprogress.h"
#include "src/core/toxfile.h"

namespace Ui {
class FileTransferWidget;
}

class QVariantAnimation;
class QPushButton;

class FileTransferWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileTransferWidget(QWidget* parent, ToxFile file);
    virtual ~FileTransferWidget();
    bool isActive() const;
    static QString getHumanReadableSize(qint64 size);

    void onFileTransferUpdate(ToxFile file);

protected:
    void updateWidgetColor(ToxFile const& file);
    void updateWidgetText(ToxFile const& file);
    void updateFileProgress(ToxFile const& file);
    void updateSignals(ToxFile const& file);
    void updatePreview(ToxFile const& file);
    void setupButtons(ToxFile const& file);
    void handleButton(QPushButton* btn);
    void showPreview(const QString& filename);
    void acceptTransfer(const QString& filepath);
    void setBackgroundColor(FileStatus status, bool useAnima = true);
    void setButtonColor(const QColor& c);

    bool drawButtonAreaNeeded() const;

    virtual void paintEvent(QPaintEvent*) final override;

private slots:
    void onLeftButtonClicked();
    void onRightButtonClicked();
    void onPreviewButtonClicked();

private:
    static QPixmap scaleCropIntoSquare(const QPixmap& source, int targetSize);
    static int getExifOrientation(const char* data, const int size);
    static void applyTransformation(const int oritentation, QImage& image);
    static bool tryRemoveFile(const QString& filepath);

    void updateWidget(ToxFile const& file);

private:
    Ui::FileTransferWidget* ui;
    ToxFileProgress fileProgress;
    ToxFile fileInfo;
    QVariantAnimation* backgroundColorAnimation = nullptr;
    QVariantAnimation* buttonColorAnimation = nullptr;
    QColor backgroundColor;
    QColor buttonColor;
    QColor buttonBackgroundColor;

    bool active;
    FileStatus lastStatus;

    enum class ExifOrientation {
        /* do not change values, this is exif spec
         *
         * name corresponds to where the 0 row and 0 column is in form row-column
         * i.e. entry 5 here means that the 0'th row corresponds to the left side of the scene and
         * the 0'th column corresponds to the top of the captured scene. This means that the image
         * needs to be mirrored and rotated to be displayed.
         */
        TopLeft = 1,
        TopRight = 2,
        BottomRight = 3,
        BottomLeft = 4,
        LeftTop = 5,
        RightTop = 6,
        RightBottom = 7,
        LeftBottom = 8
    };
};

#endif  // FILETRANSFERWIDGET_H
