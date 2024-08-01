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

#ifndef OPTIONSTREE_H
#define OPTIONSTREE_H

#include "varianttree.h"

/**
 * \class OptionsTree
 * \brief Dynamic hierachical options structure
 * OptionsTree allows the dynamic creation of options (of type QVariant)
 * and will save and load these to/from xml.
 */
class OptionsTree : public QObject {
    Q_OBJECT
public:
    OptionsTree(QObject* parent = nullptr);
    ~OptionsTree();

    QVariant getOption(const QString& name, const QVariant& defaultValue = QVariant::Invalid) const;
    inline QVariant getOption(const char* name,
                              const QVariant& defaultValue = QVariant::Invalid) const {
        return getOption(QString(QString::fromLatin1(name)), defaultValue);
    }

    // Warning: when inserting Map/Hash be very careful with keys. They are going to become xml
    // element names. full set of supported types can be found in VariantTree::variantToElement()
    void setOption(const QString& name, const QVariant& value);
    bool isInternalNode(const QString& node) const;
    void setComment(const QString& name, const QString& comment);
    QString getComment(const QString& name) const;
    QStringList allOptionNames() const;
    QStringList getChildOptionNames(const QString& = QString(""), bool direct = false,
                                    bool internal_nodes = false) const;

    bool removeOption(const QString& name, bool internal_nodes = false);

    static bool isValidName(const QString& name);

    // Map helpers
    QString mapLookup(const QString& basename, const QVariant& key) const;
    QString mapPut(const QString& basename, const QVariant& key);
    void mapPut(const QString& basename, const QVariant& key, const QString& node,
                const QVariant& value);
    QVariant mapGet(const QString& basename, const QVariant& key, const QString& node) const;
    QVariant mapGet(const QString& basename, const QVariant& key, const QString& node,
                    const QVariant& def) const;
    QVariantList mapKeyList(const QString& basename, bool sortedByNumbers = false) const;

    bool saveOptions(const QString& fileName, const QString& configName, const QString& configNS,
                     const QString& configVersion, bool streamWriter = false) const;
    bool loadOptions(const QString& fileName, const QString& configName,
                     const QString& configNS = "", const QString& configVersion = "",
                     bool streamReader = false);
    bool loadOptions(const QDomElement& name, const QString& configName,
                     const QString& configNS = "", const QString& configVersion = "");
    static bool exists(QString fileName);

signals:
    void optionChanged(const QString& option);
    void optionAboutToBeInserted(const QString& option);
    void optionInserted(const QString& option);
    void optionAboutToBeRemoved(const QString& option);
    void optionRemoved(const QString& option);

private:
    VariantTree tree_;
    friend class OptionsTreeReader;
    friend class OptionsTreeWriter;
};

#endif  // OPTIONSTREE_H
