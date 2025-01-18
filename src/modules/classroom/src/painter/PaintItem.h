
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

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <QByteArray>
#include <QClipboard>
#include <QDataStream>
#include <QDebug>
#include <QDesktopServices>
#include <QFont>
#include <QGuiApplication>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QUrl>
#include <QUuid>

#include "lib/board/Draw.h"
#include "lib/board/DrawFile.h"
#include "lib/board/DrawLine.h"
#include "lib/board/DrawText.h"

#include "src/base/colorhelper.h"
#include "src/base/files.h"
#include "src/base/logs.h"
#include "src/base/utils.h"

#include "SharedPaintPolicy.h"
#include "Util.h"
#include "base/colorhelper.h"

namespace module::classroom {

// using namespace gloox;

using ColorHelper = base::ColorHelper;

class CPaintItem;
class CLineItem;
class CBackgroundImageItem;
class CFileItem;
class CTextItem;
class CImageItem;
class CImageFileItem;

enum PaintItemType {
    PT_NONE = 0,
    PT_LINE,
    PT_IMAGE,
    PT_BG_IMAGE,
    PT_FILE,
    PT_IMAGE_FILE,
    PT_TEXT,
    PT_MAX
};

typedef std::string ItemId;

typedef std::vector<std::shared_ptr<CPaintItem>> ITEM_LIST;
typedef std::set<std::shared_ptr<CPaintItem>> ITEM_SET;

class IGluePaintCanvas {
public:
    IGluePaintCanvas() {}

    virtual ~IGluePaintCanvas() {}

    virtual void drawSendingStatus(std::shared_ptr<CPaintItem> item) = 0;

    virtual QRectF itemBoundingRect(std::shared_ptr<CPaintItem> item) = 0;

    virtual void drawLine(std::shared_ptr<CLineItem> line) = 0;

    virtual void drawFile(std::shared_ptr<CFileItem> file) = 0;

    virtual void drawText(std::shared_ptr<CTextItem> text) = 0;

    virtual void drawImage(std::shared_ptr<CImageItem> image) = 0;

    virtual void drawImageFile(std::shared_ptr<CImageFileItem> imageFile) = 0;

    virtual void removePaintItem(CPaintItem* item) = 0;

    //    virtual void removeItem(std::shared_ptr<CPaintItem> item) = 0;

    virtual void moveItem(std::shared_ptr<CPaintItem> item, double x, double y) = 0;

    virtual void updateItem(std::shared_ptr<CPaintItem> item) = 0;

    virtual void clearBackgroundImage() = 0;

    virtual void clearScreen() = 0;

    virtual void setBackgroundColor(int r, int g, int b, int a) = 0;

    virtual void drawBackgroundGridLine(int size) = 0;

    virtual void drawBackgroundImage(std::shared_ptr<CBackgroundImageItem> line) = 0;
};

struct SPaintData {
    double posX;
    double posY;
    double scale;
    bool posSetFlag;
    ItemId itemId;
    std::string owner;
};

[[nodiscard]] inline static ItemId CreateItemId() {
    return ok::base::UUID::make().toStdString();
}

class CPaintItem : public std::enable_shared_from_this<CPaintItem> {
public:
    enum Action { Create, Update };

    CPaintItem(ItemId id)
            : canvas_(nullptr)
            , object_(nullptr)
            , mine_(false)
            , packetId_(-1)
            , wroteBytes_(0)
            , totalBytes_(0)
            , action_(Create)
            , type_(PaintItemType::PT_NONE) {
        // 初始化item id
        data_.itemId = id;
        data_.posX = 0.f;
        data_.posY = 0.f;
        data_.scale = 1.f;
        data_.posSetFlag = false;

        prevData_ = data_;

        qDebug() << "CPaintItem id:" << qstring(itemId()) << " create ptr=>" << this;
    }

    CPaintItem(const CPaintItem& item) : action_(Create), type_(PaintItemType::PT_NONE) {
        canvas_ = item.canvas_;
        object_ = item.object_;
        mine_ = item.mine_;
        packetId_ = item.packetId_;
        wroteBytes_ = item.wroteBytes_;
        totalBytes_ = item.totalBytes_;

        data_ = item.data_;
        prevData_ = data_;
    }

    virtual ~CPaintItem() {
        qDebug() << "CPaintItem id:" << qstring(itemId()) << " deleted ptr=>" << this;
        remove();
    }

