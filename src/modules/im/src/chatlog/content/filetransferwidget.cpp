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

#include "filetransferwidget.h"
#include "ui_filetransferwidget.h"



#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>

#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>
#include <math.h>
#include <cassert>
#include <libexif/exif-loader.h>

#include "base/files.h"
#include "lib/audio/player.h"
#include "src/persistence/profile.h"
#include "src/application.h"
#include "lib/ui/gui.h"
#include "src/core/core.h"
#include "src/core/corefile.h"
#include "src/lib/storage/settings/style.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/widget.h"

namespace module::im {

FileTransferWidget::FileTransferWidget(QWidget* parent, ToxFile file)
        : QWidget(parent)
        , ui(new Ui::FileTransferWidget)
        , fileInfo(file)
        , backgroundColor(lib::settings::Style::getColor(
                  lib::settings::Style::ColorPalette::TransferMiddle))
        , buttonColor(
                  lib::settings::Style::getColor(lib::settings::Style::ColorPalette::TransferWait))
        , buttonBackgroundColor(
                  lib::settings::Style::getColor(lib::settings::Style::ColorPalette::GroundBase))
        , active(true) {
    ui->setupUi(this);

    // hide the QWidget background (background-color: transparent doesn't seem to work)
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui->previewButton->hide();
    ui->filenameLabel->setText(file.fileName);
    ui->progressBar->setValue(0);
    ui->fileSizeLabel->setText(getHumanReadableSize(file.fileSize));
    ui->etaLabel->setText("");


    backgroundColorAnimation = new QVariantAnimation(this);
    backgroundColorAnimation->setDuration(500);
    backgroundColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(backgroundColorAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& val) {
                backgroundColor = val.value<QColor>();
                update();
            });

    buttonColorAnimation = new QVariantAnimation(this);
    buttonColorAnimation->setDuration(500);
    buttonColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(buttonColorAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& val) {
                buttonColor = val.value<QColor>();
                update();
            });

    CoreFile* coreFile = CoreFile::getInstance();

    connect(ui->leftButton, &QPushButton::clicked, this, &FileTransferWidget::onLeftButtonClicked);
    connect(ui->rightButton, &QPushButton::clicked, this, &FileTransferWidget::onRightButtonClicked);
    connect(ui->previewButton, &QPushButton::clicked, this, &FileTransferWidget::onPreviewButtonClicked);

    ui->playButton->setIcon(QIcon(lib::settings::Style::getImagePath("fileTransferInstance/arrow_white.svg")));
    connect(ui->playButton, &QPushButton::clicked, this, &FileTransferWidget::onPlayButtonClicked);

    // Set lastStatus to anything but the file's current value, this forces an update
    lastStatus =
            file.status == FileStatus::FINISHED ? FileStatus::INITIALIZING : FileStatus::FINISHED;
    updateWidget(file);

    setFixedHeight(64);
}

FileTransferWidget::~FileTransferWidget() {
    delete ui;
}

// TODO(sudden6): remove file IO from the UI
/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
bool FileTransferWidget::tryRemoveFile(const QString& filepath) {
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

void FileTransferWidget::onFileTransferUpdate(ToxFile file) {
    updateWidget(file);
}

void FileTransferWidget::onCopy() {
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) return;

    auto img = getImage();
    if (!img.isNull()) {
        // is image
        clipboard->setImage(img);
        return;
    }

    auto fp = fileInfo.filePath;
    if (!fp.isEmpty()) {
        clipboard->setText(fp);
    }
}

QImage FileTransferWidget::getImage() {
    if (!previewable()) {
        return {};
    }

    auto f = fileInfo.file;
    if (!f->exists()) {
        qWarning() << "File is no existing";
        return {};
    }

    if (!f->open(QIODevice::ReadOnly)) {
        qWarning() << "Can not to read";
        return {};
    }

    auto imageFileData = f->readAll();
    if (imageFileData.isEmpty()) {
        qWarning() << "Empty image data";
        return {};
    }

    auto image = QImage::fromData(imageFileData);
    if (image.isNull()) {
        qWarning() << "Unable to read the image data";
        return {};
    }

    return image;
}

bool FileTransferWidget::isActive() const {
    return active;
}

