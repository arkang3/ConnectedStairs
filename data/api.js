function toggleLight(element,color){

  var xhr = new XMLHttpRequest();
  var params;
  if(element.checked){
      console.log("lighton " + color);
      params = "{on:true}";
  }else{
      console.log("lightoff ");
      params = "{on:false}";
  }
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      
    }
  }
  xhr.open("GET", "/action?JSON="+params, true); 
  xhr.send();

}

function setBrightness(element){
  console.log(element.value)
  var xhr = new XMLHttpRequest();
  var params = "{on:true,bri:"+element.value+"}";
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      
    }
  }
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
  
  var hue = ((element.value/1000)*360).toFixed(0)
  var color = hslToHex(hue,100,50)
  console.log(color)
  var xhr = new XMLHttpRequest();
  var params = "{on:true,color:\""+color+"\"}";
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      
    }
  }
  xhr.open("GET", "/action?JSON="+params, true); 
  xhr.send();
}






function isStairsConfigure(){

  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      console.log(this.responseText);

      const obj = JSON.parse(this.responseText);
      if(obj.status=="1"){
          // alert("You need to configure your stairs!");
          document.getElementById("lightButton").checked =true;
      }
    }
  }
  xhr.open("GET", "/getStairsStatus", true); 
  xhr.send();
}

function getMQTTConfig(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(this.responseText=="KO"){
      }else{
        console.log(this.responseText);
        const obj = JSON.parse(this.responseText);
        document.getElementById("mqtt_server").value = obj.server
        document.getElementById("mqtt_port").value = obj.port
        document.getElementById("mqtt_username").value = obj.username
        document.getElementById("mqtt_password").value = obj.password
      }
      
    }
  }
  xhr.open("GET", "/getMQTTConfig", true); 
  xhr.send();
}

function setMQTTConf(){
  var xhr = new XMLHttpRequest();
  var params = "{\"server\":\""+document.getElementById("mqtt_server").value+"\",\"port\":"+document.getElementById("mqtt_port").value+",\"username\":\""+document.getElementById("mqtt_username").value+"\",\"password\":\""+document.getElementById("mqtt_password").value+"\"}";
  console.log(params)
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(this.responseText=="KO"){
        alert("error in config")
      }else{
        alert("new config load")
      }
    }
  }
  xhr.open("GET", "/setMQTTConf?JSON="+params, true); 
  xhr.send();
}


function getStairsConfig(){
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
  xhr.open("GET", "/getStairsConfig", true); 
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
function setStairsConf(){
  console.log("onStairsConf : ");

  var params = document.getElementById("json").value
  if(isJSON(params)){
    var xhr = new XMLHttpRequest();
    console.log(params)
    xhr.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        if(this.responseText=="KO"){
          alert("error in config")
        }else{
          alert("new config load")
        }
      }
    }
    xhr.open("GET", "/setStairsConf?JSON="+params, true); 
    xhr.send();
  }else{
    alert("invalid json")
  }
}









