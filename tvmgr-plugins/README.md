-----------------
Build:

bitbake tvsettings-plugins

-----------------
Test:

Initialize TVMgr plugin:

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.tvMgrInit"}' http://127.0.0.1:9998/jsonrpc


Terminate TVMgr plugin:

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.tvMgrTerm"}' http://127.0.0.1:9998/jsonrpc


TVMgr-Plugin GET APIs:


curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getPictureMode"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getBacklight"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getBrightness"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getContrast"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getSaturation"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getSharpness"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getHue"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getColorTemperature"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.getAspectRatio"}' http://127.0.0.1:9998/jsonrpc




TVMgr-Plugin SET APIs:

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setColorTemperature","params":{"colorTemp": "Standard"}}' http://127.0.0.1:9998/jsonrpc                       
<colorTemp> <"Standard","Warm","Cold","User">

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setPictureMode","params":{"pictureMode": "1"}}' http://127.0.0.1:9998/jsonrpc

<pictureMode> <0-11>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setBacklight","params":{"backlight": "1"}}' http://127.0.0.1:9998/jsonrpc

<backlight> <0-100>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setBrightness","params":{"brightness": "70"}}' http://127.0.0.1:9998/jsonrpc

<brightness> <0-100>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setContrast","params":{"contrast": "1"}}' http://127.0.0.1:9998/jsonrpc

<contrast> <0-100>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setSharpness","params":{"sharpness": "1"}}' http://127.0.0.1:9998/jsonrpc

<sharpness> <0-100>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setSaturation","params":{"saturation": "1"}}' http://127.0.0.1:9998/jsonrpc

<saturation> <0-100>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setHue","params":{"hue": "1"}}' http://127.0.0.1:9998/jsonrpc

<hue> <0-100>

curl --header "Content-Type: application/json"   --request POST   --data '{"jsonrpc": "2.0","id": 1234567890,"method": "org.rdk.tv.ControlSettings.1.setAspectRatio","params":{"aspectRatio": "1"}}' http://127.0.0.1:9998/jsonrpc

<sspectRatio> <0-4>
