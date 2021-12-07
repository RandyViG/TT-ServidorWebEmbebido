const ip = `http://${server}`
const port = 8000
const url = `https://${server}/datos_sensor`;
var data = `{"sesion":${obtSessionID()}}`;

const lblTemp = document.getElementById("temp-data");
const lblGas = document.getElementById("gas-data");
const lblHum = document.getElementById("hum-data");


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
        console.log(data);
        const socket = new WebSocket(`wss://${websocket}`);
        // Abre la conexión
        socket.addEventListener('open', function (event) {
            socket.send('Open conection!');
        });

        // Escucha por mensajes
        socket.addEventListener('message', async function (event) {
            var datos = event.data;
            console.log('Data: ', datos);
            sensores = JSON.parse(datos)

            if(datos !== '{}'){
                graph_data["temp"] = sensores["temp"];
                graph_data["hum"] = sensores["hum"];
                graph_data["gas"] = sensores["gas"];
                  
                lblTemp.textContent = `${sensores["temp"]} °C`;
                lblHum.textContent = `${sensores["hum"]} %RH`;
                lblGas.textContent = `${sensores["gas"]} %LEL`;

                if(sensores["gas"] >= 1.5 && sensores["gas"] <= 2.5){
                    swal({
                        title: "Urgente!",
                        text: "Se detecto un valor anormal en el nivel de gas LP",
                        icon: "warning",
                        dangerMode: true,
                      })
                }
            }
            actualizar_grafo();
            // console.log("Datos_graf: ",graph_data);
            await new Promise(r => setTimeout(r, 1000));
            socket.send('SMO');
        });
    }
    else{
        console.error('Error: en ws');
    }
}).catch(err=>console.error(err))

function actualizar_grafo(){
    myChart.data.datasets[0].data[0] = graph_data["temp"];
    myChart.data.datasets[0].data[1] = 50-graph_data["temp"];

    myChart2.data.datasets[0].data[0] = graph_data["gas"];
    myChart2.data.datasets[0].data[1] = 5-graph_data["gas"];

    myChart3.data.datasets[0].data[0] = graph_data["hum"];
    myChart3.data.datasets[0].data[1] = 100-graph_data["hum"];

    myChart.update();
    myChart2.update();
    myChart3.update();
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

function obtSessionID() {
    let arr = document.cookie.split(';');
    for(let atr of arr){
        let aux = atr.split('=')
        if(aux[0].replace(" ", "") === 'session_id'){
            return aux[1];
        }
    }
}