#include "MeetingVideosLayout.h"

MeetingVideosLayout::MeetingVideosLayout(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
}

void MeetingVideosLayout::resetLayout(module::meet::VideoLayoutType type) {
    layoutType = type;
}

module::meet::VideoLayoutType MeetingVideosLayout::currentLayoutType() const {
    return layoutType;
}

QSize MeetingVideosLayout::sizeHint() const {
    return QSize(600, 400);
}

QSize MeetingVideosLayout::minimumSizeHint() const {
    return QSize(300, 200);
}
