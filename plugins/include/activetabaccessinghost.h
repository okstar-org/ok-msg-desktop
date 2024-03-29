#ifndef ACTIVETABACCESSINGHOST_H
#define ACTIVETABACCESSINGHOST_H

#include <QtPlugin>

class QString;
class QTextEdit;

class ActiveTabAccessingHost {
public:
    virtual ~ActiveTabAccessingHost() { }

    virtual QTextEdit *getEditBox() = 0;
    virtual QString    getJid()     = 0;
    virtual QString    getYourJid() = 0; // return full jid of your account for active tab
};

Q_DECLARE_INTERFACE(ActiveTabAccessingHost, "org.okstar.msg.ActiveTabAccessingHost/0.1");
#endif // ACTIVETABACCESSINGHOST_H
