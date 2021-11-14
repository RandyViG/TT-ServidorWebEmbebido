var ctx = document.getElementById('myChart').getContext('2d');
myChart = new Chart(ctx, {
    type: 'doughnut',
    data: {
        datasets: [{
          label: 'My First Dataset',
          data: [graph_data["temp"],50-graph_data["temp"]],
          backgroundColor: [
            'rgb(255, 99, 132)',
            'rgb(255, 255, 255)'
          ],
          hoverOffset: 4
        }]
      },
      options: {
        events: []
      }
});

var ctx = document.getElementById('myChart2').getContext('2d');
myChart2 = new Chart(ctx, {
    type: 'doughnut',
    data: {
        datasets: [{
          label: 'My First Dataset',
          data: [graph_data["gas"],5-graph_data["gas"]],
          backgroundColor: [
            'rgb(255, 205, 86)',
            'rgb(255, 255, 255)'
          ],
          hoverOffset: 4
        }],
    },
    options: {
      events: []
    }
});

var ctx = document.getElementById('myChart3').getContext('2d');
myChart3 = new Chart(ctx, {
    type: 'doughnut',
    data: {
        datasets: [{
          label: 'My First Dataset',
          data: [graph_data["hum"],100-graph_data["hum"]],
          backgroundColor: [
            'rgb(54, 162, 235)',
            'rgb(255, 255, 255)'
          ],
          hoverOffset: 4
        }]
      },
      options: {
        events: []
      }
});
