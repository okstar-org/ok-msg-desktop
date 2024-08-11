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

#ifndef VARIANTTREE_H
#define VARIANTTREE_H

#include <QHash>
#include <QObject>
#include <QVariant>

class QDomDocument;
class QDomDocumentFragment;
class QDomElement;

/**
 * \class VariantTree
 * \brief A recursive structure for storing QVariants in trees, with comments
 *
 * All the methods in this class are recursive, based on a hierachy delimited
 * with dots in the node name. As such, the nodes "Paris" and "Benvolio" are
 * top level elements (members of this layer), while "Capulet.Juliet" is a
 * member of a deeper node ("Capulet") and "Verona.Montague.Romeo" represents
 * the node "Romeo" which is a member of "Montague", which is again a member
 * of "Verona" (which is a member of this layer).
 *
 * As such, for each function, if the supplied node contains a dot ("."),
 * the node name is split at the first (if there are several) dot, with the
 * remainder passed to the same method of the member of this node with the
 * name given by the primary component. For the set methods, multiple layers
 * in the hierachy may be created implicitly if the node is not found.
 */
class VariantTree : public QObject {
    Q_OBJECT
public:
    VariantTree(QObject* parent = nullptr);
    ~VariantTree();

    void setValue(QString node, QVariant value);
    QVariant getValue(const QString& node) const;

    bool isInternalNode(QString node) const;

    void setComment(QString node, QString comment);
    QString getComment(QString node) const;

    bool remove(const QString& node, bool internal_nodes = false);

    QStringList nodeChildren(const QString& node = "", bool direct = false,
                             bool internal_nodes = false) const;

    void toXml(QDomDocument& doc, QDomElement& ele) const;
    void fromXml(const QDomElement& ele);

    static bool isValidNodeName(const QString& name);

    static const QVariant missingValue;
    static const QString missingComment;

protected:
    static QVariant elementToVariant(const QDomElement&);
    static void variantToElement(const QVariant&, QDomElement&);

    static bool getKeyRest(const QString& node, QString& key, QString& rest);

private:
    QHash<QString, VariantTree*> trees_;
    QHash<QString, QVariant> values_;
    QHash<QString, QString> comments_;
    QHash<QString, QDomDocumentFragment> unknowns_;  // unknown types preservation
    QHash<QString, QString> unknowns2_;              // unknown types preservation

    // needed to have a document for the fragments.
    static QDomDocument* unknownsDoc;
    friend class OptionsTreeReader;
    friend class OptionsTreeWriter;
};

#endif  // VARIANTTREE_H
