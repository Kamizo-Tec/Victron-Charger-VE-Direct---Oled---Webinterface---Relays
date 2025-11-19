#pragma once

#include <Arduino.h>  // PROGMEM

static const char HTML_index[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<!--<html lang="en">  -->
<head>
  <meta charset="UTF-8">
 <title>Victron 100/20</title> 
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>

    body {
      background-color: #121212;
      color: #e0e0e0;
      font-family: Arial, sans-serif;
      padding: 12px;
      font-size: 20px; 
    }

    .container {
      max-width: 640px;
      margin: auto;
      background-color: #1e1e1e;
      padding: 5px;
      border-radius: 8px;
    }

    .progress-bar {
      background-color: #333;
      border-radius: 4px;
      height: 20px;
      overflow: hidden;
    }

    .progress-fill {
      background-color: #009dd1;
      height: 100%;
      width: 0%;
      text-align: center;
      color: #fff;
      line-height: 20px;
    }

    .alert {
      background-color: #b71c1c;
      color: #fff;
      text-align: center;
      padding: 8px;
      border-radius: 5px;
      display: none;
    }

    .button {
      display: block;
      width: 100%; /*96*/
      padding: 10px;
      background-color: #1976d2;
      color: #fff;
      text-align: center;
      text-decoration: none;
      border-radius: 5px;
      margin-top: 3px; /* 8 - abstand buttons*/
    }
      /* Hover-Effekt */
      button:hover {
      background: #ff7f50
  }
    /* Aktiver Button (z.B. bei Klick) */
      button.active {
      background: #c0392b;
      box-shadow: 0 0 8px #060606ff;
    }

.section {
  width: 96%;   /* widh infoelements */
  overflow: hidden;
  margin-bottom: 10px;
  padding: 6px;
  border-radius: 5px;
  background-color: #0099ff; /* #395a7f; Standard: Blau */
  color: white;
  /* border: none; */
  /* text-align: left; */
 
}

.label {
  float: left;
  font-weight: bold;
  width: 100px;
}

.value {
  text-align: center;
  margin-left: 100px; /* Platz für das Label */
  display: block;
}

.value span {
  display: inline-block;
  margin: 0 25px; /* Abstand zwischen Volt und Ampere */
}


/* Relay-Button bekommt eigene Hintergrundfarbe dynamisch */

.relay-button {
   font-size: 20px; 
  cursor: pointer;
  width: 100%;
  border: none;
  text-align: left;
  transition: background-color 0.3s ease; /* sanfter Übergang */
}

/* TOGGLE MENUE CCS ------------------------------------ */

/* Menücontainer */

#menu {
  display: none;
  background: #2b2b2b;
  border-radius: 6px;
  padding: 15px;
  margin-top: 10px;
  text-align: center; /* ersetzt Flex-Zentrierung */
}

/* Menü anzeigen bei Checkbox */
#nav-toggle-cb {
  display: none;
}

#nav-toggle-cb:checked + #menu {
  display: block; /* statt flex */
}

/* Toggle-Button */
#nav-toggle {
  cursor: pointer;
  display: block;
  padding: 10px;
  background: #0099ff;
  color: #fff;
  text-align: center;
  border-radius: 5px;
}

/* Switch ohne Animation */
.switch {
  position: relative;
  display: inline-block;
  width: 50px;
  height: 22px;
  margin: 10px;
}

.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

/* Schieberfläche */
.slider {
  position: absolute;
  background-color: grey;
  border-radius: 22px;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
}

/* Schiebeknopf */
.slider:before {
  position: absolute;
  content: "";
  height: 16px;
  width: 16px;
  left: 4px;
  top: 3px;
  background-color: white;
  border-radius: 50%;
}

/* Zustand EIN ohne transform */
input:checked + .slider {
  background-color: #0099ff;
}
input:checked + .slider:before {
  left: 30px; /* feste Position statt translateX */
}

/* Checkbox-Bereich */
.checkbox-container {
  text-align: center;
  margin-top: 20px;
}

/* Switch-Labels */
label {
  margin: 0 10px;
}


  </style>

</head>

<body onload="get_values()">

  <noscript>
    <div class="alert"><strong>JavaScript ist deaktiviert. Bitte aktivieren!</strong></div>
  </noscript>


  <div class="container">
  <!-- //////   all things in container  //////////////////////////  -->
  
 <!-- Toggle-Button -->

<div class="section" id="device-toggle" style="cursor: pointer;">
  <span class="label">Device</span>
  <span class="value" id="deviceName"></span>
</div>

<input type="checkbox" id="nav-toggle-cb" style="display:none;">

 <!-- //////   MENUE ITEMS  //////////////////////////  -->

