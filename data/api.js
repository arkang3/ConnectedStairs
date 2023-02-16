
function isStairsConfigure(){

  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      console.log(this.responseText);

      const obj = JSON.parse(this.responseText);
      if(obj.isOnline=="0"){
          alert("You need to configure your stairs!");
      }else{
        console.log("stairs config")
      }
    }
  }
  xhr.open("GET", "/getStairsStatus", true); 
  xhr.send();

}

function loadMQTTConfig(){

  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(this.responseText=="KO"){
      }else{
        console.log(this.responseText);
        const obj = JSON.parse(this.responseText);
        document.getElementById("mqtt_server").value = obj.host
        document.getElementById("mqtt_port").value = obj.port
        document.getElementById("mqtt_username").value = obj.username
        document.getElementById("mqtt_password").value = obj.password
      }
      
    }
  }
  xhr.open("GET", "/loadMQTTConfig", true); 
  xhr.send();
}

function onMQTTConf(){
  
  var xhr = new XMLHttpRequest();
  var params = "{\"server\":\""+document.getElementById("mqtt_server").value+"\",\"port\":"+document.getElementById("mqtt_port").value+",\"username\":\""+document.getElementById("mqtt_username").value+"\",\"password\":\""+document.getElementById("mqtt_password").value+"\"}";
  console.log(params)
  xhr.open("GET", "/setMQTTConf?JSON="+params, true); 
  xhr.send();

}

function loadStairsConfig(){

  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(this.responseText=="KO"){
      }else{
        console.log(this.responseText);
        json.value = this.responseText
        enableSave();
      }
      
    }
  }
  xhr.open("GET", "/loadStairsConfig", true); 
  xhr.send();
}

function isJSON(text){
  if (typeof text!=="string"){
      return false;
  }
  try{
      var json = JSON.parse(text);
      return (typeof json === 'object');
  }
  catch (error){
      return false;
  }
}
function onStairsConf(){
  console.log("onStairsConf : ");

  var params = document.getElementById("json").value
  if(isJSON(params)){
    var xhr = new XMLHttpRequest();
    console.log(params)
    xhr.open("GET", "/setStairsConf?JSON="+params, true); 
    xhr.send();
  }else{
    alert("invalid json")
  }
}




function toggleLight(element,color){

    var xhr = new XMLHttpRequest();
    var params;
    if(element.checked){
        console.log("lighton " + color);
        params = "{on:true,color:\""+color+"\"}";
    }else{
        console.log("lightoff ");
        params = "{on:false}";
    }
    
    xhr.open("GET", "/action?JSON="+params, true); 
    xhr.send();

}

function setBrightness(element){
    console.log(element.value)
    var xhr = new XMLHttpRequest();
    var params = "{on:true,bri:"+element.value+"}";
    xhr.open("GET", "/action?JSON="+params, true); 
    xhr.send();
}

function hslToHex(h, s, l) {
    l /= 100;
    const a = s * Math.min(l, 1 - l) / 100;
    const f = n => {
      const k = (n + h / 30) % 12;
      const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
      return Math.round(255 * color).toString(16).padStart(2, '0');   // convert to Hex and prefix "0" if needed
    };
    return `0x${f(0)}${f(8)}${f(4)}`;
  }

function setColor(element){
    
    var hue = ((element.value/100)*360).toFixed(0)
    var color = hslToHex(hue,100,50)
    console.log(color)
    var xhr = new XMLHttpRequest();
    var params = "{on:true,color:\""+color+"\"}";
    xhr.open("GET", "/action?JSON="+params, true); 
    xhr.send();
}






function connectToWifi(){

  let element = document.getElementById("loader");
  element.classList.remove("hide");

  var xhr = new XMLHttpRequest();
  // xhr.open("GET", "/setWifiConf?ssid="+document.getElementById("ssid").value+"&password="+document.getElementById("password").value, true); 
  xhr.open("GET", "/setWifiConf", true);
  xhr.send();
  
}

function scanNetworks(){
  console.log("scanNetworks")
  let element = document.getElementById("loader");
  element.classList.remove("hide");
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var idInterval = setInterval(function() {
      
        var xhttp2 = new XMLHttpRequest();
        xhttp2.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {

            const selector = document.getElementById("ssid");

            while (selector.options.length > 0) {                
              selector.remove(0);
            }    

            console.log(this.responseText);

            const obj = JSON.parse(this.responseText);

            for (var i = 0; i < obj.ssidList.length; i++) {
              const newOption = document.createElement("option");

              var ssidName = obj.ssidList[i];

              newOption.value = ssidName.value;
              newOption.text = ssidName.ssid;
              selector.add(newOption, null);
            }
            
            clearInterval(idInterval);
            let element = document.getElementById("loader");
            element.classList.add("hide");
          }else{
            console.log("not ready")
          }
        };
        xhttp2.open("GET", "/getWifiList", true);
        xhttp2.send();
        
      }, 2000);
    }
  };
  xhttp.open("GET", "/scanNetworks", true);
  xhttp.send();

}