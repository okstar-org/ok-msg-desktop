#ifndef PLATFORMPAGE_H
#define PLATFORMPAGE_H

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QUrl>

class QWidget;
namespace ok::platform {

class PlatformPageContainer;

// 工作平台的页签基本信息类
class PlatformPage : public QObject {
    Q_OBJECT
public:
    PlatformPage(PlatformPageContainer* pageContainer);
    // 标题，页签显示
    virtual QString getTitle();
    // 图标，页签图标
    virtual QIcon getIcon();
    // 初始化，主要创建QWidget控件，在PlatformPage构造后调用
    virtual void createContent(QWidget* parent) = 0;
    // 获取创建好的QWidget控件
    virtual QWidget* getWidget();
    // 开始，createContent后调用
    virtual void start();
    // 关闭，页签关闭时先调用
    virtual void doClose() = 0;
    // 页签URL，作为页签唯一性判定，如果不需要可以去掉
    virtual QUrl getUrl();
    // 是否支持在页签上显示关闭按钮
    virtual bool pageClosable() { return true; }

protected:
    QPointer<PlatformPageContainer> pageContainer;
};

}  // namespace ok::platform

#endif  // !PLATFORMPAGE_H
