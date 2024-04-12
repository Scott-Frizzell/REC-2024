class Button {
    id;
    element;
    color;
    state;
    interval;

    constructor(id) {
        this.id = id;
        this.element = document.getElementById(id);
        this.color = this.element.dataset.color;
        this.state = false;
        this.interval = null;
        this.element.onclick = (e) => {
            clearInterval(this.interval);
            this.interval = null;
            this.state = false;
            this.element.style.filter = "saturate(0%) brightness(0%)";
        }
    }
}

const trackTop = [-10, 10.5];
const trackLeft = [-10, 23.2];

const gridContainer = document.getElementById("grid-container");
const trackContainer = document.getElementById("track-container");
const vehiclesContainer = document.getElementById("vehicles-container");
const sectionsContainer = document.getElementById("sections-container");
const videoContainer = document.getElementById("videos-container");
const sectionsScrollbar = document.getElementById("sections-scrollbar");
const vehiclesScrollbar = document.getElementById("vehicles-scrollbar");

let activeElement = null;

const buttons = [];

for (i = 0; i < gridContainer.children.length; i++) {
    buttons.push(new Button(gridContainer.children.item(i).id));
}

const ws = new WebSocket("ws://localhost:8080");
ws.addEventListener('open', (e) => {
    ws.send(JSON.stringify({ type: "GET_DATA", data: {}}));
});
ws.addEventListener("message", (msg) => {
    let parsed = JSON.parse(msg.data);
    switch (parsed.type) { 
        case "UPDATE_BUTTONS":
            buttons.forEach((val, idx, arr) => {
                if (parsed.data.buttons[idx].state) {
                    if (!val.interval) {
                        enableButton(val);
                        if (idx < 7) {
                            enableButton(arr[0]);
                        } else {
                            enableButton(arr[7]);
                        }
                        
                    }
                }
            });
            break;
        case "UPDATE_VEHICLES":
            for (let i = 1; i < vehiclesContainer.children.length; i++) {
                if (!parsed.data.vehicles.some((val, idx, arr) => {
                    if (val.id == vehiclesContainer.children.item(i).dataset.id) {
                        vehiclesContainer.children.item(i).innerHTML = `
                            <div id="vehicle${val.id}-indicator" style="background-color: ${val.status == "Offline" ? "red" : "lawngreen"}; filter: drop-shadow(0px 0px 8px ${val.status == "Offline" ? "red" : "lawngreen"}) blur(1px)"></div>
                            <h1>Vehicle ${val.id}</h1>
                            <p id="vehicle${val.id}-section">Current Location: ${val.status == "Online" ? "Point " + val.location : "N/A"}</p>
                            <p>Nickname: ${val.nickname}</p>
                            <p>Status: ${val.status}</p>`;
                        document.getElementById(`trackvehicle${val.id}`).style.top = `${trackTop[val.location]}vh`;
                        document.getElementById(`trackvehicle${val.id}`).style.left = `${trackLeft[val.location]}vw`;
                        arr.splice(idx, 1);
                        return true;
                    }
                    return false;
                })) {
                    vehiclesContainer.children.item(i).remove();
                    i--;
                }
            }
            parsed.data.vehicles.forEach((val) => {
                let el = document.createElement("div");
                el.className = "vehicle";
                el.id = `vehicle${val.id}`;
                el.dataset.id = val.id;
                el.innerHTML = `
                    <div id="vehicle${val.id}-indicator" style="background-color: ${val.status == "Offline" ? "red" : "lawngreen"}; filter: drop-shadow(0px 0px 8px ${val.status == "Offline" ? "red" : "lawngreen"}) blur(1px)"></div>
                    <h1>Vehicle ${val.id}</h1>
                    <p id="vehicle${val.id}-section">Current Location: ${val.status == "Online" ? "Point " + val.location : "N/A"}</p>
                    <p>Nickname: ${val.nickname}</p>
                    <p>Status: ${val.status}</p>`;
                vehiclesContainer.append(el);
                el = document.createElement("div");
                el.className = "track-vehicle";
                el.id = `trackvehicle${val.id}`;
                el.style.top = `${trackTop[val.location]}vh`;
                el.style.left = `${trackLeft[val.location]}vw`;
                el.innerHTML = `<img src="resource/vehicle.png" draggable="false">`;
                trackContainer.append(el);
            });
            updateScrollbarHeight(vehiclesScrollbar, vehiclesContainer);
            break;
        case "UPDATE_SECTIONS":
            for (let i = 1; i < sectionsContainer.children.length; i++) {
                if (!parsed.data.sections.some((val, idx, arr) => {
                    if (val.id == sectionsContainer.children.item(i).dataset.id) {
                        sectionsContainer.children.item(i).innerHTML = `
                            <div id="section${val.id}-indicator" style="background-color: ${val.vehicle != 0 ? "red" : "lawngreen"}; filter: drop-shadow(0px 0px 8px ${val.vehicle != 0 ? "red" : "lawngreen"}) blur(1px)"></div>
                            <h1>Section ${val.id}</h1>
                            <p id="section${val.id}-vehicle">Current Vehicle ID: ${val.vehicle == 0 ? "None" : val.vehicle}</p>`;
                        document.getElementById(`brake${val.id}`).style.backgroundColor = `${val.brake.status == "closed" ? "red" : "lawngreen"}`;
                        document.getElementById(`brake${val.id}`).style.filter = `drop-shadow(0px 0px 8px ${val.brake.status == "closed" ? "red" : "lawngreen"}) blur(1px)`;
                        arr.splice(idx, 1);
                        return true;
                    }
                    return false;
                })) {
                    sectionsContainer.children.item(i).remove();
                    i--;
                }
            }
            parsed.data.sections.forEach((val) => {
                let el = document.createElement("div");
                el.className = "track-section";
                el.id = `section${val.id}`;
                el.dataset.id = val.id;
                el.innerHTML = `
                    <div id="section${val.id}-indicator" style="background-color: ${val.vehicle != 0 ? "red" : "lawngreen"}; filter: drop-shadow(0px 0px 8px ${val.vehicle != 0 ? "red" : "lawngreen"}) blur(1px)"></div>
                    <h1>Section ${val.id}</h1>
                    <p id="section${val.id}-vehicle">Current Vehicle ID: ${val.vehicle == 0 ? "None" : val.vehicle}</p>`;
                sectionsContainer.append(el);
                el = document.createElement("div");
                el.className = "brake";
                el.id = `brake${val.id}`;
                el.style.top = `${val.brake.top}vh`;
                el.style.left = `${val.brake.left}vw`;
                el.style.backgroundColor = `${val.brake.status == "closed" ? "red" : "lawngreen"}`;
                el.style.filter = `drop-shadow(0px 0px 8px ${val.brake.status == "closed" ? "red" : "lawngreen"}) blur(1px)`;
                trackContainer.append(el);
            });
            updateScrollbarHeight(sectionsScrollbar, sectionsContainer);
            break;
        case "UPDATE_LAUNCH":
            document.getElementById("stat-time").innerHTML = `Time since last launch: ${Math.floor(parsed.data.timeSinceLastLaunch / 60)}:${parsed.data.timeSinceLastLaunch % 60 < 10 ? '0' : ''}${parsed.data.timeSinceLastLaunch % 60}`
            document.getElementById("stat-launches").innerHTML = `Launches this hour: ${parsed.data.launchesThisHour}`;
            document.getElementById("stat-last-launches").innerHTML = `Launches last hour: ${parsed.data.launchesLastHour}`;
        default:
            break;
    }
})