bool FileTransferWidget::previewable() {
    auto f = fileInfo.file;

    if (f && !f->exists()) {
        return false;
    }
    static const QStringList previewExtensions = {"bmp", "png", "jpeg", "jpg", "gif", "svg"};
    return previewExtensions.contains(QFileInfo(f->fileName()).suffix(),
                                      Qt::CaseSensitivity::CaseInsensitive);
}

void FileTransferWidget::acceptTransfer(const QString& filepath) {
    if (filepath.isEmpty()) {
        return;
    }

    // test if writable
    if (!tryRemoveFile(filepath)) {
        lib::ui::GUI::showWarning(
                tr("Location not writable", "Title of permissions popup"),
                tr("You do not have permission to write that location. Choose another, or "
                            "cancel the save dialog.",
                            "text of permissions popup"));
        return;
    }

    // everything ok!
    CoreFile* coreFile = CoreFile::getInstance();
    coreFile->acceptFileRecvRequest(fileInfo.receiver, fileInfo.fileId, filepath);
}

void FileTransferWidget::setBackgroundColor(FileStatus status, bool useAnima) {
    QColor color;
    bool whiteFont;
    switch (status) {
        case FileStatus::INITIALIZING:
        case FileStatus::PAUSED:
        case FileStatus::TRANSMITTING:
            color = lib::settings::Style::getColor(
                    lib::settings::Style::ColorPalette::TransferMiddle);
            whiteFont = false;
            break;
        case FileStatus::BROKEN:
        case FileStatus::CANCELED:
            color = lib::settings::Style::getColor(lib::settings::Style::ColorPalette::TransferBad);
            whiteFont = true;
            break;
        case FileStatus::FINISHED:
            color = lib::settings::Style::getColor(
                    lib::settings::Style::ColorPalette::TransferGood);
            whiteFont = true;
            break;
        default:
            return;
            break;
    }

    if (color != backgroundColor) {
        if (useAnima) {
            backgroundColorAnimation->setStartValue(backgroundColor);
            backgroundColorAnimation->setEndValue(color);
            backgroundColorAnimation->start();
        } else {
            backgroundColor = color;
        }
    }

    setProperty("fontColor", whiteFont ? "white" : "black");

    setStyleSheet(
            lib::settings::Style::getStylesheet("fileTransferInstance/filetransferWidget.css"));
}

void FileTransferWidget::setButtonColor(const QColor& c) {
    if (c != buttonColor) {
        buttonColorAnimation->setStartValue(buttonColor);
        buttonColorAnimation->setEndValue(c);
        buttonColorAnimation->start();
    }
}

bool FileTransferWidget::drawButtonAreaNeeded() const {
    return (ui->rightButton->isVisible() || ui->leftButton->isVisible()) &&
           !(ui->leftButton->isVisible() && ui->leftButton->objectName() == "ok");
}

void FileTransferWidget::paintEvent(QPaintEvent*) {
    // required by Hi-DPI support as border-image doesn't work.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    qreal ratio = static_cast<qreal>(geometry().height()) / static_cast<qreal>(geometry().width());
    const int r = 24;
    const int buttonFieldWidth = 32;
    const int lineWidth = 1;

    // Draw the widget background:
    painter.setClipRect(QRect(0, 0, width(), height()));
    painter.setBrush(QBrush(backgroundColor));
    painter.drawRoundedRect(geometry(), r * ratio, r, Qt::RelativeSize);

    if (drawButtonAreaNeeded()) {
        // Draw the button background:
        QPainterPath buttonBackground;
        buttonBackground.addRoundedRect(width() - 2 * buttonFieldWidth - lineWidth * 2, 0,
                                        buttonFieldWidth, buttonFieldWidth + lineWidth, 50, 50,
                                        Qt::RelativeSize);
        buttonBackground.addRect(width() - 2 * buttonFieldWidth - lineWidth * 2, 0,
                                 buttonFieldWidth * 2, buttonFieldWidth / 2);
        buttonBackground.addRect(width() - 1.5 * buttonFieldWidth - lineWidth * 2, 0,
                                 buttonFieldWidth * 2, buttonFieldWidth + 1);
        buttonBackground.setFillRule(Qt::WindingFill);
        painter.setBrush(QBrush(buttonBackgroundColor));
        painter.drawPath(buttonBackground);

        // Draw the left button:
        QPainterPath leftButton;
        leftButton.addRoundedRect(QRect(width() - 2 * buttonFieldWidth - lineWidth, 0,
                                        buttonFieldWidth, buttonFieldWidth),
                                  50, 50, Qt::RelativeSize);
        leftButton.addRect(QRect(width() - 2 * buttonFieldWidth - lineWidth, 0,
                                 buttonFieldWidth / 2, buttonFieldWidth / 2));
        leftButton.addRect(QRect(width() - 1.5 * buttonFieldWidth - lineWidth, 0,
                                 buttonFieldWidth / 2, buttonFieldWidth));
        leftButton.setFillRule(Qt::WindingFill);
        painter.setBrush(QBrush(buttonColor));
        painter.drawPath(leftButton);

        // Draw the right button:
        painter.setBrush(QBrush(buttonColor));
        painter.setClipRect(
                QRect(width() - buttonFieldWidth, 0, buttonFieldWidth, buttonFieldWidth));
        painter.drawRoundedRect(geometry(), r * ratio, r, Qt::RelativeSize);
    }
}

