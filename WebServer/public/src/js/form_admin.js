formNodo = document.getElementById("lbl-sensor");
usrForm = document.getElementById("lbl-nombre-usr");
pswForm = document.getElementById("lbl-psw");
emailForm = document.getElementById("lbl-mail");
confPswForm = document.getElementById("lbl-conf-psw");
formUp = document.getElementById("form-up-usr");

errorMsg = document.getElementById("error-msg");

formNodo.addEventListener("change",(event)=>{
    nodo = event.target.value;
    if(nodo < 0)
        event.target.value = 0;
});

formUp.addEventListener('submit',upUsrSubmin);

function upUsrSubmin(event) {
    event.preventDefault();
    
    if (validarVacios()) 
    errorEvent();
    else
    if(validarPsw(pswForm.value,confPswForm.value)){
            var addUserBool = obtVariable("addUser");
            var url = '';
            var data = '';
            console.log("Contraseñas iguales");
            // console.log(addUserBool);
            if(addUserBool === "true"){
                url = `https://192.168.15.12:8000/agregar_usuario`
                data = `{"usr":"${usrForm.value}","psw":"${pswForm.value}","email":"${emailForm.value}","nodo":${formNodo.value},"session":${obtSessionID()}}`
            }
            else{
                url = `https://192.168.15.12:8000/edit_usuario`
                data = `{"usr":"${usrForm.value}","psw":"${pswForm.value}","target":"${obtVariable("user")}","nodo":${formNodo.value},"email":"${emailForm.value}","session":${obtSessionID()}}`
            }

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
                console.log(data)
                if(data.result == 200){
                    window.location.href = '/configuracion';
                }else{
                    errorEvent();
                }
            }).catch(err=>console.error(err))
        }else{
            console.log("Contraseñas diferentes");
            errorEvent();
        }
}

function errorEvent() {
    usrForm.classList.add("form-input-error");
    pswForm.classList.add("form-input-error");
    confPswForm.classList.add("form-input-error");
    emailForm.classList.add("form-input-error");
    errorMsg.classList.remove("hidden");

    usrForm.value = "";
    pswForm.value = "";
    confPswForm.value = "";
}

function validarPsw(psw,conf){
    if(psw===conf){
        return true;
    }else{
        return false;
    }
}

function obtSessionID() {
    let arr = document.cookie.split(';');
    for(let atr of arr){
        let aux = atr.split('=')
        if(aux[0].replace(" ", "") === 'session_id'){
            return aux[1];
        }
    }
}

function obtVariable(variable) {
    var url_string = window.location.href
    var url = new URL(url_string);
    var c = url.searchParams.get(variable);
    return c;
}

function validarVacios() {
    if (pswForm.value === "") return true;
    if (confPswForm.value === "") return true; else return false;
}