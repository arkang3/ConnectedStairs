const char uploadJson_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/post" method='POST'>
    JSON: <input type="text" name="JSON">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";