var ctx = document.getElementById('myChart').getContext('2d');
var myChart = new Chart(ctx, {
    type: 'doughnut',
    data: {
        datasets: [{
          label: 'My First Dataset',
          data: [100,200],
          backgroundColor: [
            'rgb(255, 99, 132)',
            'rgb(255, 255, 255)'
          ],
          hoverOffset: 4
        }]
      }
});

var ctx = document.getElementById('myChart2').getContext('2d');
var myChart2 = new Chart(ctx, {
    type: 'doughnut',
    data: {
        datasets: [{
          label: 'My First Dataset',
          data: [50,250],
          backgroundColor: [
            'rgb(255, 205, 86)',
            'rgb(255, 255, 255)'
          ],
          hoverOffset: 4
        }],
    }
});

var ctx = document.getElementById('myChart3').getContext('2d');
var myChart3 = new Chart(ctx, {
    type: 'doughnut',
    data: {
        datasets: [{
          label: 'My First Dataset',
          data: [40,160],
          backgroundColor: [
            'rgb(54, 162, 235)',
            'rgb(255, 255, 255)'
          ],
          hoverOffset: 4
        }]
      }
});

swal({
  title: "Urgente!",
  text: "Se detecto un valor anormal en el nivel de gas LP",
  icon: "warning",
  dangerMode: true,
})