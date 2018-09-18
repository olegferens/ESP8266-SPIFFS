////////////////////Flash Settings////////////////////
//      Board:      NodeMCU 1.0(ESP-12E Module)     //
//      CPU Freq:   160MHz                          //
//      Flash Size: 4M (3M SPIFFS)                  //
////////////////////Flash Settings////////////////////


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"

extern "C" {
  #include "user_interface.h"
}

// Struct for holding JSON configuration
struct Configuration {
  bool connect_to_wifi;
  String apSSID;
  String apPassword;
  String clientSSID;
  String clientPassword;
  String hostname;
  String www_username;
  String www_password;
};

ESP8266WebServer server(80); // Initialize web server

// Setup global vars
int esp12led = 2;
int nodemculed = 16;
struct Configuration global_conf;
String domain_name_prefix = "rota";
String permitted_domain_characters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"};



// CSS Colors
String color1 = "#c10000"; // Accent
String color2 = "#000000"; // Background
String color3 = "#2f3136"; // Bar
String color4 = "#bfbfbb"; // Text
String color5 = "#ffe500"; // Important Bar



// Generate CSS style on the fly to save variable space
String GenPageStyleCSS()
{
  String page_style_css = "<title>I have been hack</title>\n"
                    "<font face=\"verdana\">\n"
                    "<style type=\"text/css\">\n"
                    "    #submit {\n"
                    "        background-color: " + color1 + ";\n"
                    "        padding: .5em;\n"
                    "        -moz-border-radius: 5px;\n"
                    "        -webkit-border-radius: 5px;\n"
                    "        border-radius: 6px;\n"
                    "        color: " + color2 + ";\n"
                    "        font-family: 'verdana';\n"
                    "        font-size: 20px;\n"
                    "        text-decoration: none;\n"
                    "        border: none;\n"
                    "    }\n"
                    "    #submit:hover {\n"
                    "        border: none;\n"
                    "        background: cyan;\n"
                    "        box-shadow: 0px 0px 1px #777;\n"
                    "  color: black;\n"
                    "    }\n"
                    "    body\n"
                    "    {\n"
                    "        color: " + color1 + ";\n"
                    "        background-color: " + color2 + ";\n"
                    "    }\n"
                    "    .topnav {\n"
                    "        background-color: " + color3 + ";\n"
                    "        border-left: solid " + color1 + " 5px;\n"
                    "        border-radius: 3px;\n"
                    "        overflow: hidden;\n"
                    "    }\n"
                    "    .topnav a {\n"
                    "        float: left;\n"
                    "        color: " + color4 + ";\n"
                    "        text-align: center;\n"
                    "        padding: 4px 10px;\n"
                    "        text-decoration: none;\n"
                    "        font-size: 17px;\n"
                    "        border-radius: 3px;\n"
                    "        margin-top: 0.5rem;\n"
                    "        margin-left: 0.5rem;\n"
                    "        margin-right: 0.5rem;\n"
                    "        margin-bottom: 0.5rem;\n"
                    "    }\n"
                    "    .topnav a:hover {\n"
                    "        background-color: " + color1 + ";\n"
                    "        color: black;\n"
                    "    }\n"
                    "    .topnav-right {\n"
                    "        float: right;\n"
                    "    }\n"
                    "    h1 {\n"
                    "        font-size: 1.7rem;\n"
                    "        margin-top: 1rem;\n"
                    "        margin-left: auto;\n"
                    "        margin-right: auto;\n"
                    "        background: " + color3 + ";\n"
                    "        color: " + color4 + ";\n"
                    "        padding: 0.2em 1em;\n"
                    "        border-radius: 3px;\n"
                    "        border-left: solid " + color1 + " 5px;\n"
                    "        font-weight: 100;\n"
                    "    }\n"
                    "    h2 {\n"
                    "        font-size: 1rem;\n"
                    "        margin-top: 1rem;\n"
                    "        margin-left: auto;\n"
                    "        margin-right: auto;\n"
                    "        background: " + color3 + ";\n"
                    "        color: " + color4 + ";\n"
                    "        padding: 0.2em 1em;\n"
                    "        border-radius: 3px;\n"
                    "        border-left: solid " + color1 + " 5px;\n"
                    "        font-weight: 100;\n"
                    "    }\n"
                    "    h3 {\n"
                    "        font-size: 1rem;\n"
                    "        margin-top: 1rem;\n"
                    "        margin-left: auto;\n"
                    "        margin-right: auto;\n"
                    "        background: " + color5 + ";\n"
                    "        color: " + color2 + ";\n"
                    "        padding: 0.2em 1em;\n"
                    "        border-radius: 3px;\n"
                    "        font-weight: 100;\n"
                    "    }\n"
                    "    .column {\n"
                    "        margin-left: 10rem;\n"
                    "        margin-right: 10rem;\n"
                    "    }\n"
                    "    .column input {\n"
                    "        float: right;\n"
                    "        margin-top: 0.5rem;\n"
                    "        background: transparent;\n"
                    "        color: " + color4 + ";\n"
                    "        outline: 0;\n"
                    "        border: 0;\n"
                    "        border-bottom: solid " + color1 + " 2px;\n"
                    "        font-size: 14px;\n"
                    "    }\n"
                    "    .column input {\n"
                    "        clear: both;\n"
                    "    }\n"
                    "    .column span {\n"
                    "        margin-top: 0.5rem;\n"
                    "        display: inline-block;\n"
                    "    }\n"
                    "    .column about {\n"
                    "      font-size: +12px;\n"
                    "    }\n"
                    "\n"    
                    "    .prefix {\n"
                    "      display: flex;\n"
                    "        justify-content: space-between;\n"
                    "    }\n"
                    "\n"    
                    "    hr {\n"
                    "      border-color: " + color3 + ";\n"
                    "        background-color: " + color3 + ";\n"
                    "        height: 2px;\n"
                    "        border: none;\n"
                    "    }\n"
                    "</style>\n";
  return page_style_css;
}

