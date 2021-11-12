lblUser = document.getElementById("nav_usr_name");
setName();

btnLogout = document.getElementById("btn-logout");
btnSettings = document.getElementById("btn-settings");
btnDashboard = document.getElementById("btn-dashboard");

btnLogout.addEventListener('click',logout);

const socket = new WebSocket('ws://localhost:8000');

// Abre la conexión
socket.addEventListener('open', function (event) {
    socket.send('Hello Server!');
});

// Escucha por mensajes
socket.addEventListener('message', function (event) {
    console.log('Message from server', event.data);
});

function setName() {
    let arr = document.cookie.split(';');
    for(let atr of arr){
        let aux = atr.split('=')
        if(aux[0].replace(" ", "") === 'session_user'){
            lblUser.append(aux[1]);
            break;
        }
    }
}

function logout(){
    window.location.href = '/logout';
}