QString FileTransferWidget::getHumanReadableSize(qint64 size) {
    static const char* suffix[] = {"B", "kiB", "MiB", "GiB", "TiB"};
    int exp = 0;

    if (size > 0) {
        exp = std::min((int)(log(size) / log(1024)), (int)(sizeof(suffix) / sizeof(suffix[0]) - 1));
    }

    return QString().setNum(size / pow(1024, exp), 'f', exp > 1 ? 2 : 0).append(suffix[exp]);
}

void FileTransferWidget::updateWidgetColor(ToxFile const& file) {
    if (lastStatus == file.status) {
        return;
    }

    switch (file.status) {
        case FileStatus::INITIALIZING:
        case FileStatus::PAUSED:
        case FileStatus::TRANSMITTING:
        case FileStatus::BROKEN:
        case FileStatus::CANCELED:
        case FileStatus::FINISHED:
            setBackgroundColor(file.status);
            break;
        default:
            qWarning() << "Invalid file status" << file.fileId;
            break;
    }
}

void FileTransferWidget::updateWidgetText(ToxFile const& file) {
    if (lastStatus == file.status && file.status != FileStatus::PAUSED) {
        return;
    }

    switch (file.status) {
        case FileStatus::INITIALIZING:
            if (file.direction == FileDirection::SENDING) {
                ui->progressLabel->setText(tr("Waiting to send...", "file transfer widget"));
            } else {
                ui->progressLabel->setText(
                        tr("Accept to receive this file", "file transfer widget"));
            }
            break;
        case FileStatus::PAUSED:
            //        ui->etaLabel->setText("");
            //        if (file.pauseStatus.localPaused()) {
            //            ui->progressLabel->setText(tr("Paused", "file transfer widget"));
            //        } else {
            //            ui->progressLabel->setText(tr("Remote Paused", "file transfer widget"));
            //        }
            break;
        case FileStatus::TRANSMITTING:
            ui->etaLabel->setText("");
            ui->progressLabel->setText(tr("Resuming...", "file transfer widget"));
            break;
        case FileStatus::BROKEN:
        case FileStatus::CANCELED:
            break;
        case FileStatus::FINISHED:
            break;
        default:
            qWarning() << "Invalid file status" << file.fileId;
            break;
    }
}

void FileTransferWidget::updatePreview(ToxFile const& file) {
    if (lastStatus == file.status) {
        return;
    }

    switch (file.status) {
        case FileStatus::INITIALIZING:
        case FileStatus::PAUSED:
        case FileStatus::TRANSMITTING:
        case FileStatus::BROKEN:
        case FileStatus::CANCELED:
            if (file.direction == FileDirection::SENDING) {
                showPreview(file.filePath);
            }
            break;
        case FileStatus::FINISHED:
            showPreview(file.filePath);
            break;
        default:
            qWarning() << "Invalid file status" << file.fileId;
            //        assert(false);
    }
}

