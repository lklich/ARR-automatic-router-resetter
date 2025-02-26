document.addEventListener('DOMContentLoaded', function() {
    var dhcpChecked = document.getElementById('DHCP').textContent;
    var enabledChecked = document.getElementById('ENABLED').textContent;
    var mqttChecked = document.getElementById('MQTT').textContent;
    var timeValue = document.getElementById('TIME').textContent;
    document.getElementById('dhcp').checked = dhcpChecked == '1';
    document.getElementById('enabled').checked = enabledChecked == '1';
    document.getElementById('time').value = timeValue;
    document.getElementById('mqtt').checked = mqttChecked == '1';

    document.getElementById('networkCombo').addEventListener('change', function() {
      const selectedSSID = this.value;
      document.getElementById('ssid').value = selectedSSID;
      document.getElementById('wifipass').value = "";
    });
      fetchNetworks();
});

function fetchNetworks() {
  fetch('/ssid')
    .then(response => response.json())
    .then(data => {
      const combo = document.getElementById('networkCombo');
      data.networks.forEach(network => {
        const option = document.createElement('option');
        option.value = network.ssid;
        option.textContent = `${network.ssid} (${network.rssi} dBm)`;
        combo.appendChild(option);
      });
    })
    .catch(error => console.error('Data error:', error));
}

function start_handler() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/", true);
  xhr.send();
  setTimeout(function () { window.open("/", "_self"); }, 1000);
}

function clear_handler() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/clearlog", true);
  xhr.send();
  setTimeout(function () { window.open("/", "_self"); }, 1000);
}

function logout_handler() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function () { window.open("/logged-out", "_self"); }, 1000);
  setTimeout(function () { window.open("/", "_self"); }, 1000);
}

function reboot_handler() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/reboot", true);
  xhr.send();
  setTimeout(function () { window.open("/reboot", "_self"); }, 500);
}

function _(el) {
    return document.getElementById(el);
}