// Html string that holds the menu
String menu_html =  "<div class=\"topnav\">\n"
                    "  <a href=\"/\">Home</a>\n"
                    "  <a href=\"settings\">Settings</a>\n"
                    "  <a href=\"api/settings\">API</a>\n"
                    "  <div class=\"topnav-right\">\n"
                    "    <a href=\"about\">About</a>\n"
                    "  </div>\n"
                    "</div>\n";

// Reset html page
String reset_html = GenPageStyleCSS() +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"restart\">Restart</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>Module restarting...</span></center>";

                    
                    
// About html page
String about_html = GenPageStyleCSS() +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"about\">About</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>About</span></center>";


// Main index html page for web server
String index_html = GenPageStyleCSS() +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"home\">Home</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>I made a thing do some stuff</span></center>";

String failed_authentication_html = GenPageStyleCSS() +
                    menu_html + 
                    "<h1 class=\"header\" data-translate=\"home\">Failed Authentication</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>You have not authenticated with this page. Please visit Home.</span></center>";
                    
                    






// Settings html page for web server
// Function to generate settings page based on current configuration
// No args
// Return 1 String
String GenSettingsHTML()
{
  String settings_html = GenPageStyleCSS();
  settings_html = settings_html + menu_html + "<h1 class=\"header\" data-translate=\"settings\">Settings</h1>\n"
                    "<html><body>\n"
                    "    <div class=\"column\">\n"
                    "    <form action=\"/restart_esp8266\">\n"
                    "        <input type=\"submit\" id=\"submit\" value=\"Restart\"/>\n"
                    "    </form>\n"
                    "    <br><br><br>\n"
                    "    <form name='frm' method='get'>\n"
                    "            <h2 class=\"header\" data-translate=\"settings\">Admin Settings</h2>\n"
                    "            <about/>Use these settings when logging in to this web page.</about>\n"
                    "            <hr>\n"
                    "            <br>\n"
                    "            <span style='font-size: +14px'/>Connect to WiFi</span>\n";

  if (global_conf.connect_to_wifi)
    settings_html = settings_html + "            <input type='checkbox' name='connect_to_wifi' checked><br>\n";
  else
    settings_html = settings_html + "            <input type='checkbox' name='connect_to_wifi'><br>\n";
    
  settings_html = settings_html + ""
                    "            <span style=\"font-size: +14px\"/>Admin Username</span>\n"
                    "            <input type='text' name='www_username' value='" + global_conf.www_username + "' placeholder=\"Username\"><br>\n"
                    "            <span style=\"font-size: +14px\"/>Admin Password</span>\n"
                    "            <input type='password' name='www_password' value='" + global_conf.www_password + "' placeholder=\"Password\"><br>\n"
                    "            <br><br>\n"
                    "            <h2 class=\"header\" data-translate=\"settings\">WiFi Connection Settings</h2>\n"
                    "            <about/>Use these settings to connect this device to a WiFi Network in your area.</about>\n"
                    "            <hr>\n"
                    "            <br>\n"
                    "            <span style=\"font-size: +14px\"/>WiFi Connection SSID</span>\n"
                    "            <input type='text' name='new_target_ssid' value='" + global_conf.clientSSID + "' placeholder=\"SSID\"><br>\n"
                    "            <span style=\"font-size: +14px\"/>WiFi Connection Password</span>\n"
                    "            <input type='password' name='new_target_password' value='" + global_conf.clientPassword + "' placeholder=\"Password\"><br>\n"
                    "            <br><br>\n"
                    "            <h2 class=\"header\" data-translate=\"settings\">Access Point Settings</h2>\n"
                    "            <about/>Use these settings to configure this device's access point. These settings will be used by other wireless clients when they connect to this device.</about>\n"
                    "            <hr>\n"
                    "            <br>\n"
                    "            <span style=\"font-size: +14px\"/>Access Point SSID</span>\n"
                    "            <input type='text' name='new_ssid' value='" + global_conf.apSSID + "' placeholder=\"SSID\"><br>\n"
                    "            <span style=\"font-size: +14px\"/>Access Point Password</span>\n"
                    "            <input type='password' name='new_password' value='" + global_conf.apPassword + "' placeholder=\"Password\"><br>\n"
                    "            <br><br>\n"
                    "            <h2 class=\"header\" data-translate=\"settings\">mDNS Settings</h2>\n"
                    "            <about/>The mDNS settings are used to ensure other clients on the same network can access this device using a domain name rather than an IP address.</about>\n"
                    "            <br>\n"
                    "            <about/><b>Note:</b> The domain name will have a default prefix of \"" + domain_name_prefix + "\".</about>\n"
                    "            <hr>\n"
                    "            <br>\n"
                    "            <div class='prefix'>\n"
                    "                <span style='font-size: +14px'/>mDNS Domain Name</span>\n"
                    "                <div style='text-align: right'>\n"
                    "                    <span style='font-size: +14px'/>" + domain_name_prefix + "-</span>\n"
                    "                    <input type='text' name='new_hostname' value='" + global_conf.hostname + "' placeholder='Hostname'>\n"
                    "                </div>\n"
                    "            </div>\n"
                    "            <h3 class='header' data-translate='settings'><spon style='font-size: +14px'>URL</spon><spon style='float: right;font-size: +14px'>http://" + ConvertHostname() + ".local</spon></h3>\n"
                    "            <br><br>\n"
                    "            <input type='submit' id='submit' value='Apply'>\n"
                    "      </form>\n"
                    "    </div>\n"
                    "</body></html>\n";

  return settings_html;
}