void FileTransferWidget::updateFileProgress(ToxFile const& file) {
    switch (file.status) {
        case FileStatus::INITIALIZING:
            break;
        case FileStatus::PAUSED:
            fileProgress.resetSpeed();
            break;
        case FileStatus::TRANSMITTING: {
            if (!fileProgress.needsUpdate()) {
                break;
            }

            fileProgress.addSample(file);
            auto speed = fileProgress.getSpeed();
            auto progress = fileProgress.getProgress();
            auto remainingTime = fileProgress.getTimeLeftSeconds();

            ui->progressBar->setValue(static_cast<int>(progress * 100.0));

            // update UI
            if (speed > 0) {
                // ETA
                QTime toGo = QTime(0, 0).addSecs(remainingTime);
                QString format = toGo.hour() > 0 ? "hh:mm:ss" : "mm:ss";
                ui->etaLabel->setText(toGo.toString(format));
            } else {
                ui->etaLabel->setText("");
            }

            ui->progressLabel->setText(getHumanReadableSize(speed) + "/s");
            break;
        }
        case FileStatus::BROKEN:
        case FileStatus::CANCELED:
        case FileStatus::FINISHED: {
            ui->progressBar->hide();
            ui->progressLabel->hide();
            ui->etaLabel->hide();
            break;
        }
        default:
            qWarning() << "Invalid file status" << file.fileId;
            //        assert(false);
    }
}

void FileTransferWidget::updateSignals(ToxFile const& file) {
    if (lastStatus == file.status) {
        return;
    }

    switch (file.status) {
        case FileStatus::CANCELED:
        case FileStatus::BROKEN:
        case FileStatus::FINISHED:
            active = false;
            disconnect(CoreFile::getInstance(), nullptr, this, nullptr);
            break;
        case FileStatus::INITIALIZING:
        case FileStatus::PAUSED:
        case FileStatus::TRANSMITTING:
            break;
        default:
            qWarning() << "Invalid file status" << file.fileId;
            break;
    }
}

void FileTransferWidget::setupButtons(ToxFile const& file) {
    if (lastStatus == file.status && file.status != FileStatus::PAUSED) {
        return;
    }

    switch (file.status) {
        case FileStatus::TRANSMITTING:
            ui->leftButton->setIcon(
                    QIcon(lib::settings::Style::getImagePath("fileTransferInstance/pause.svg")));
            ui->leftButton->setObjectName("pause");
            ui->leftButton->setToolTip(tr("Pause transfer"));

            ui->rightButton->setIcon(
                    QIcon(lib::settings::Style::getImagePath("fileTransferInstance/no.svg")));
            ui->rightButton->setObjectName("cancel");
            ui->rightButton->setToolTip(tr("Cancel transfer"));

            setButtonColor(lib::settings::Style::getColor(
                    lib::settings::Style::ColorPalette::TransferGood));
            break;

        case FileStatus::PAUSED:
            //        if (file.pauseStatus.localPaused()) {
            //            ui->leftButton->setIcon(QIcon(Style::getImagePath("fileTransferInstance/arrow_white.svg")));
            //            ui->leftButton->setObjectName("resume");
            //            ui->leftButton->setToolTip(tr("Resume transfer"));
            //        } else {
            //            ui->leftButton->setIcon(QIcon(Style::getImagePath("fileTransferInstance/pause.svg")));
            //            ui->leftButton->setObjectName("pause");
            //            ui->leftButton->setToolTip(tr("Pause transfer"));
            //        }

            ui->rightButton->setIcon(
                    QIcon(lib::settings::Style::getImagePath("fileTransferInstance/no.svg")));
            ui->rightButton->setObjectName("cancel");
            ui->rightButton->setToolTip(tr("Cancel transfer"));

            setButtonColor(lib::settings::Style::getColor(
                    lib::settings::Style::ColorPalette::TransferMiddle));
            break;

        case FileStatus::INITIALIZING:
            ui->rightButton->setIcon(
                    QIcon(lib::settings::Style::getImagePath("fileTransferInstance/no.svg")));
            ui->rightButton->setObjectName("cancel");
            ui->rightButton->setToolTip(tr("Cancel transfer"));

            if (file.direction == FileDirection::SENDING) {
                ui->leftButton->setIcon(QIcon(
                        lib::settings::Style::getImagePath("fileTransferInstance/pause.svg")));
                ui->leftButton->setObjectName("pause");
                ui->leftButton->setToolTip(tr("Pause transfer"));
            } else {
                ui->leftButton->setIcon(
                        QIcon(lib::settings::Style::getImagePath("fileTransferInstance/yes.svg")));
                ui->leftButton->setObjectName("accept");
                ui->leftButton->setToolTip(tr("Accept transfer"));
            }
            break;
        case FileStatus::CANCELED:
        case FileStatus::BROKEN:
            ui->leftButton->hide();
            ui->rightButton->hide();
            break;
        case FileStatus::FINISHED:
            ui->leftButton->setIcon(
                    QIcon(lib::settings::Style::getImagePath("fileTransferInstance/yes.svg")));
            ui->leftButton->setObjectName("ok");
            ui->leftButton->setToolTip(tr("Open file"));
            ui->leftButton->show();

            ui->rightButton->setIcon(
                    QIcon(lib::settings::Style::getImagePath("fileTransferInstance/dir.svg")));
            ui->rightButton->setObjectName("dir");
            ui->rightButton->setToolTip(tr("Open file directory"));
            ui->rightButton->show();

            break;
        default:
            qWarning() << "Invalid file status" << file.fileId;
            break;
    }
}

