#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H


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
class QPushButton;

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
  void onCoreChanged(Core &core);
  void onFriendAdded(const ToxPk &friendPk, bool isFriend);
  void onFriendUsernameChanged(const ToxPk &friendPk, const QString &username);

  void onFriendStatusChanged(const ToxPk &friendPk, Status::Status status);
  void onFriendStatusMessageChanged(const ToxPk &friendPk,
                                    const QString &message);


  void onFriendAvatarChanged(const ToxPk &friendPk, const QByteArray &avatar);

  void onFriendRequestReceived(const ToxPk &friendPk, const QString &message);




  void onGroupJoined( const GroupId & groupId, const QString& name);
  void onGroupInviteReceived(const GroupInvite &inviteInfo);




void onGroupInviteAccepted(const GroupInvite &inviteInfo);

void onGroupPeerListChanged(QString groupnumber);

void onGroupPeerSizeChanged(QString groupnumber, const uint size);

void onGroupPeerNameChanged(QString groupnumber, const ToxPk &peerPk,
                            const QString &newName);
void onGroupTitleChanged(QString groupnumber, const QString &author,
                         const QString &title);

void onGroupPeerStatusChanged(QString groupnumber, QString peerPk,
                              bool  online);
void onGroupClicked();

private:
    void init();
    void deinit();
    void connectToCore(Core* core);


    Ui::ContactWidget *ui;
    Core *core;
    std::unique_ptr<FriendListWidget> contactListWidget;

    std::unique_ptr<QWidget> contentWidget;
    std::unique_ptr<ContentLayout> contentLayout;


    GroupInviteForm *groupInviteForm;
    uint32_t unreadGroupInvites;
    QPushButton *friendRequestsButton;
    QPushButton *groupInvitesButton;

};

#endif // CONTACTWIDGET_H