<!-- switchmenue buttons -->
<div id="menu">
  <div style="text-align:center;">

  <!-- wr Relay------------------------------------- -->
<button id="wr_relay" class="section relay-button"
  onclick="toggle_wr()">
  <span class="label">Inverter </span>
  <span class="value" id="wr_relaystate">OFF</span>
</button>

<!-- Pc Relay ------------------------------------------ -->
<button id="pc_relay" class="section relay-button"
  onclick="togglePc_Relay()">
  <span class="label">Pc-NP93</span>
  <span class="value" id="pc_relaystate">OFF</span>
</button>


  <!--SETTINGS MENUE  -------------------------- -->

<hr style="width:100%">

<table width="550" border="0" cellspacing="0" cellpadding="0" align="center">
  <tr> 
    <td bgcolor="#0099ff"><font color="#FFFFFF" size="3" > > cooler</font></td>
    <td bgcolor="#0099ff" colspan="3"> 
      <div align="center"><font color="#FFFFFF" size="3">temp</font></div>
    </td>
    <td bgcolor="#0099ff"><b></b></td>
    <td bgcolor="#0099ff"> 
      <div align="center"><font color="#FFFFFF" size="3">status</font></div>
    </td>
    <td bgcolor="#0099ff"><b></b></td>
    <td><b></b></td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
    <td>&nbsp;</td>
  </tr>
  <!-- FAN TEMP --> 
  <tr height="40"> 
    <td> 
      <div align="left"><label><font size="3">start Fan </font></label></div>
    </td>
    <td width="30"> 
      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('fan_t', -1)" 
          onMouseUp="sendValue('fan_t', fan_t)">-</button> </div>
    </td>
    <td width="60"> 
      <div style="text-align:center;"> 
        <input type="text" id="fan_t" readonly style="width: 36px; height:26px; font-size:13px; margin: 0 4px;" name="text32">
      </div>
    </td>
    <td width="30"> 
      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('fan_t', 1)" 
          onMouseUp="sendValue('fan_t', fan_t)">+</button> </div>
    </td>
    <td width="10"></td>
    <td width="80"> 
      <div id="fanstate" style="width:100px;height:26px;background-color:#395a7f;color:white;text-align:center;line-height:26px;border-radius:4px;font-size:13px;"> 
        Fan off </div>
    </td>
    <td width="10"></td>
    <td width="80">&nbsp;</td>
    <td width="10">&nbsp;</td>
    <td width="30">&nbsp;</td>
    <td width="60">&nbsp;</td>
    <td width="30">&nbsp;</td>
    <td width="10">&nbsp;</td>
  </tr>
  <!-- STOP VOLT --> <!-- FAN TEMP --> 
</table>