// Function to convert the hostname to a domain name
// Compare each charater to the list of acceptable characters
// If it is not in the list, it will not be in the domain name
// No Args
// Return domain name string
String ConvertHostname()
{
  String domain_name;
  String comp_char;
  bool character_match;

  // For loop to iterate through the hostname
  for (int i = 0; i < global_conf.hostname.length(); i++)
  {
    character_match = false;

    // For loop to iterate through the list of acceptable characters
    for (int x = 0; x < 37; x++)
    {
      comp_char = (String)global_conf.hostname.charAt(i);
      comp_char.toLowerCase();
      if (comp_char == permitted_domain_characters[x])
      {
        character_match = true;
        break;
      }
      else if ((String)global_conf.hostname.charAt(i) == " ")
      {
        domain_name = domain_name + "-";
        break;
      }
    }
    if (character_match == true)
      domain_name = domain_name + comp_char;
  }

  domain_name = domain_name_prefix + "-" + domain_name;

  Serial.print("mDNS Domain Name: ");
  Serial.println(domain_name);
  
  return domain_name;
}




// Function to tell server how to behave for each address
// No args
// No Return
void SetServerBehavior()
{
  if (!MDNS.begin((const char*)ConvertHostname().c_str()))
  {
    Serial.println("Could not configure mDNS");
    return;
  }
  
  //server.on("/", HandleClient);

  // Check for authentication on all pages
  server.on("/", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    HandleClient();
  });
  server.on("/restart_esp8266", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    RestartESP();
  });
  server.on("/settings", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    SettingsESP();
  });
  server.on("/about", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    AboutESP();
  });
  server.on("/api/settings", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    APIESP();
  });
  server.on("/api/set", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    ProcessJSONPost();
  });

  //server.on("/restart_esp8266", RestartESP);
  //server.on("/settings", SettingsESP);
  //server.on("/about", AboutESP);
  //server.on("/api/settings", APIESP);
  //server.on("/api/set", ProcessJSONPost);
  
  server.begin();

  MDNS.addService("http", "tcp", 80);
}





