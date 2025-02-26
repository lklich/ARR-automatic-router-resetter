document.addEventListener('DOMContentLoaded', function() {
    function refresh() {
      var myIP = document.getElementById("myIP").textContent;
      fetch('http://' + myIP + '/data')
        .then(response => response.json())
        .then(data => {
          var my_state = data.connected;
          if ((my_state == "0") && (data.counterr > 0)) {
             state1 = "Disconnect"; 
            } else {
              state1 = "OK";  
            }
          if (data.counterr > 0) {
            state1 = "Connect error"; 
          } else {
            state1 = "OK";  
          }
 
          document.getElementById('state').innerHTML = state1;
          document.getElementById('enable').innerHTML = data.enable;
          document.getElementById('timeok').innerHTML = SecToTime(data.countok * 5);
          document.getElementById('countok').innerHTML = data.countok;
          document.getElementById('counterr').innerHTML = data.counterr;
          document.getElementById('wifi').innerHTML = data.rssip + "%";
        });
    }
    setInterval(refresh, 1000);
  });
  
  function SecToTime(seconds) {
    let hours = Math.floor(seconds / 3600);
    let minutes = Math.floor((seconds % 3600) / 60);
    let remainingSeconds = seconds % 60;
    let formattedTime = [
        String(hours).padStart(2, '0'),
        String(minutes).padStart(2, '0'),
        String(remainingSeconds).padStart(2, '0')
    ].join(':');
    return formattedTime;
}

async function loadFile() {
  try {
    const response = await fetch('/nettest');
    if (response.ok) {
      const text = await response.text();
      document.getElementById('log').value = text;
    } else {
      console.error('Network error: ', response.status);
    }
  } catch (error) {
    console.error('Error: ', error);
    }
  }  
window.onload = loadFile;
  