<table border="0" cellspacing="0" cellpadding="0" align="center" width="550">
  <tr> 
    <td bgcolor="#0099ff"><font color="#FFFFFF" size="3"> > settings</font></td>
    <td bgcolor="#0099ff" colspan="3">
      <div align="center"><font color="#FFFFFF" size="3">delay</font></div>
    </td>
    <td bgcolor="#0099ff"><b></b></td>
    <td bgcolor="#0099ff">
      <div align="center"><font color="#FFFFFF" size="3">status</font></div>
    </td>
    <td bgcolor="#0099ff"><b></b></td>
    <td bgcolor="#0099ff"><b></b></td>
    <td bgcolor="#0099ff">&nbsp;</td>
    <td bgcolor="#0099ff" width="30">&nbsp;</td>
    <td bgcolor="#0099ff">
      <div align="center"><font color="#FFFFFF" size="3">volt</font></div>
    </td>
    <td bgcolor="#0099ff">&nbsp;</td>
    <td bgcolor="#0099ff">&nbsp;</td>
  </tr>
  <!-- START VOLT --> 
  <tr height="40"> 
    <td> 
      <div align="left"><label><font size="3">start:</font></label></div>
    </td>
    <td width="30"> 
      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('start_delay', -1)" 
          onMouseUp="sendValue('start_delay', start_delay)">-</button> </div>
    </td>
    <td width="60"> 
      <div style="text-align:center;"> 
        <input type="text" id="start_delay" readonly style="width: 36px; height:26px; font-size:13px; margin: 0 4px;" name="text3">
      </div>
    </td>
    <td width="30"> 
      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('start_delay', 1)" 
          onMouseUp="sendValue('start_delay', start_delay)">+</button> </div>
    </td>
    <td width="10"></td>
    <td width="80"> 
      <div id="onTimer" style="width:100px;height:26px;background-color:#395a7f;color:white;text-align:center;line-height:26px;border-radius:4px;font-size:13px;"> 
        Timer off </div>
    </td>
    <td width="10"></td>
    <td width="80"> 
      <div id="startCountdown" style="width:80px;height:26px;background-color:#395a7f;color:white;text-align:center;line-height:26px;border-radius:4px;font-size:13px;"> 
        -- min </div>
    </td>
    <td width="10">&nbsp;</td>
    <td width="30">      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('start_inverter', -1)" 
          onMouseUp="sendValue('start_inverter', start_inverter)">-</button> </div></td>
    <td width="60">      <div style="text-align:center;"> 
        <input type="text" id="start_inverter" readonly style="width: 36px; height:26px; font-size:13px; margin: 0 4px;" name="text3">
      </div></td>
    <td width="30">      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('start_inverter', 1)" 
          onMouseUp="sendValue('start_inverter', start_inverter)">+</button> </div></td>
    <td width="10">&nbsp;</td>
  </tr>
  <!-- STOP VOLT --> 
  <tr height="40"> 
    <td> 
      <div align="left"><label><font size="3">stop:</font></label></div>
    </td>
    <td> 
      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('stop_delay', -1)" 
          onMouseUp="sendValue('stop_delay', stop_delay)">-</button> </div>
    </td>
    <td> 
      <div style="text-align:center;"> 
        <input type="text" id="stop_delay" readonly style="width: 36px; height:26px; font-size:13px; margin: 0 4px;" name="text3">
      </div>
    </td>
    <td> 
      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('stop_delay', 1)" 
          onMouseUp="sendValue('stop_delay', stop_delay)">+</button> </div>
    </td>
    <td></td>
    <td> 
      <div id="offTimer" style="width:100px;height:26px;background-color:#395a7f;color:white;text-align:center;line-height:26px;border-radius:4px;font-size:13px;"> 
        Timer off </div>
    </td>
    <td></td>
    <td> 
      <div id="stopCountdown" style="width:80px;height:26px;background-color:#395a7f;color:white;text-align:center;line-height:26px;border-radius:4px;font-size:13px;"> 
        -- min </div>
    </td>
    <td>&nbsp;</td>
    <td>      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('stop_inverter', -1)" 
          onMouseUp="sendValue('stop_inverter', stop_inverter)">-</button> </div></td>
    <td>      <div style="text-align:center;"> 
        <input type="text" id="stop_inverter" readonly style="width: 36px; height:26px; font-size:13px; margin: 0 4px;" name="text3">
      </div></td>
    <td>      <div style="text-align:center;"> <button style="width:26px;height:26px;font-size:13px;" 
          onMouseDown="changeValue('stop_inverter', 1)" 
          onMouseUp="sendValue('stop_inverter', stop_inverter)">+</button> </div></td>
    <td>&nbsp;</td>
  </tr>
  <!--  --> 
</table>





        <hr style="width:100%">

    <!--CHECKBOXEN  AutoMode FAN Reset -------------------------- -->

    <div class="checkbox-container">

      <label for="reset">reset ESP</label>
      <label class="switch">
        <input id="resetesp" type="checkbox">
        <span class="slider"></span>
      </label>


      <label for="auto">AutoMode</label>
      <label class="switch">
        <input id="autoMode" type="checkbox">
        <span class="slider"></span>
      </label>


   <hr style="width:100%">

<!-- Switch Display------------------------------------- -->
<button id="switchdisplay" class="section relay-button" onclick="toggleDisplay()">
  <span class="label">Display</span>
  <span class="value" id="displaystate">ON</span>
</button>

  <!-- link All Values --------------------------------------- -->
<button id="allvaluesbtn" class="section relay-button" onclick="openAllValues()">
  <span class="label">ESP info</span>
  <span class="value"></span>
</button>

    </div>
  </div>
</div>

 <!-- //////   MENU END  //////////////////////////  -->

 <!--<hr style="border-color:#444;"> -->

 <!-- //////   DEVICE INFOS  //////////////////////////  -->

<div class="alert" id="vcc_alert"><strong>WARNING ESP VOLTAGE TOO LOW</strong></div>

<div class="section">
  <span class="label">Solar</span>
  <span class="value">
    <span id="panelvolt"></span> /
    <span id="panelcurrent"></span>
  </span>
</div>

     <div class="section">
     <span class="label">Battery</span>
     <span class="value">
     <span id="battvolt"></span> /
     <span id="battcurrent"></span>
     </span>
     </div>


   <div class="section">
     <span class="label">MppT</span>
     <span class="value">
       <span id="trackermode"></span> /
        <span id="operationstate"></span>
     </span>
    </div>


   <div class="section">
     <span class="label">Vic/Bat</span>
       <span class="value">
        <span id="temp_Victron"></span> /
       <span id="temp_Battery"></span>
      </span>
    </div>

     <div class="section"><span class="label">Runtime </span><span class="value" id="runtime"></span></div>
  

