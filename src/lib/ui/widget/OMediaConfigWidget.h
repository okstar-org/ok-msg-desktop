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

#include <QWidget>
#include "OVideoOutputWidget.h"
#include "base/compatiblerecursivemutex.h"
#include "lib/audio/iaudiosink.h"
#include "lib/audio/iaudiosource.h"
#include "lib/video/videomode.h"
#include "lib/ortc/ok_rtc.h"

class QPushButton;
class AudioVolumnSlider;
class QHBoxLayout;
class QStackedLayout;
class QMenu;
class QActionGroup;


namespace lib::audio{
class IAudioControl;
}

namespace lib::video {
class CameraSource;
class VideoFrame;
}  // namespace lib::video

namespace lib::ui {

class PopupMenuComboBox;
class RoundedPixmapLabel;



class OMediaConfigWidget : public QWidget {
    Q_OBJECT
public:
    explicit OMediaConfigWidget(QWidget* parent = nullptr);

    virtual void retranslateUi() = 0;

    [[nodiscard]] inline const lib::ortc::CtrlState& getCtrlState() const {
        return ctrlState;
    }

    const lib::ortc::DeviceConfig getConf();

    void addFooterButton(QPushButton* button);

    inline const QStringList& getAudioDeviceList(){
        return aDeviceList;
    }

    inline const QVector<lib::video::VideoDevice> & getVideoDeviceList(){
        return vDeviceList;
    }
protected:
    void initDeviceInfo();
    void updateAudioVideoIcon(bool audio, bool video, bool spk);

    RoundedPixmapLabel* avatarLabel = nullptr;

    lib::ui::PopupMenuComboBox* micSpeakSetting = nullptr;
    lib::ui::PopupMenuComboBox* cameraSetting = nullptr;
    lib::ui::PopupMenuComboBox* volumnSetting = nullptr;

    AudioVolumnSlider* volumnSlider = nullptr;

    QHBoxLayout* buttonLayout = nullptr;

    lib::ortc::CtrlState ctrlState;


    // audio
    QMenu* audioMenu = nullptr;
    QActionGroup* aGroup = nullptr;
    QString selectedAudio;
    lib::audio::IAudioControl* audioControl;
    std::unique_ptr<lib::audio::IAudioSource> audioSource;
    std::unique_ptr<lib::audio::IAudioSink> audioSink;
    QStringList aDeviceList;

    // video
    QMenu* videoMenu = nullptr;
    QActionGroup* vGroup = nullptr;
    QString selectedVideo;
    lib::video::VideoType selectedVideoType;
    QVector<lib::video::VideoDevice> vDeviceList;
    QStackedLayout* videoOutLayout = nullptr;
    OVideoOutputWidget* cameraOutput = nullptr;
    CompatibleRecursiveMutex mutex;

signals:
    // 状态改变事件
    // void stateChanged();

public slots:
    void audioSelected(QAction* action);
    void videoSelected(QAction* action);

    void doOpenVideo();
    void doCloseVideo();
    void closeVideo();

    void doOpenAudio();
    void doCloseAudio();
    void closeAudio();
};
}