void FileTransferWidget::handleButton(QPushButton* btn) {
    qDebug() << "handle button for file:" << fileInfo.fileName;
    CoreFile* coreFile = CoreFile::getInstance();
    if (fileInfo.direction == FileDirection::SENDING) {
        if (btn->objectName() == "cancel") {
            coreFile->cancelFileSend(fileInfo.receiver, fileInfo.fileId);
        } else if (btn->objectName() == "pause") {
            coreFile->pauseResumeFile(fileInfo.receiver, fileInfo.fileId);
        } else if (btn->objectName() == "resume") {
            coreFile->pauseResumeFile(fileInfo.receiver, fileInfo.fileId);
        }
    } else  // receiving or paused
    {
        if (btn->objectName() == "cancel") {
            coreFile->cancelFileRecv(fileInfo.receiver, fileInfo.fileId);
        } else if (btn->objectName() == "pause") {
            coreFile->pauseResumeFile(fileInfo.receiver, fileInfo.fileId);
        } else if (btn->objectName() == "resume") {
            coreFile->pauseResumeFile(fileInfo.receiver, fileInfo.fileId);
        } else if (btn->objectName() == "accept") {
            QString path = Nexus::getProfile()->getSettings()->getGlobalAutoAcceptDir() + "/" +
                           fileInfo.fileName;
            if (path.isEmpty()) return;
            qDebug() << "accept file save to path:" << path;
            acceptTransfer(path);
        }
    }

    if (btn->objectName() == "ok" || btn->objectName() == "previewButton") {
        Widget::confirmExecutableOpen(QFileInfo(fileInfo.filePath));
    } else if (btn->objectName() == "dir") {
        QString dirPath = QFileInfo(fileInfo.filePath).dir().path();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    }
}

void FileTransferWidget::showPreview(const QString& filename) {
    qDebug() << __func__ << filename;

    if (!previewable()) {
        return;
    }

    QFile imageFile(filename);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        return;
    }

    auto imageFileData = imageFile.readAll();
    if (imageFileData.isEmpty()) {
        qWarning() << "Empty image data!";
        return;
    }

    auto image = QImage::fromData(imageFileData);
    if (image.isNull()) {
        qWarning() << "Unable to read the image data!";
        return;
    }

    const int exifOrientation = getExifOrientation(imageFileData.constData(), imageFileData.size());
    if (exifOrientation) {
        applyTransformation(exifOrientation, image);
    }

    const int size = qMax(ui->previewButton->width(), ui->previewButton->height()) - 4;
    const QPixmap iconPixmap = scaleCropIntoSquare(QPixmap::fromImage(image), size);
    ui->previewButton->setIcon(QIcon(iconPixmap));
    ui->previewButton->setIconSize(iconPixmap.size());
    ui->previewButton->show();

    // Show mouseover preview, but make sure it's not larger than 50% of the screen
    // width/height
    const QRect desktopSize = QApplication::desktop()->geometry();
    const int maxPreviewWidth{desktopSize.width() / 2};
    const int maxPreviewHeight{desktopSize.height() / 2};
    const QImage previewImage = [&image, maxPreviewWidth, maxPreviewHeight]() {
        if (image.width() > maxPreviewWidth || image.height() > maxPreviewHeight) {
            return image.scaled(maxPreviewWidth, maxPreviewHeight, Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);
        } else {
            return image;
        }
    }();

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    previewImage.save(&buffer, "PNG");
    buffer.close();

    ui->previewButton->setToolTip("<img src=data:image/png;base64," + imageData.toBase64() + "/>");
}

