#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H


#include <QPushButton>
#include <QWidget>
#include <memory>

#include "friendlistwidget.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/status.h"
#include "src/widget/MainLayout.h"
#include "src/widget/friendlistwidget.h"

namespace Ui {
class ContactWidget;
}

class ToxPk;
class Core;
class GroupInviteForm;
class AddFriendForm;

/**
 * 通讯录界面
 * @brief ContactWidget::ContactWidget
 */
class ContactWidget : public MainLayout
{
    Q_OBJECT

public:
    explicit ContactWidget(QWidget *parent = nullptr);
    ~ContactWidget();

    [[__nodiscard__]]  ContentLayout* getContentLayout() const override{
        //MUST to be initialized.
        assert(contentLayout);
        return contentLayout.get();
    }

public slots:
    /**
     * do开头接受界面事件，调用core执行操作
     * on开头接受core事件，操作界面
    */

  void do_openAddForm();

  void onCoreChanged(Core &core);
  void onFriendAdded(const ToxPk &friendPk, bool isFriend);
  void onFriendUsernameChanged(const ToxPk &friendPk, const QString &username);

  void onFriendStatusChanged(const ToxPk &friendPk, Status::Status status);
  void onFriendStatusMessageChanged(const ToxPk &friendPk,
                                    const QString &message);


  void onFriendAvatarChanged(const ToxPk &friendPk, const QByteArray &avatar);

  void onFriendRequest(const ToxPk &friendPk, const QString &message);


  void do_friendDelete(const ToxPk &friendPk);
  //朋友请求
  void do_friendRequest(const ToxPk& friendAddress, const QString& nick, const QString& message);
  //朋友请求接受
  void do_friendRequestAccept(const ToxPk &friendPk);
  //朋友请求拒绝
  void do_friendRequestReject(const ToxPk &friendPk);

  void onGroupJoined( const GroupId & groupId, const QString& name);
  void onGroupInfoReceived(const GroupId & groupId, const GroupInfo& info);
  void onGroupInviteReceived(const GroupInvite &inviteInfo);


    void onGroupInviteAccepted(const GroupInvite &inviteInfo);

    void onGroupPeerListChanged(QString groupnumber);

    void onGroupPeerSizeChanged(QString groupnumber, const uint size);

    void onGroupPeerNameChanged(QString groupnumber, const ToxPk &peerPk,
                                const QString &newName);
    void onGroupTitleChanged(QString groupnumber, const QString &author,
                             const QString &title);

    void onGroupPeerStatusChanged(const QString & groupnumber,const GroupOccupant &go);
    void onGroupClicked();


private:
    void init();
    AddFriendForm* makeAddForm();
    void deinit();
    void connectToCore(Core* core);
    void friendRequestsUpdate() ;


    Ui::ContactWidget *ui;
    Core *core;
    FriendListWidget* contactListWidget;

    std::unique_ptr<QWidget> contentWidget;
    std::unique_ptr<ContentLayout> contentLayout;


    AddFriendForm *addForm;

    GroupInviteForm *groupInviteForm;
    uint32_t unreadGroupInvites;
    QPushButton *friendRequestsButton;
    QPushButton *groupInvitesButton;

};

#endif // CONTACTWIDGET_H
