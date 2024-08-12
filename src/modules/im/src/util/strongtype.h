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

#ifndef STRONGTYPE_H
#define STRONGTYPE_H

#include <QHash>

template <typename T> struct Addable {
    T operator+(T const& other) const { return static_cast<T const&>(*this).get() + other.get(); };
};

template <typename T, typename Underlying> struct UnderlyingAddable {
    T operator+(Underlying const& other) const {
        return T(static_cast<T const&>(*this).get() + other);
    };
};

template <typename T, typename Underlying> struct UnitlessDifferencable {
    T operator-(Underlying const& other) const {
        return T(static_cast<T const&>(*this).get() - other);
    };

    Underlying operator-(T const& other) const {
        return static_cast<T const&>(*this).get() - other.get();
    }
};

template <typename T, typename> struct Incrementable {
    T& operator++() {
        auto& underlying = static_cast<T&>(*this).get();
        ++underlying;
        return static_cast<T&>(*this);
    }

    T operator++(int) {
        auto ret = T(static_cast<T const&>(*this));
        ++(*this);
        return ret;
    }
};

template <typename T, typename> struct EqualityComparible {
    bool operator==(const T& other) const {
        return static_cast<T const&>(*this).get() == other.get();
    };
    bool operator!=(const T& other) const {
        return static_cast<T const&>(*this).get() != other.get();
    };
};

template <typename T, typename Underlying> struct Hashable {
    friend uint qHash(const Hashable<T, Underlying>& key, uint seed = 0) {
        return qHash(static_cast<T const&>(*key).get(), seed);
    }
};

template <typename T, typename Underlying> struct Orderable : EqualityComparible<T, Underlying> {
    bool operator<(const T& rhs) const { return static_cast<T const&>(*this).get() < rhs.get(); }
    bool operator>(const T& rhs) const { return static_cast<T const&>(*this).get() > rhs.get(); }
    bool operator>=(const T& rhs) const { return static_cast<T const&>(*this).get() >= rhs.get(); }
    bool operator<=(const T& rhs) const { return static_cast<T const&>(*this).get() <= rhs.get(); }
};

/* This class facilitates creating a named class which wraps underlying POD,
 * avoiding implict casts and arithmetic of the underlying data.
 * Usage: Declare named type with arbitrary tag, then hook up Qt metatype for use
 * in signals/slots. For queued connections, registering the metatype is also
 * required before the type is used.
 *   using MsgId = NamedType<uint32_t, struct ReceiptNumTag>;
 *   Q_DECLARE_METATYPE(MsgId);
 *   qRegisterMetaType<MsgId>();
 */

template <typename T, typename Tag, template <typename, typename> class... Properties>
class NamedType : public Properties<NamedType<T, Tag, Properties...>, T>... {
public:
    using UnderlyingType = T;

    NamedType() {}
    explicit NamedType(T const& value) : value_(value) {}
    T& get() { return value_; }
    T const& get() const { return value_; }

private:
    T value_;
};

template <typename T, typename Tag, template <typename, typename> class... Properties>
uint qHash(const NamedType<T, Tag, Properties...>& key, uint seed = 0) {
    return qHash(key.get(), seed);
}
#endif  // STRONGTYPE_H