// Function to start an AP if it cant connect to one
// SSID and Password Args
// Return start AP bool
bool startAP(const char* apssid, const char* password)
{
  WiFi.hostname(global_conf.hostname);
  Serial.println("Configuring Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP((const char*)apssid, (const char*)password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  SetServerBehavior();

  return true;
}





// Function to join wifi after config load
// SSID and Password Args
// Return join wifi bool
bool joinWiFi(const char* ssid, const char* password)
{
  WiFi.hostname(global_conf.hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin((const char*)ssid, (const char*)password);

  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the WiFi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if (i >= 10)
      return false;
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  SetServerBehavior();
  
  return true;
}





// Function to load the JSON file from SPIFFS into a config struct
// No args
// Returns config struct
struct Configuration loadConfig() {
  struct Configuration conf;
  
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return conf;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return conf;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return conf;
  }

  // Load JSON into local variables
  bool connect_to_wifi = json["connect_to_wifi"];
  const char* apSSID = json["apSSID"];
  const char* apPassword = json["apPassword"];
  const char* clientSSID = json["clientSSID"];
  const char* clientPassword = json["clientPassword"];
  const char* hostname = json["hostname"];
  const char* www_username = json["www_username"];
  const char* www_password = json["www_password"];

  // Load local variables into struct
  conf.connect_to_wifi = connect_to_wifi;
  conf.apSSID = apSSID;
  conf.apPassword = apPassword;
  conf.clientSSID = clientSSID;
  conf.clientPassword = clientPassword;
  conf.hostname = hostname;
  conf.www_username = www_username;
  conf.www_password = www_password;

  // Output struct
  Serial.print("Connect to WiFi?: ");
  Serial.println(conf.connect_to_wifi);
  Serial.print("Loaded apSSID: ");
  Serial.println(conf.apSSID);
  Serial.print("Loaded apPassword: ");
  Serial.println(conf.apPassword);
  Serial.print("Loaded clientSSID: ");
  Serial.println(conf.clientSSID);
  Serial.print("Loaded clientPassword: ");
  Serial.println(conf.clientPassword);
  Serial.print("Loaded hostname: ");
  Serial.println(conf.hostname);
  Serial.print("Loaded username: ");
  Serial.println(conf.www_username);
  Serial.print("Loaded password: ");
  Serial.println(conf.www_password);
  
  return conf;
}





// Function to save the configuration settings to a JSON file in SPIFFS
// No Args
// Return save bool
bool saveConfig() {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["connect_to_wifi"] = global_conf.connect_to_wifi;
  json["apSSID"] = global_conf.apSSID;
  json["apPassword"] = global_conf.apPassword;
  json["clientSSID"] = global_conf.clientSSID;
  json["clientPassword"] = global_conf.clientPassword;
  json["hostname"] = global_conf.hostname;
  json["www_username"] = global_conf.www_username;
  json["www_password"] = global_conf.www_password;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);

  Serial.println("Saved new configuration");
  
  return true;
}