<!-- ================== SCRIPT 1: JSON / Victron ================== -->

<script>
var websocket;

function initWebSocket() {
  var gateway = "ws://" + window.location.hostname + "/ws";
  websocket = new WebSocket(gateway);

  websocket.onopen = function() {
    console.log("WebSocket verbunden");
    setInterval(function() {
      if (websocket.readyState !== WebSocket.CLOSED) {
        //websocket.send("connected_allvalues");
      }
    }, 5000);
  };

  
// Toggle Menue Checkbox mit autofunktion ---------------------------

var toggleDiv = document.getElementById("device-toggle");
var navCheckbox = document.getElementById("nav-toggle-cb");
var resetTimer = null;

if (toggleDiv && navCheckbox) {
  toggleDiv.onclick = function() {
    navCheckbox.checked = !navCheckbox.checked;

    if (navCheckbox.checked) {
      toggleDiv.style.backgroundColor = "tomato";

      // Funktion aufrufen, wenn aktiviert
      get_values();

      // Timer zurücksetzen
      if (resetTimer) {
        clearTimeout(resetTimer);
      }

      resetTimer = setTimeout(function() {
        navCheckbox.checked = false;
        toggleDiv.style.backgroundColor = "#395a7f";

         //  Speichern beim automatischen Zuklappen
        saveAll();

      }, 1800000); // 30 Minuten
    } else {
      toggleDiv.style.backgroundColor = "#395a7f";

      if (resetTimer) {
        clearTimeout(resetTimer);
        resetTimer = null;
      }

      //  Speichern beim manuellen Zuklappen
      saveAll();

    }
  };
}

//----------------WEBSOCKET JSON ------------------------------------------------------

  websocket.onmessage = function(event) {

  const msg = event.data; // define the msg

    if (event.data.charAt(0) === '{') {
      var b = JSON.parse(event.data);

      // Victron-Daten -----------------------------------------------
        if (b.victron) {

        //  document.getElementById("panelvolt").innerHTML = b.victron.Panel_voltage + " V";
      //    document.getElementById("panelcurrent").innerHTML = b.victron.Panel_power + " W";
         // document.getElementById("battvolt").innerHTML = b.victron.Voltage + " V";
        //  document.getElementById("battcurrent").innerHTML = parseFloat(b.victron.Battery_current).toFixed(1) + " A";

        document.getElementById("panelvolt").innerHTML = parseFloat(b.victron.Panel_voltage).toFixed(1) + " V";
        document.getElementById("panelcurrent").innerHTML = parseFloat(b.victron.Panel_power).toFixed(0) + " W";
        document.getElementById("battvolt").innerHTML = parseFloat(b.victron.Voltage).toFixed(1) + " V";
        document.getElementById("battcurrent").innerHTML = parseFloat(b.victron.Battery_current).toFixed(1) + " A";
        document.getElementById("operationstate").innerHTML = b.victron.Operation_state_text;
        document.getElementById("trackermode").innerHTML = b.victron.Tracker_operation_mode_text;

        } else {
          // Victron offline → Platzhalter setzen
        document.getElementById("panelvolt").innerHTML = "-";
        document.getElementById("panelcurrent").innerHTML = "-";
        document.getElementById("battvolt").innerHTML = "-";
        document.getElementById("battcurrent").innerHTML = "-";
        document.getElementById("operationstate").innerHTML = "-";
        document.getElementById("trackermode").innerHTML = "-";
        }

// ESP-Daten
        document.getElementById("deviceName").textContent = b.Device_name || b.Device_model;
        document.getElementById("temp_Victron").innerHTML = (b.Temp_Victron) + " °C";
        document.getElementById("temp_Battery").innerHTML = (b.Temp_Battery) + " °C";
        document.getElementById("runtime").innerHTML = formatRuntimeCompact(b.Runtime);
// VCC-Warnung
        document.getElementById("vcc_alert").style.display = b.ESP_VCC < 2.6 ? "block" : "none";

        } else {
    // All Textmessages → zentrale Funktion
    handleAllMessages(msg);

    }
  };

  websocket.onerror = function() {
    console.log("WebSocket Fehler");
  };

  websocket.onclose = function() {
    console.log("WebSocket getrennt");
    setTimeout(initWebSocket, 3500);
  };
}

  window.onload = function() {
  initWebSocket();
};

</script>