function enableBlinkButton(button) {
    if (button.interval) return;
    button.interval = setInterval(() => {
        if (button.state) {
            button.element.style.filter = "saturate(0%) brightness(0%)";
        } else {
            button.element.style.filter = `saturate(100%) brightness(100%) drop-shadow(0px 0px 15px ${button.color})`;
        }
        button.state = !button.state;
    }, 500);
}

function enableButton(button) {
    button.element.style.filter = `saturate(100%) brightness(100%) drop-shadow(0px 0px 15px ${button.color})`;
    button.state = true;
}

navigator.mediaDevices.getUserMedia({ video: true }).then((stream) => {
    var video = document.getElementById('videoFeed');
    video.srcObject = stream;
    video.play();
}).catch((err) => {
    console.log('An error occurred: ' + err);
});

sectionsScrollbar.addEventListener("mousedown", (e) => {
    sectionsScrollbar.dataset.lastPos = e.clientY;
    activeElement = {
        scrollbar: sectionsScrollbar,
        container: sectionsContainer,
        controlled: sectionsContainer.children.length > 1 ? sectionsContainer.children.item(1) : null
    }
});

vehiclesScrollbar.addEventListener("mousedown", (e) => {
    vehiclesScrollbar.dataset.lastPos = e.clientY;
    activeElement = {
        scrollbar: vehiclesScrollbar,
        container: vehiclesContainer,
        controlled: vehiclesContainer.children.length > 1 ? vehiclesContainer.children.item(1) : null
    }
});

