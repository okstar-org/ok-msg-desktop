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
(function () {
    
    window.loggedin = false;

    var wsUri = "ws://localhost:65500";
    var appList = $("#app-list");
    var socket = new WebSocket(wsUri);
    function appItemClicked(event) {
        let app = event.data;
        let {name, homePage} = app;
        let data = {
            command: "app-center.openApp",
            homePage,
            name
        }
        socket.send(JSON.stringify(data));
    }

    socket.onclose = function () {
        console.error("web channel closed");
    };

    socket.onerror = function (error) {
        console.error("web channel error: " + error);
    };

    socket.onmessage = function (event) {
        console.log('Message from server', event.data);
        const app = JSON.parse(event.data);
        let appItem = $("<li><div class='app'> <img src='" + app.avatar + "' /> <span>" + app.name + "</span></div></li>")
        appList.append(appItem);
        appItem.on('click', null, app, appItemClicked)
    };

    socket.onopen = function () {

    }

})();



