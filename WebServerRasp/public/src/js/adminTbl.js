contMain = document.getElementById("content_main");
btnAddUsr = document.getElementById("btn-add-usr");

btnAddUsr.addEventListener("click",(event)=>{
    window.location.href = `/form_admin?addUser=true`;
});

document.addEventListener('DOMContentLoaded', function() {
    obtenerDatosUsuario();
 }, false);

function obtenerDatosUsuario() {
    var url = `http://127.0.0.1:8000/datos_usuarios`
    var data = `{"usr":"admin"}`
    
    fetch(url, {
        method: 'POST', // or 'PUT'
        mode: 'cors',
        body: data, // data can be `string` or {object}!
        headers:{
            'Content-Type': 'text/plain',
            'Content-Length': data.length
        }
    }).then(res => res.json())
    .then(data => {
        aux = ""
        data.map((usr)=>{
            aux += `
            <section class="main_account">
                    <h1 class="account-name">${usr.user}</h1>
                    <a id="btn-conf-${usr.user}" class="btn"><img src="./src/assets/config2-icon.png" alt=""></a>
                    <a id="btn-del-${usr.user}" class="btn"><img src="./src/assets/delete-icon1.png" alt=""></a>
            </section>
            `
        });
        contMain.innerHTML = aux;
        
        data.map((usr)=>{
            btnConf = document.getElementById(`btn-conf-${usr.user}`);
            btnDel = document.getElementById(`btn-del-${usr.user}`);
            
            btnConf.addEventListener('click', function() {
                onConfigHandler(usr.user);
             }, false);  
             
             btnDel.addEventListener('click', function() {
                onDeleteHandler(usr.user);
             }, false);  
        })

    }).catch(err=>console.error(err))
}

function onConfigHandler(id) {
    console.log(`configurar ${id}`);
    window.location.href = `/form_admin?user=${id}`;
}

function onDeleteHandler(id) {
    console.log(`Borrar ${id}`);
    window.location.href = `/eliminar_usuario?user=${id}`;
}