// Function to handle client input when at /settings
// No Args
// No Return
void SettingsESP()
{
  //if (authenticated)
  //{
  if (server.args() > 0)
  {
    Serial.println("Server arguments received");
    
    global_conf.connect_to_wifi = false;
    
    for (uint8_t i = 0; i < server.args(); i++)
    {
      //Serial.println(server.argName(i));

      // BRANCH STATEMENTS TO CHANGE CONFIGURATION
      if (server.argName(i) == "connect_to_wifi" && server.arg(i).length() > 0)
      {
        Serial.print("Connect to WiFi?: ");
        Serial.println(server.arg(i));
        global_conf.connect_to_wifi = true;
      }
      if (server.argName(i) == "new_target_ssid" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point to connect to: ");
        Serial.println(server.arg(i));
        global_conf.clientSSID = server.arg(i);
      }
      else if (server.argName(i) == "new_target_password" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point Password to connect with: ");
        Serial.println(server.arg(i));
        global_conf.clientPassword = server.arg(i);
      }
      else if (server.argName(i) == "new_ssid" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point to create: ");
        Serial.println(server.arg(i));
        global_conf.apSSID = server.arg(i);
      }
      else if (server.argName(i) == "new_password" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point password to create with: ");
        Serial.println(server.arg(i));
        global_conf.apPassword = server.arg(i);
      }
      else if (server.argName(i) == "new_hostname" && server.arg(i).length() > 0)
      {
        Serial.print("New Hostname: ");
        Serial.println(server.arg(i));
        global_conf.hostname = server.arg(i);
      }
      else if (server.argName(i) == "www_username" && server.arg(i).length() > 0)
      {
        Serial.print("New Username: ");
        Serial.println(server.arg(i));
        global_conf.www_username = server.arg(i);
      }
      else if (server.argName(i) == "www_password" && server.arg(i).length() > 0)
      {
        Serial.print("New Password: ");
        Serial.println(server.arg(i));
        global_conf.www_password = server.arg(i);
      }
    }
    saveConfig();
  }
  // Show response AFTER setting new config
  String settings_html = GenSettingsHTML();
  delay(1000);
  server.send(200, "text/html", settings_html);
  Serial.println(settings_html);

  Serial.println("-----------------------------------------");
  //}
  //else
  //  server.send(200, "text/html", failed_authentication_html);
}





// Function to convert shit to *
// Takes 1 String arg
// Returns 1 String
String HideString(String plain)
{
  String hidden = "";
  
  for (int i = 0; i < plain.length(); i++)
    hidden = hidden + "*";

  return hidden;
}




// Function to handle client at /api/settings
// Displays settings in json format
// No Args
// No Return
void APIESP()
{ 
  //if (authenticated)
  //{
  String json_string = "{\n";
  if (global_conf.connect_to_wifi)
    json_string = json_string + "    \"connect_to_wifi\": true,\n";
  else
    json_string = json_string + "    \"connect_to_wifi\": false,\n";


  json_string = json_string + "    \"apSSID\": \"" + global_conf.apSSID + "\","
                "    \"clientSSID\": \"" + global_conf.clientSSID + "\","
                "    \"hostname\": \"" + global_conf.hostname + "\","
                "    \"www_username\": \"" + global_conf.www_username + "\""
                "}";
  server.send(200, "application/json", json_string);
  //}
  //else
  //  server.send(200, "text/html", failed_authentication_html);
}



