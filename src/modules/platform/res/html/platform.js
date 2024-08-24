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
    var appList = document.querySelector("#app-list");
    var socket = new WebSocket(wsUri);
    function appItemClicked(app) {
        let {name, homePage, uuid, author, type} = app;
        let data = {
            command: "app-center.openApp",
            homePage,
            name,
            uuid,
            author,
            type
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

        let appItem = document.createElement("li");
        // appItem.innerHTML = `<div class="app">
        // <img src="${app.avatar}" /> <span>" + app.name + "</span></div>`;

        appItem.innerHTML = `
        <li>
            <div class='app'>
                <div class="image">
                    <img src="${app.avatar}">
                </div>
                <div class="name">${app.name}</div>
                <a class="author" title="由 ${app.author} 提供">${app.author}</a>
             </div>
        </li>`;

        appList.appendChild(appItem);

        appItem.addEventListener("click", (event) => {
            appItemClicked(app);
        });
    };

    socket.onopen = function () {

    }

})();



