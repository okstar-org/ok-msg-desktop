#ifndef EVENTCREATOR_H
#define EVENTCREATOR_H

#include <QtPlugin>

class EventCreatingHost;

class EventCreator {
public:
    virtual ~EventCreator() { }

    virtual void setEventCreatingHost(EventCreatingHost *host) = 0;
};

Q_DECLARE_INTERFACE(EventCreator, "org.okstar.msg.EventCreator/0.1");

#endif // EVENTCREATOR_H