    virtual Action action() {
        return action_;
    }

    virtual void setAction(Action action) {
        action_ = action;
    }

    virtual bool isValid() = 0;

    void setCanvas(IGluePaintCanvas* canvas) {
        canvas_ = canvas;
    }

    struct SPaintData& data() {
        return data_;
    }

    struct SPaintData& prevData() {
        return prevData_;
    }

    void setData(const struct SPaintData& data) {
        prevData_ = data_;
        data_ = data;
    }

    bool isAvailablePosition() {
        return data_.posSetFlag;
    }

    double posX() {
        return data_.posX;
    }

    double posY() {
        return data_.posY;
    }

    void setPos(double x, double y) {
        mutex_.lock();
        prevData_.posX = data_.posX;
        prevData_.posY = data_.posY;
        prevData_.posSetFlag = data_.posSetFlag;
        data_.posX = x;
        data_.posY = y;
        data_.posSetFlag = true;
        mutex_.unlock();
    }

    void setScale(double scale) {
        prevData_.scale = data_.scale;
        data_.scale = scale;
    }

    double scale() {
        return data_.scale;
    }

    void setOwner(const std::string& owner) {
        data_.owner = prevData_.owner = owner;
    }

    const std::string& owner() const {
        return data_.owner;
    }

    void setItemId(ItemId itemId) {
        data_.itemId = prevData_.itemId = itemId;
    }

    ItemId itemId() const {
        return data_.itemId;
    }

    void setMyItem() {
        mine_ = true;
    }

    bool isMyItem() {
        return mine_;
    }

    QRectF boundingRect() {
        if (canvas_) return canvas_->itemBoundingRect(shared_from_this());

        return QRect();
    }

    // TODO : sending packet work is not complete.
    // void setPacketId( int packetId ) { packetId_ = packetId; }
    // int packetId( void ) { return packetId_; }
    size_t wroteBytes() {
        return wroteBytes_;
    }

    size_t totalBytes() {
        return totalBytes_;
    }

    static bool deserializeBasicData(const std::string& data,
                                     struct SPaintData& res,
                                     int* readPos = nullptr) {
        return true;
    }

    static std::string serializeBasicData(const struct SPaintData& data, int* writePos = nullptr) {
        std::string buf = "gaojie";

        return buf;
    }

protected:
    static void copyTextToClipBoard(QString str, bool firstItem) {
        QClipboard* clipboard = QGuiApplication::clipboard();

        if (firstItem) {
            clipboard->setText("");
        }

        QString cbText = QGuiApplication::clipboard()->text();
        if (cbText.isEmpty() == false) cbText += NATIVE_NEWLINE_STR;

        cbText += str;

        clipboard->setText(cbText);
    }

    // virtual methods
public:
    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> sbDraw) = 0;

    virtual std::shared_ptr<lib::board::SmartBoardDraw> serialize() = 0;

    virtual PaintItemType type() {
        return type_;
    }

    virtual void move(double x, double y) {
        setPos(x, y);
        if (canvas_) canvas_->moveItem(shared_from_this(), x, y);
    }

    virtual void update() {
        if (canvas_) canvas_->updateItem(shared_from_this());
    }

    virtual void draw() = 0;

    virtual void execute() {}

    virtual void remove() {
        //        if (canvas_)
        //            canvas_->removeItem(this);
    }

    virtual void drawSendingStatus(size_t wroteBytes, size_t totalBytes) {
        wroteBytes_ = wroteBytes;
        totalBytes_ = totalBytes;
    }

    virtual bool isScalable() {
        return false;
    }

    virtual void copyToClipboard(bool firstItem = true) {
        if (firstItem) QGuiApplication::clipboard()->setText("");  // init clipboard
    }

protected:
    IGluePaintCanvas* canvas_;
    void* object_;
    bool mine_;
    int packetId_;
    size_t wroteBytes_;
    size_t totalBytes_;
    std::recursive_mutex mutex_;

    struct SPaintData data_;
    struct SPaintData prevData_;
    Action action_;
    PaintItemType type_;
};

class CLineItem : public CPaintItem {
public:
    CLineItem(ItemId id) : CPaintItem(id) {
        type_ = (PaintItemType::PT_LINE);
    }

    CLineItem(ItemId id, const QColor& color, int width) : CPaintItem(id), clr_(color), w_(width) {
        type_ = (PaintItemType::PT_LINE);
    }