<!-- ================== SCRIPT 2: Relais + Hilfsfunktionen ================== -->

<script>

function openAllValues() {
  window.open("/allvalues", "_blank");
}

// toggle wr_relay ---------------------------------------------

function toggle_wr() {
  var val = document.getElementById("wr_relaystate");
  if (val.innerHTML === "ON") {
    websocket.send("wr:off");
  } else {
    websocket.send("wr:on");
  }
}


// pc_relay ------------------------------------------------
function togglePc_Relay() {
  var val = document.getElementById("pc_relaystate");
  if (val.innerHTML === "ON") {
    websocket.send("pc:off");
  } else {
    websocket.send("pc:on");
  }
}

// Display ------------------------------------------------------------
function toggleDisplay() {
  var val = document.getElementById("displaystate");
  if (val.innerHTML === "ON") {
    websocket.send("display:off");
  } else {
    websocket.send("display:on");
  }
}

// FAN 
function toggleFan() {
  var val = document.getElementById("fanstate");
  var currentState = val.innerText.trim();

  if (currentState === "Fan on") {
    websocket.send("fan:off");
    val.innerText = "Fan off";
    section.style.backgroundColor = "#395a7f"; // Farbe für "aus"
  } else {
    websocket.send("fan:on");
    val.innerText = "Fan on";
     section.style.backgroundColor = "tomato"; // Farbe für "an"
  }
}


// -------------handle messages ------------------------

function handleAllMessages(msg) {

  // =====================
  // 🔹 AUTO MODE
  // =====================
  if (msg === "autoMode:on") {
    autoCheckbox.checked = true;
  } else if (msg === "autoMode:off") {
    autoCheckbox.checked = false;
  }

  // =====================
  // 🔹 WR RELAY
  // =====================
  else if (msg === "wr:on" || msg === "wr:off") {
    var section = document.getElementById("wr_relay");
    var val = document.getElementById("wr_relaystate");

    if (msg === "wr:on") {
      val.innerHTML = "ON";
      section.style.backgroundColor = "tomato";
    } else {
      val.innerHTML = "OFF";
      section.style.backgroundColor = "#395a7f";  
    }
  }

  // =====================
  // 🔹 PC RELAY
  // =====================
  else if (msg === "pc:on" || msg === "pc:off") {
    var section = document.getElementById("pc_relay");
    var val = document.getElementById("pc_relaystate");

    if (msg === "pc:on") {
      val.innerHTML = "ON";
      section.style.backgroundColor = "tomato";
    } else {
      val.innerHTML = "OFF";
      section.style.backgroundColor = "#395a7f";  
    }
  }

  // =====================
  // 🔹 DISPLAY
  // =====================
  else if (msg === "display:on" || msg === "display:off") {
    var section = document.getElementById("switchdisplay");
    var val = document.getElementById("displaystate");

    if (msg === "display:on") {
      val.innerHTML = "ON";
      section.style.backgroundColor = "tomato"; 
    } else {
      val.innerHTML = "OFF";
      section.style.backgroundColor = "#395a7f"; 
    }
  }

  // =====================
  // 🔹 FAN
  // =====================
else if (msg === "fan:on" || msg === "fan:off") {
  var fanStateDiv = document.getElementById("fanstate");

  if (msg === "fan:on") {
    fanStateDiv.innerText = "Fan on";
  } else {
    fanStateDiv.innerText = "Fan off";
  }
}


// 🔹 TIMER ON
else if (msg.indexOf("on_timer_min:") === 0) {
  var val = msg.split(":")[1];
  var onTimer = document.getElementById("onTimer");
  var startCD = document.getElementById("startCountdown");

  if (val === "off") {
    onTimer.innerText = "Timer off";
    startCD.innerText = "-- min";
    onTimer.style.backgroundColor = "#395a7f";
  } else {
    onTimer.innerText = "Timer run";
    startCD.innerText = val + " min";
    onTimer.style.backgroundColor = "tomato";
  }
}

// 🔹 TIMER OFF
else if (msg.indexOf("off_timer_min:") === 0) {
  var val = msg.split(":")[1];
  var offTimer = document.getElementById("offTimer");
  var stopCD = document.getElementById("stopCountdown");

  if (val === "off") {
    offTimer.innerText = "Timer off";
    stopCD.innerText = "-- min";
    offTimer.style.backgroundColor = "#395a7f";
  } else {
    offTimer.innerText = "Timer run";
    stopCD.innerText = val + " min";
    offTimer.style.backgroundColor = "tomato";
  }
}

  // =====================
  // 🔹 FALLBACK / unbekannte Nachrichten
  // =====================
  else {
    console.log(" Unbekannte Nachricht:", msg);
  }
}

