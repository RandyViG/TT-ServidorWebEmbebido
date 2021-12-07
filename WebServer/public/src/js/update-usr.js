usrForm = document.getElementById("lbl-nombre-usr");
pswForm = document.getElementById("lbl-psw");
ConfPswForm = document.getElementById("lbl-conf-psw");
formUsr = document.getElementById("form-up-usr");
errorMsg = document.getElementById("error-msg");

formUsr.addEventListener('submit',upUsrSubmin);

function upUsrSubmin(event) {
    event.preventDefault();
    var url = `https://${server}/edit_usuario`
    var data = `{"usr":"${usrForm.value}","psw":"${pswForm.value}","session":${obtSessionID()}}`
    
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
            window.location.href = '/logout';
        }else{
            usrForm.classList.add("form-input-error");
            pswForm.classList.add("form-input-error");
            ConfPswForm.classList.add("form-input-error");
            errorMsg.classList.remove("hidden");

            usrForm.value = "";
            pswForm.value = "";
            ConfPswForm.value = "";
            errorMsg.value = "";
        }
    }).catch(err=>console.error(err))
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