function mover(e) {
    if (activeElement == null) return;
    let pos = parseInt(activeElement.scrollbar.dataset.pos) + e.clientY - activeElement.scrollbar.dataset.lastPos;
    activeElement.scrollbar.dataset.lastPos = e.clientY;
    if (pos < 10) {
        activeElement.scrollbar.dataset.pos, pos = 10;
    } else if (pos > activeElement.container.clientHeight - activeElement.scrollbar.clientHeight - 10) {
        activeElement.scrollbar.dataset.pos, pos = activeElement.container.clientHeight - activeElement.scrollbar.clientHeight - 10;
    }
    let childHeight = activeElement.container.children.item(1).clientHeight;
    let totalHeight = childHeight * (activeElement.container.children.length - 1);
    activeElement.scrollbar.dataset.pos = pos;
    activeElement.scrollbar.style.top = `${pos}px`;
    activeElement.controlled.style.marginTop = `-${(pos - 10) / (activeElement.container.clientHeight - activeElement.scrollbar.clientHeight - 20) * (totalHeight - activeElement.container.clientHeight)}px`;
}

document.addEventListener("wheel", (e) => {
    if (e.target == sectionsContainer || sectionsContainer.contains(e.target)) activeElement = {
        scrollbar: sectionsScrollbar,
        container: sectionsContainer,
        controlled: sectionsContainer.children.length > 1 ? sectionsContainer.children.item(1) : null
    };
    if (e.target == vehiclesContainer || vehiclesContainer.contains(e.target)) activeElement = {
        scrollbar: vehiclesScrollbar,
        container: vehiclesContainer,
        controlled: vehiclesContainer.children.length > 1 ? vehiclesContainer.children.item(1) : null
    };
    if (activeElement == null || activeElement.scrollbar.style.opacity == "0") return;
    let pos = parseInt(activeElement.scrollbar.dataset.pos) - 15 * Math.sign(e.wheelDelta);
    console.log(activeElement.scrollbar.dataset.pos)
    if (pos < 10) {
        activeElement.scrollbar.dataset.pos, pos = 10;
    } else if (pos > activeElement.container.clientHeight - activeElement.scrollbar.clientHeight - 10) {
        activeElement.scrollbar.dataset.pos, pos = activeElement.container.clientHeight - activeElement.scrollbar.clientHeight - 10;
    }
    let childHeight = activeElement.container.children.item(1).clientHeight;
    let totalHeight = childHeight * (activeElement.container.children.length - 1);
    activeElement.scrollbar.dataset.pos = pos;
    activeElement.scrollbar.style.top = `${pos}px`;
    activeElement.controlled.style.marginTop = `-${(pos - 10) / (activeElement.container.clientHeight - activeElement.scrollbar.clientHeight - 20) * (totalHeight - activeElement.container.clientHeight)}px`;
    activeElement = null;
});

function updateScrollbarHeight(scrollbar, container) {
    if (container.children.length == 1) {
        scrollbar.style.opacity = "0";
        return;
    }
    let childHeight = container.children.item(1).clientHeight;
    let totalHeight = childHeight * (container.children.length - 1);
    if (container.clientHeight + 10 > totalHeight) {
        scrollbar.style.opacity = "0";
        return;
    }
    scrollbar.style.opacity = "80%";
    scrollbar.style.height = `${ (container.clientHeight - 30) * (container.clientHeight - 10) / totalHeight }px`;
}

window.onresize = (e) => {
    updateScrollbarHeight(vehiclesScrollbar, vehiclesContainer);
    updateScrollbarHeight(sectionsScrollbar, sectionsContainer);
}

window.onload = (e) => {
    updateScrollbarHeight(vehiclesScrollbar, vehiclesContainer);
    updateScrollbarHeight(sectionsScrollbar, sectionsContainer);
}

document.addEventListener("mousemove", mover);
document.addEventListener("mouseup", (e) => { activeElement = null; });