//-----------------------------------------------------------

// Zeitformatierung
function formatRuntimeCompact(seconds) {
  var days = Math.floor(seconds / 86400);
  seconds %= 86400;
  var hours = Math.floor(seconds / 3600);
  seconds %= 3600;
  var minutes = Math.floor(seconds / 60);
  seconds = Math.floor(seconds % 60);

  function pad(num) {
    return (num < 10 ? "0" : "") + num;
  }

  var text = "";
  if (days > 0) {
    text += days + "d ";
  }
  text += pad(hours) + ":" + pad(minutes) + ":" + pad(seconds);
  return text;
}
  
// autobutton checkbox slider
var autoCheckbox = document.getElementById("autoMode");

if (autoCheckbox) {
  autoCheckbox.onclick = function() {
    var mode = autoCheckbox.checked ? "on" : "off";
    setAutoMode(mode);
  };
}

function setAutoMode(mode) {
  if (websocket && websocket.readyState === WebSocket.OPEN) {
    websocket.send("autoMode:" + mode);
  }
}

// reset esp  checkbox slider -------------------------------------------------
// Checkbox holen
var resetCheckbox = document.getElementById("resetesp");

if (resetCheckbox) {
  resetCheckbox.onclick = function() {
    if (resetCheckbox.checked) {
      resetEsp();

      setTimeout(function() {
        resetCheckbox.checked = false;
      }, 1000);
    }

  };
}

function resetEsp() {
  if (websocket && websocket.readyState === WebSocket.OPEN) {
    websocket.send("esp:reset");
  }


//--------------

}

</script>

<!-- ================== SCRIPT LOAD VALUES ================== -->

<script>

//--------function load values -------------------------------------------------
var start_inverter = 0;
var stop_inverter = 0;
var start_delay = 0;
var stop_delay = 0;
var fan_t = 0;

function get_values() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/sendValues", true);

  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4 && xhr.status === 200) {
      var txt = xhr.responseText.replace(/\s/g, "");

      // Manuelles Parsen
      var startInvMatch = txt.match(/"start_inverter":(\d+)/);
      var stopInvMatch = txt.match(/"stop_inverter":(\d+)/);
      var startDelayMatch = txt.match(/"start_delay":(\d+)/);
      var stopDelayMatch = txt.match(/"stop_delay":(\d+)/);
      var fanMatch = txt.match(/"fan_t":(\d+)/);

      if (startInvMatch && stopInvMatch && startDelayMatch && stopDelayMatch && fanMatch) {
        start_inverter = parseInt(startInvMatch[1]);
        stop_inverter = parseInt(stopInvMatch[1]);
        start_delay = parseInt(startDelayMatch[1]);
        stop_delay = parseInt(stopDelayMatch[1]);
        fan_t = parseInt(fanMatch[1]);

        document.getElementById("start_inverter").value = start_inverter;
        document.getElementById("stop_inverter").value = stop_inverter;
        document.getElementById("start_delay").value = start_delay;
        document.getElementById("stop_delay").value = stop_delay;
        document.getElementById("fan_t").value = fan_t;
      }
    }
  };

  xhr.send();
}


 // ------change inveter/ send inverter  ---------------------------------------------

function changeValue(type, delta) {
  if (type === 'start_inverter') {
    start_inverter = Math.min(100, Math.max(1, start_inverter + delta));
    if (stop_inverter >= start_inverter) stop_inverter = start_inverter - 1;
  } else if (type === 'stop_inverter') {
    stop_inverter = Math.min(99, Math.max(1, stop_inverter + delta));
    if (stop_inverter >= start_inverter) stop_inverter = start_inverter - 1;
  } else if (type === 'fan_t') {
    fan_t = Math.min(60, Math.max(20, fan_t + delta)); // Temperaturbereich 20-60
  } else if (type === 'start_delay') {
    start_delay = Math.min(30, Math.max(1, start_delay + delta)); // z.B. 0-300 Sekunden
  } else if (type === 'stop_delay') {
    stop_delay = Math.min(30, Math.max(1, stop_delay + delta)); // z.B. 0-300 Sekunden
  }

  // Felder aktualisieren
  document.getElementById("start_inverter").value = start_inverter;
  document.getElementById("stop_inverter").value = stop_inverter;
  document.getElementById("start_delay").value = start_delay;
  document.getElementById("stop_delay").value = stop_delay;
  document.getElementById("fan_t").value = fan_t;
}


// ---  senden ------------------------------------------

function sendValue(varName, value) {
    var xhr = new XMLHttpRequest(); 
    xhr.open("GET", "/sendback?var=" + varName + "&val=" + value, true);
    xhr.send();
}

