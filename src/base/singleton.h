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
#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>

template <class T>

class singleton {
protected:
    singleton(){};

private:
    singleton(const singleton&){};
    singleton& operator=(const singleton&) {};
    static T* m_instance;

public:
    template <typename... Args> static T* GetInstance(Args&&... args) {
        if (m_instance == NULL) m_instance = new T(std::forward<Args>(args)...);
        return m_instance;
    }

    static void DestroyInstance() {
        if (m_instance) delete m_instance;
        m_instance = NULL;
    }
};

template <class T> T* singleton<T>::m_instance = NULL;

#endif  // SINGLETON_H
