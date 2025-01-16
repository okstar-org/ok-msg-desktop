
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

#include <memory>

#include "CommonPacketBuilder.h"
#include "PacketCodeDefine.h"
#include "PaintItem.h"
#include "PaintItemFactory.h"
#include "PainterView.h"

namespace PaintPacketBuilder {

using CPaintItem = painter::CPaintItem;
using PaintItemType = painter::PaintItemType;
using CBackgroundImageItem = painter::CBackgroundImageItem;
using CPaintItemFactory = painter::CPaintItemFactory;

class CSetBackgroundGridLine {
public:
    static std::string make(int size, const std::string* target = nullptr) {
        std::string data;
        int pos = 0;
        try {
            pos += CPacketBufferUtil::writeInt16(data, pos, size, true);

            return CommonPacketBuilder::makePacket(CODE_PAINT_SET_BG_GRID_LINE, data, nullptr,
                                                   target);
        } catch (...) {
        }

        return "";
    }

    static bool parse(const std::string& body, int& size) {
        int pos = 0;
        try {
            std::uint16_t s;
            pos += CPacketBufferUtil::readInt16(body, pos, s, true);

            size = s;
            return true;
        } catch (...) {
        }
        return false;
    }
};

class CSetBackgroundColor {
public:
    static std::string make(int r, int g, int b, int a, const std::string* target = nullptr) {
        std::string data;
        int pos = 0;
        try {
            pos += CPacketBufferUtil::writeInt16(data, pos, r, true);
            pos += CPacketBufferUtil::writeInt16(data, pos, g, true);
            pos += CPacketBufferUtil::writeInt16(data, pos, b, true);
            pos += CPacketBufferUtil::writeInt16(data, pos, a, true);

            return CommonPacketBuilder::makePacket(CODE_PAINT_SET_BG_COLOR, data, nullptr, target);
        } catch (...) {
        }

        return "";
    }

    static bool parse(const std::string& body, int& r, int& g, int& b, int& a) {
        int pos = 0;
        try {
            std::uint16_t rr, gg, bb, aa;
            pos += CPacketBufferUtil::readInt16(body, pos, rr, true);
            pos += CPacketBufferUtil::readInt16(body, pos, gg, true);
            pos += CPacketBufferUtil::readInt16(body, pos, bb, true);
            pos += CPacketBufferUtil::readInt16(body, pos, aa, true);

            r = rr;
            g = gg;
            b = bb;
            a = aa;
            return true;
        } catch (...) {
        }
        return false;
    }
};

class CSetBackgroundImage {
public:
    static std::string make(std::shared_ptr<CBackgroundImageItem> item,
                            const std::string* target = nullptr) {
        std::string data = item->serialize();

        try {
            return CommonPacketBuilder::makePacket(CODE_PAINT_SET_BG_IMAGE, data, nullptr, target);
        } catch (...) {
        }

        return "";
    }

    static std::shared_ptr<CBackgroundImageItem> parse(const std::string& body) {
        std::shared_ptr<CBackgroundImageItem> item;
        try {
            item = std::make_shared<CBackgroundImageItem>();
            if (!item.get()->deserialize(body)) {
                return std::make_shared<CBackgroundImageItem>();
            }
        } catch (...) {
            return std::make_shared<CBackgroundImageItem>();
        }

        return item;
    }
};

class CClearBackground {
public:
    static std::string make(void) {
        try {
            std::string body;
            return CommonPacketBuilder::makePacket(CODE_PAINT_CLEAR_BG, body);
        } catch (...) {
        }

        return "";
    }

    static bool parse(const std::string& body) {
        try {
            // NOTHING BODY
            return true;
        } catch (...) {
            return false;
        }
        return true;
    }
};

class CCreateItem {
public:
    static std::string make(std::shared_ptr<CPaintItem> item, const std::string* target = nullptr) {
        std::string body;
        std::string data = item->serialize();

        int pos = 0;
        try {
            pos += CPacketBufferUtil::writeInt16(body, pos, item->type(), true);
            body += data;

            return CommonPacketBuilder::makePacket(CODE_PAINT_CREATE_ITEM, body, nullptr, target);
        } catch (...) {
        }

        return "";
    }

    static std::shared_ptr<CPaintItem> parse(const std::string& body) {
        std::shared_ptr<CPaintItem> item;

        try {
            int pos = 0;
            std::uint16_t temptype;
            pos += CPacketBufferUtil::readInt16(body, pos, temptype, true);

            PaintItemType type = (PaintItemType)temptype;

            item = CPaintItemFactory::createItem(type);

            std::string itemData((const char*)body.c_str() + pos, body.size() - pos);
            if (!item->deserialize(itemData)) {
                return std::shared_ptr<CPaintItem>();
            }
        } catch (...) {
            return std::shared_ptr<CPaintItem>();
        }

        return item;
    }
};

class CClearScreen {
public:
    static std::string make(void) {
        try {
            std::string body;
            return CommonPacketBuilder::makePacket(CODE_PAINT_CLEAR_SCREEN, body);
        } catch (...) {
        }

        return "";
    }

    static bool parse(const std::string& body) {
        try {
            // NOTHING BODY
            return true;
        } catch (...) {
            return false;
        }
        return true;
    }
};
};  // namespace PaintPacketBuilder