    size_t pointCount() const {
        return listList_.size();
    }

    virtual const QPointF* point(size_t index) const {
        if (listList_.size() <= index) return nullptr;
        return &listList_[index];
    }

    const QColor& color() const {
        return clr_;
    }

    void setColor(QColor& color) {
        clr_ = color;
    }

    int width() const {
        return w_;
    }

    void setWidth(int width) {
        w_ = width;
    }

    void addPoint(const QPointF& pt) {
        listList_.push_back(pt);
    }

    virtual std::vector<QPointF>& points() {
        return listList_;
    }

    virtual void draw() {
        if (canvas_) canvas_->drawLine(std::static_pointer_cast<CLineItem>(shared_from_this()));
    }

    virtual bool isValid() {
        return true;
    }

    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> draw) {
        if (!draw) {
            return;
        }

        qDebug() << "draw: " << qstring(draw->id());

        const auto* _item = draw->findPlugin<lib::board::DrawLine>(lib::board::DrawType::Line);
        if (!_item) return;

        // action
        switch (_item->action()) {
            case lib::board::Action::Create:
                action_ = Create;
                break;
            case lib::board::Action::Update:
                action_ = Update;
                break;
            case lib::board::ON:
                break;
            case lib::board::OFF:
                break;
        }

        // id
        setItemId(_item->id());

        // style
        for (const auto& it : draw->style()) {
            if (it.first == "color") {
                QColor clr(qstring(it.second));
                clr_ = (clr);
            } else if (it.first == "width") {
                w_ = std::stoi(it.second);
            }
        }

        // points
        for (auto op : _item->points()) {
            listList_.push_back(QPointF(op.x, op.y));
        }
    }

    virtual std::shared_ptr<lib::board::SmartBoardDraw> serialize() {
        qDebug() << "item:" << qstring(itemId());

        std::shared_ptr<lib::board::SmartBoardDraw> draw =
                std::make_shared<lib::board::SmartBoardDraw>();

        lib::board::PointList drawpoints;
        for (auto p : points()) {
            struct lib::board::Point sp;
            sp.x = p.x();
            sp.y = p.y();
            drawpoints.push_back(sp);
        };

        lib::board::DrawLine* line = new lib::board::DrawLine(itemId(), drawpoints);
        draw->addPlugin(line);

        // style
        std::map<std::string, std::string>& style = draw->style();

        const std::string& color_ =
                ColorHelper::makeColorString(color(), ColorHelper::FORMAT::HEX).toStdString();

        style.insert(std::pair<std::string, std::string>("color", color_));
        style.insert(std::pair<std::string, std::string>("width", std::to_string(width())));

        // position ignore
        return draw;
    }

private:
    std::vector<QPointF> listList_;
    QColor clr_;
    int w_;
};

class CTextItem : public CPaintItem {
public:
    CTextItem(ItemId id) : CPaintItem(id) {
        type_ = (PaintItemType::PT_TEXT);
    }

    CTextItem(ItemId id, const QString& text, const QFont& font, const QColor& color)
            : CPaintItem(id), text_(text), font_(font), clr_(color) {
        type_ = (PaintItemType::PT_TEXT);
    }

    const QString& text() const {
        return text_;
    }

    virtual void setText(const QString& text) {
        text_ = text;
    }

    const QFont& font() const {
        return font_;
    }

    virtual void setFont(QFont font) {
        font_ = font;
    }

    const QColor& color() const {
        return clr_;
    }

    void setColor(QColor& color) {
        clr_ = color;
    }

    virtual void draw() {
        if (canvas_) {
            canvas_->drawText(std::static_pointer_cast<CTextItem>(shared_from_this()));
        }
    }

    virtual bool isScalable() {
        return true;
    }

    virtual void copyToClipboard(bool firstItem = true) {
        CPaintItem::copyToClipboard(firstItem);
        CPaintItem::copyTextToClipBoard(text_, firstItem);
    }

