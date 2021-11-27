
loginForm = document.getElementById('login-form');
loginForm.addEventListener('submit',logSubmit);

usrForm = document.getElementById("usr-form");
pswForm = document.getElementById("psw-form");
errorMsg = document.getElementById("error-msg");

function logSubmit(event) {
    event.preventDefault();
    var url = `https://192.168.15.12:8000/login_data`
    var data = `{"usr":"${usrForm.value}","psw":"${pswForm.value}"}`
    
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
        if(data.result === 200){
            document.cookie = `session_id = ${data.id}; SameSite=Lax`;
            document.cookie = `session_sha = ${data.sha}; SameSite=Lax`;
            document.cookie = `session_user = ${data.user}; SameSite=Lax`;
            window.location.href = '/';
        }
        else{
            console.error('Error: en post');
            usrForm.classList.add("form-input-error");
            pswForm.classList.add("form-input-error");
            errorMsg.classList.remove("hidden");
        }
    }).catch(err=>console.error(err))
}