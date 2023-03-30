

function connectToWifi(){

    let element = document.getElementById("loader");
    element.classList.remove("hide");
    console.log("/setWifiConf?ssid="+document.getElementById("ssid").value+"&password="+document.getElementById("password").value);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/setWifiConf?ssid="+document.getElementById("ssid").value+"&password="+document.getElementById("password").value, true); 
    // xhr.open("GET", "/setWifiConf", true);
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