void FileTransferWidget::onLeftButtonClicked() {
    handleButton(ui->leftButton);
}

void FileTransferWidget::onRightButtonClicked() {
    handleButton(ui->rightButton);
}

void FileTransferWidget::onPreviewButtonClicked() {
    handleButton(ui->previewButton);
}

void FileTransferWidget::onPlayButtonClicked()
{
    if(!ok::base::Files::isAudio(fileInfo.filePath)){
        return;
    }

    //播放音频文件
    auto a = ok::Application::Instance();
    a->getAudioPlayer()->play(fileInfo.filePath);
}

QPixmap FileTransferWidget::scaleCropIntoSquare(const QPixmap& source, const int targetSize) {
    QPixmap result;

    // Make sure smaller-than-icon images (at least one dimension is smaller) will not be
    // upscaled
    if (source.width() < targetSize || source.height() < targetSize) {
        result = source;
    } else {
        result = source.scaled(targetSize, targetSize, Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
    }

    // Then, image has to be cropped (if needed) so it will not overflow rectangle
    // Only one dimension will be bigger after Qt::KeepAspectRatioByExpanding
    if (result.width() > targetSize) {
        return result.copy((result.width() - targetSize) / 2, 0, targetSize, targetSize);
    } else if (result.height() > targetSize) {
        return result.copy(0, (result.height() - targetSize) / 2, targetSize, targetSize);
    }

    // Picture was rectangle in the first place, no cropping
    return result;
}

int FileTransferWidget::getExifOrientation(const char* data, const int size) {
    ExifData* exifData =
            exif_data_new_from_data(reinterpret_cast<const unsigned char*>(data), size);

    if (!exifData) {
        return 0;
    }

    int orientation = 0;
    const ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
    const ExifEntry* const exifEntry = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);
    if (exifEntry) {
        orientation = exif_get_short(exifEntry->data, byteOrder);
    }
    exif_data_free(exifData);
    return orientation;
}

void FileTransferWidget::applyTransformation(const int orientation, QImage& image) {
    QTransform exifTransform;
    switch (static_cast<ExifOrientation>(orientation)) {
        case ExifOrientation::TopLeft:
            break;
        case ExifOrientation::TopRight:
            image = image.mirrored(1, 0);
            break;
        case ExifOrientation::BottomRight:
            exifTransform.rotate(180);
            break;
        case ExifOrientation::BottomLeft:
            image = image.mirrored(0, 1);
            break;
        case ExifOrientation::LeftTop:
            exifTransform.rotate(90);
            image = image.mirrored(0, 1);
            break;
        case ExifOrientation::RightTop:
            exifTransform.rotate(-90);
            break;
        case ExifOrientation::RightBottom:
            exifTransform.rotate(-90);
            image = image.mirrored(0, 1);
            break;
        case ExifOrientation::LeftBottom:
            exifTransform.rotate(90);
            break;
        default:
            qWarning() << "Invalid exif orientation passed to applyTransformation!";
    }
    image = image.transformed(exifTransform);
}

void FileTransferWidget::updateWidget(ToxFile const& file) {
    assert(file == fileInfo);

    fileInfo = file;

    // If we repainted on every packet our gui would be *very* slow
    bool bTransmitNeedsUpdate = fileProgress.needsUpdate();

    updatePreview(file);
    updateFileProgress(file);
    updateWidgetText(file);
    updateWidgetColor(file);
    setupButtons(file);
    updateSignals(file);

    lastStatus = file.status;

    // trigger repaint
    switch (file.status) {
        case FileStatus::TRANSMITTING:
            if (!bTransmitNeedsUpdate) {
                break;
            }
        // fallthrough
        default:
            update();
    }
}
}  // namespace module::im