function saveAll() {
    var xhr = new XMLHttpRequest(); 
    xhr.open("GET", "/saveAll", true);
    xhr.send();
}

</script>


</body>
</html>


    )rawliteral";


//#define HTML_index_LEN 7320
//----------------------------------------------------------
//--------------------------------------------------


static const char HTML_allvalues[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<!--<html lang="en">  -->
<head>
  <meta charset="UTF-8">
 <title>Victron 100/20</title> 
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>

    body {
      background-color: #121212;
      color: #e0e0e0;
      font-family: Arial, sans-serif;
      padding: 12px;
      font-size: 20px; 

    }
    .container {
      max-width: 640px;
      margin: auto;
      background-color: #1e1e1e;
      padding: 5px;
      border-radius: 8px;
    }
    .section {
      margin-bottom: 10px;
      padding: 6px;
      background-color: #395a7f; 
      border-radius: 5px;

    }
    .label {
      font-weight: bold;
      display: inline-block;
      width: 100px;
    }

    .value {
      float: right;
    }

    .progress-bar {
      background-color: #333;
      border-radius: 4px;
      height: 20px;
      overflow: hidden;
    }
    .progress-fill {
      background-color: #009dd1;
      height: 100%;
      width: 0%;
      text-align: center;
      color: #fff;
      line-height: 20px;
    }
    .alert {
      background-color: #b71c1c;
      color: #fff;
      text-align: center;
      padding: 8px;
      border-radius: 5px;
      display: none;
    }
    .button {
      display: block;
      width: 96%;
      padding: 8px;
      background-color: #1976d2;
      color: #fff;
      text-align: center;
      text-decoration: none;
      border-radius: 5px;
      margin-top: 8px;
    }
    .wifi-symbol {
      display: block;
      margin: 10px auto;
    }
  </style>


</head>
<body>

  <noscript>
    <div class="alert"><strong>JavaScript ist deaktiviert. Bitte aktivieren!</strong></div>
  </noscript>

  <div class="container">

  <div class="section"><span class="label">Device:</span><span class="value" id="deviceName"></span></div>
  <div class="section"><span class="label">Panel:</span><span class="value"><span id="panelvolt"></span> <span id="panelcurrent"></span></span></div>
  <div class="section"><span class="label">Battery:</span><span class="value"><span id="battvolt"></span> <span id="battcurrent"></span></span></div>
  <div class="section"><span class="label">MppT</span><span class="value"><span id="trackermode"></span> /<span id="operationstate"></span></span></div>
  <div class="section"><span class="label">Today:</span><span class="value" id="todaykwh"></span></div>
  <div class="section"><span class="label">Max:</span><span class="value" id="maxtoday"></span></div>
  <div class="section"><span class="label">Total:</span><span class="value" id="totalkwh"></span></div>
  <div class="section"><span class="label">Load:</span><span class="value" id="loadcurrent"></span></div>
  <div class="section"><span class="label">Load out:</span><span class="value" id="loadstate"></span></div>
  <div class="section"><span class="label">Error:</span><span class="value" id="errorMessage"></span></div>

  <h3 style="text-align:left;">ESP-Info</h3>
  <hr style="border-color:#444;">

  <div class="alert" id="vcc_alert"><strong>WARNING ESP VOLTAGE TOO LOW</strong></div>

  <div class="section"><span class="label">WiFi:</span><span class="value" id="wifirssi"></span></div>
  <div class="section"><span class="label">IP:</span><span class="value" id="espip"></span></div>
  <div class="section"><span class="label">VCC:</span><span class="value" id="espvcc"></span></div>
  <div class="section"><span class="label">Flash:</span><span class="value" id="jsonspace"></span></div>
  <div class="section"><span class="label">Heap:</span><span class="value" id="espheap"></span></div>
  <div class="section"><span class="label">Heapfrag:</span><span class="value" id="heapfrag"></span></div>
  <div class="section"><span class="label">Json:</span><span class="value" id="jsonsize"></span></div>
  <div class="section"><span class="label">Vic/Bat</span><span class="value"><span id="temp_Victron"></span> /<span id="temp_Battery"></span> </span></div>
  <div class="section"><span class="label">Runtime:</span><span class="value" id="runtime"></span></div>


<hr style="border-color:#444;">

<!-- buttons BACK AND LIVE JSON -->

<!-- <a href="/" class="button">back</a>   -->

<a href="/livejson" class="button" target="_blank">Live JSON</a>


<!-- -------SCRIPT ALL VALUES -----------------------   -->

<script>
var websocket;

function initWebSocket() {
  var gateway = "ws://" + window.location.hostname + "/ws";
  websocket = new WebSocket(gateway);

  websocket.onopen = function() {
    console.log("WebSocket verbunden");
    setInterval(function() {
      if (websocket.readyState !== WebSocket.CLOSED) {
        //websocket.send("connected_allvalues");
      }
    }, 5000);
  };

  websocket.onmessage = function(event) {
    if (event.data.charAt(0) === '{') {
      var b = JSON.parse(event.data);

if (b.victron) {

    // Victron-Daten ALLVALUES 

  document.getElementById("panelvolt").innerHTML = b.victron.Panel_voltage + " V";
  document.getElementById("panelcurrent").innerHTML = b.victron.Panel_power + " W";
  document.getElementById("battvolt").innerHTML = b.victron.Voltage + " V";
  document.getElementById("battcurrent").innerHTML = parseFloat(b.victron.Battery_current).toFixed(1) + " A";
  document.getElementById("operationstate").innerHTML = b.victron.Operation_state_text;
  document.getElementById("trackermode").innerHTML = b.victron.Tracker_operation_mode_text;
  document.getElementById("totalkwh").innerHTML = b.victron.total_kWh + " kWh";
  document.getElementById("todaykwh").innerHTML = b.victron.today_kWh + " kWh";
  document.getElementById("maxtoday").innerHTML = b.victron.Max_pow_today + " W";
  document.getElementById("loadstate").innerHTML = b.victron.Load_output_state;
  document.getElementById("loadcurrent").innerHTML = b.victron.Load_current + " A";
  document.getElementById("errorMessage").textContent = b.victron.Current_error_text || "No DATA";
} else {
  //  Victron offline Felder neutralisieren
  document.getElementById("panelvolt").innerHTML = "-";
  document.getElementById("panelcurrent").innerHTML = "-";
  document.getElementById("battvolt").innerHTML = "-";
  document.getElementById("battcurrent").innerHTML = "-";
  document.getElementById("operationstate").innerHTML = "-";
  document.getElementById("trackermode").innerHTML = "-";
  document.getElementById("totalkwh").innerHTML = "-";
  document.getElementById("todaykwh").innerHTML = "-";
  document.getElementById("maxtoday").innerHTML = "-";
  document.getElementById("loadstate").innerHTML = "-";
  document.getElementById("loadcurrent").innerHTML = "-";
  document.getElementById("errorMessage").textContent = "No DATA";
}

      // ESP-Daten

      document.getElementById("deviceName").textContent = b.Device_name || b.Device_model;
      document.getElementById("espip").innerHTML = b.IP;
      document.getElementById("wifirssi").innerHTML = b.Wifi_RSSI + " dBm";
      document.getElementById("espvcc").innerHTML = b.ESP_VCC + " V";
      document.getElementById("jsonspace").innerHTML = b.Free_Sketch_Space + " B";
      document.getElementById("espheap").innerHTML = b.Free_Heap + " B";
      document.getElementById("heapfrag").innerHTML = b.HEAP_Fragmentation + " %";
      document.getElementById("jsonsize").innerHTML = b.json_space + " B";
      
      document.getElementById("temp_Victron").innerHTML = (b.Temp_Victron) + " °C";
      document.getElementById("temp_Battery").innerHTML = (b.Temp_Battery) + " °C";
      document.getElementById("runtime").innerHTML = formatRuntimeCompact(b.Runtime);

      // VCC-Warnung
      document.getElementById("vcc_alert").style.display = b.ESP_VCC < 2.6 ? "block" : "none";

  //  } else { // no relay in allvalues
    //  handleRelayMessage(event.data);

    }
  };

  websocket.onerror = function() {
    console.log("WebSocket Fehler");
  };

  websocket.onclose = function() {
    console.log("WebSocket getrennt");
    setTimeout(initWebSocket, 3500);
  };
}

window.onload = function() {
  initWebSocket();
};
</script>



<!-- ================== SCRIPT 2:  TIME ================== -->
<script>

function formatRuntimeCompact(seconds) {
  var days = Math.floor(seconds / 86400);
  seconds %= 86400;
  var hours = Math.floor(seconds / 3600);
  seconds %= 3600;
  var minutes = Math.floor(seconds / 60);
  seconds = Math.floor(seconds % 60);

  // Manuelles Null-Auffüllen (statt padStart)
  function pad(num) {
    return (num < 10 ? "0" : "") + num;
  }

  var text = "";
  if (days > 0) {
    text += days + "d ";
  }
  text += pad(hours) + ":" + pad(minutes) + ":" + pad(seconds);
  return text;
}

</script>

  
</body>
</html>


    )rawliteral";