// Function to handle JSON API post data
// No Args
// No Return
void ProcessJSONPost()
{
  //if (authenticated)
  //{
  server.send(200, "application/json", "{\"success\": true}");
  if (server.args() > 0)
  {
    Serial.println("Server arguments received");

    global_conf.connect_to_wifi = false;
        
    for (uint8_t i = 0; i < server.args(); i++)
    {
      StaticJsonBuffer<300> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(server.arg(i));
      
      // Load JSON into local variables
      bool connect_to_wifi = json["connect_to_wifi"];
      const char* apSSID = json["apSSID"];
      const char* apPassword = json["apPassword"];
      const char* clientSSID = json["clientSSID"];
      const char* clientPassword = json["clientPassword"];
      const char* hostname = json["hostname"];
      const char* www_username = json["www_username"];
      const char* www_password = json["www_password"];
    
      Serial.println(server.argName(i));

      // When JSON API is used
      if (server.argName(i) == "json_post" && server.arg(i).length() > 0)
      {
        Serial.print("JSON API Post: ");
        Serial.println(server.arg(i));
      
        // Load local variables into struct and output
        if (json.containsKey("connect_to_wifi"))
        {
          global_conf.connect_to_wifi = connect_to_wifi;
          Serial.print("Connect to WiFi?: ");
          Serial.println(global_conf.connect_to_wifi);
        }
        if (json.containsKey("apSSID"))
        {
          global_conf.apSSID = apSSID;
          Serial.print("Loaded apSSID: ");
          Serial.println(global_conf.apSSID);
        }
        if (json.containsKey("apPassword"))
        {
          global_conf.apPassword = apPassword;
          Serial.print("Loaded apPassword: ");
          Serial.println(global_conf.apPassword);
        }
        if (json.containsKey("clientSSID"))
        {
          global_conf.clientSSID = clientSSID;
          Serial.print("Loaded clientSSID: ");
          Serial.println(global_conf.clientSSID);
        }
        if (json.containsKey("clientPassword"))
        {
          global_conf.clientPassword = clientPassword;
          Serial.print("Loaded clientPassword: ");
          Serial.println(global_conf.clientPassword);
        }
        if (json.containsKey("hostname"))
        {
          global_conf.hostname = hostname;
          Serial.print("Loaded hostname: ");
          Serial.println(global_conf.hostname);
        }
        if (json.containsKey("www_username"))
        {
          global_conf.www_username = www_username;
          Serial.print("Loaded www_username: ");
          Serial.println(global_conf.www_username);
        }
        if (json.containsKey("www_password"))
        {
          global_conf.www_password = www_password;
          Serial.print("Loaded www_password: ");
          Serial.println(global_conf.www_password);
        }
      
      
        // Save the new config
        saveConfig();
      }
    }
  }
  //}
  //else
  //  server.send(200, "text/html", failed_authentication_html);
}





// Function to handle client at /reset_esp8266
// No Args
// No Return
void RestartESP()
{
  //if (authenticated)
  //{
  server.send(200, "text/html", reset_html);
  delay(5000);
  ESP.reset();
  //}
  //else
  //  server.send(200, "text/html", failed_authentication_html);
}





// Function to handle client at /
// No Args
// No Return
void HandleClient()
{
  server.send(200, "text/html", index_html);
}





// Function to handle client at /about
// No Args
// No Return
void AboutESP()
{
  //if (authenticated)
  server.send(200, "text/html", about_html);
  //else
  //  server.send(200, "text/html", failed_authentication_html);
}






void setup() {
  pinMode(esp12led, OUTPUT);
  //pinMode(nodemculed, OUTPUT);

  Serial.begin(115200);
  Serial.println("------------------------------------");
  delay(1000);
  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  global_conf = loadConfig();

  // Load config and try to join WiFi or create AP
  if (global_conf.clientSSID.length() == 0 || global_conf.clientPassword.length() == 0)
  {
    Serial.println("Could not load configuration");
  }
  else
  {
    Serial.println("Config Loaded");
    if (!global_conf.connect_to_wifi || !joinWiFi((const char*)global_conf.clientSSID.c_str(), (const char*)global_conf.clientPassword.c_str()))
    {
      Serial.print("\nUnable to join ");
      Serial.println(global_conf.clientSSID);
      if (startAP((const char*)global_conf.apSSID.c_str(), (const char*)global_conf.apPassword.c_str()))
      {
        Serial.print("Access Point configured: ");
        Serial.println(global_conf.apSSID);
      }
    }
  }

  Serial.print("http://");
  Serial.println(WiFi.localIP());
}





void loop() {
  server.handleClient();

  // Let the user know we are in main loop
  digitalWrite(esp12led, LOW);
  delay(500);
  digitalWrite(esp12led, HIGH);
  delay(500);  
}