    virtual bool isValid() {
        return !text_.isNull() && !text_.isEmpty();
    }

    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> draw) {
        if (!draw) {
            return;
        }

        qDebug() << "draw: " << qstring(draw->id());

        const lib::board::DrawText* _item =
                draw->findPlugin<lib::board::DrawText>(lib::board::DrawType::Text);

        if (!_item) {
            return;
        }

        // action
        switch (_item->action()) {
            case lib::board::Action::Create:
                action_ = Create;
                break;
            case lib::board::Action::Update:
                action_ = Update;
                break;
        }

        // id
        setItemId(_item->id());

        // style
        for (auto it : draw->style()) {
            if (it.first == "color") {
                QColor clr(QString::fromStdString(it.second));
                setColor(clr);
            } else if (it.first == "pixel-size") {
                QFont font;
                font.setPixelSize(std::stoi(it.second));
                setFont(font);
            }
        }

        // position
        setPos(draw->position().x, draw->position().y);

        // text
        setText(QString::fromStdString(_item->text()));
    }

    virtual std::shared_ptr<lib::board::SmartBoardDraw> serialize() {
        qDebug() << "item:" << qstring(itemId());

        std::shared_ptr<lib::board::SmartBoardDraw> draw =
                std::make_shared<lib::board::SmartBoardDraw>();

        auto data = new lib::board::DrawText(itemId(), text_.toStdString());

        // action
        switch (action()) {
            case Create:
                data->setAction(lib::board::Action::Create);
                break;
            case Update:
                data->setAction(lib::board::Action::Update);
                break;
        }

        draw->addPlugin(data);

        // position
        draw->setPosition({(int)posX(), (int)posY()});

        // style
        std::map<std::string, std::string>& style = draw->style();

        style.insert(std::pair<std::string, std::string>(
                "color",
                ColorHelper::makeColorString(color(), ColorHelper::FORMAT::HEX).toStdString()));

        style.insert(std::pair<std::string, std::string>("pixel-size",
                                                         std::to_string(font_.pixelSize())));

        qDebug() << "draw:" << (draw.get());

        return draw;
    }

private:
    QString text_;
    QFont font_;
    QColor clr_;
};

class CImageItem : public CPaintItem {
public:
    CImageItem(ItemId id) : CPaintItem(id) {
        type_ = (PaintItemType::PT_IMAGE);
    }

    void setPixmap(const QPixmap& pixmap) {
        byteArray_.clear();
        QDataStream pixmapStream(&byteArray_, QIODevice::WriteOnly);
        pixmapStream << pixmap;
        pixmapStream.setByteOrder(QDataStream::LittleEndian);
    }

    QPixmap createPixmap() {
        QPixmap pixmap;

        QDataStream pixmapStream(&byteArray_, QIODevice::ReadOnly);
        pixmapStream.setByteOrder(QDataStream::LittleEndian);

        pixmapStream >> pixmap;
        return pixmap;
    }

    virtual void draw() {
        if (canvas_) canvas_->drawImage(std::static_pointer_cast<CImageItem>(shared_from_this()));
    }

    virtual bool isScalable() {
        return true;
    }

    virtual void copyToClipboard(bool firstItem = true) {
        CPaintItem::copyToClipboard(firstItem);
        QPixmap px = createPixmap();
        QGuiApplication::clipboard()->setPixmap(px);
    }

    virtual bool isValid() {
        return true;
    }

    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> draw) {}

    std::shared_ptr<lib::board::SmartBoardDraw> serialize() {
        return std::shared_ptr<lib::board::SmartBoardDraw>();
    }

private:
    QByteArray byteArray_;
};

class CBackgroundImageItem : public CImageItem {
public:
    CBackgroundImageItem(ItemId id) : CImageItem(id) {
        type_ = PaintItemType::PT_BG_IMAGE;
    }

    virtual void move(double x, double y) {
        Q_UNUSED(x);
        Q_UNUSED(y);
    }

    virtual void remove() {
        if (canvas_) canvas_->clearBackgroundImage();
    }

    virtual void draw() {
        if (canvas_)
            canvas_->drawBackgroundImage(
                    std::static_pointer_cast<CBackgroundImageItem>(shared_from_this()));
    }

    virtual bool isScalable() {
        return false;
    }

    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> draw) {}

    std::shared_ptr<lib::board::SmartBoardDraw> serialize() const {
        return std::shared_ptr<lib::board::SmartBoardDraw>();
    }
};

class CFileItem : public CPaintItem {
public:
    CFileItem(ItemId id) : CPaintItem(id) {}
    CFileItem(ItemId id, const QString& name) : CPaintItem(id), size_(0), name_(name) {
        type_ = PaintItemType::PT_FILE;
    }

    virtual const QString& name() const {
        return name_;
    }

    virtual const QString& url() const {
        return url_;
    }

    virtual void setContentType(const QString& contentType) {
        contentType_ = contentType;
    }

    virtual const QString& contentType() const {
        return contentType_;
    }

    virtual void setUrl(const QString& url) {
        url_ = url;
    }

    virtual void setMd5(const QString& md5) {
        md5_ = md5;
    }

    virtual ok::base::FileContentType getContentType() const {
        return ok::base::Files::GetContentType(name_);
    }

    virtual const QByteArray& byteArray() const {
        return byteArray_;
    }

    virtual void draw() {
        if (canvas_) canvas_->drawFile(std::static_pointer_cast<CFileItem>(shared_from_this()));
    }

    virtual void execute() {
        QString msg = QObject::tr("cannot execute file");
        msg += name_;
    }

    virtual void drawSendingStatus(size_t wroteBytes, size_t totalBytes) {
        CPaintItem::drawSendingStatus(wroteBytes, totalBytes);
        if (canvas_) canvas_->drawSendingStatus(shared_from_this());
    }

    virtual void copyToClipboard(bool firstItem = true) {
        CPaintItem::copyToClipboard(firstItem);
        CPaintItem::copyTextToClipBoard(name_, firstItem);
    }

    virtual bool isValid() {
        return true;
    }

    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> draw) {
        if (!draw) {
            return;
        }
        qDebug() << "draw: " << qstring(draw->id());

        auto _item = draw->findPlugin<lib::board::DrawFile>(lib::board::DrawType::File);
        if (!_item) {
            return;
        }

        setItemId(_item->id());
        size_ = _item->size();
        name_ = QString::fromStdString(_item->name());
        url_ = QString::fromStdString(_item->url());
        md5_ = QString::fromStdString(_item->md5());
        token_ = QString::fromStdString(_item->token());
        contentType_ = QString::fromStdString(_item->contentType());

        // action
        switch (_item->action()) {
            case lib::board::Action::Create:
                action_ = Create;
                break;
            case lib::board::Action::Update:
                action_ = Update;
                break;
            case lib::board::ON:
                break;
            case lib::board::OFF:
                break;
        }
    }

    std::shared_ptr<lib::board::SmartBoardDraw> serialize() {
        qDebug() << "item:" << qstring(itemId());

        std::shared_ptr<lib::board::SmartBoardDraw> draw =
                std::make_shared<lib::board::SmartBoardDraw>();

        auto _item = new lib::board::DrawFile(itemId(), name_.toStdString());

        _item->setSize(size_);
        _item->setUrl(url_.toStdString());
        _item->setMd5(md5_.toStdString());
        _item->setToken(token_.toStdString());
        _item->setContentType(contentType_.toStdString());

        // action
        switch (action()) {
            case Create:
                _item->setAction(lib::board::Action::Create);
                break;
            case Update:
                _item->setAction(lib::board::Action::Update);
                break;
        }

        draw->addPlugin(_item);

        return draw;
    }

protected:
    size_t size_;
    QString name_;
    QString contentType_;
    QString url_;
    QString md5_;
    QString token_;
    QByteArray byteArray_;
};

class CImageFileItem : public CFileItem {
public:
    CImageFileItem(ItemId id) : CFileItem(id) {
        type_ = (PT_IMAGE_FILE);
    }

    CImageFileItem(ItemId id, const QString& name) : CFileItem(id, name) {
        type_ = (PT_IMAGE_FILE);
    }

    virtual void draw() {
        if (canvas_)
            canvas_->drawImageFile(std::static_pointer_cast<CImageFileItem>(shared_from_this()));
    }

    virtual bool isScalable() {
        return true;
    }

    virtual void copyToClipboard(bool firstItem = true) {
        if (byteArray_.size() <= 0) {
            return;
        }

        CPaintItem::copyToClipboard(firstItem);
        QPixmap pixmap;
        pixmap.loadFromData(byteArray_);
        QGuiApplication::clipboard()->setPixmap(pixmap);
    }

    virtual bool isValid() {
        return true;
    }

    virtual void deserialize(std::shared_ptr<lib::board::SmartBoardDraw> draw) {
        CFileItem::deserialize(draw);

        // byteArray_
    }

    std::shared_ptr<lib::board::SmartBoardDraw> serialize() {
        return CFileItem::serialize();
    }
};

}  // namespace module::classroom
