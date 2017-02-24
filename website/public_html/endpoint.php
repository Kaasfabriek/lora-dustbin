<?php

/* 
 * This file receives the data from TTN. TTN uses HTTP intergration for this
 * 
 * documentation here: https://www.thethingsnetwork.org/docs/applications/http/
 * 
 * key for identification: 2uyerebra5uret7bac5edAFe
 * 
 * {
  "app_id": "my-app-id",              // Same as in the topic
  "dev_id": "my-dev-id",              // Same as in the topic
  "hardware_serial": "0102030405060708", // In case of LoRaWAN: the DevEUI
  "port": 1,                          // LoRaWAN FPort
  "counter": 2,                       // LoRaWAN frame counter
  "is_retry": false,                  // Is set to true if this message is a retry (you could also detect this from the counter)
  "payload_raw": "AQIDBA==",          // Base64 encoded payload: [0x01, 0x02, 0x03, 0x04]
  "payload_fields": {},               // Object containing the results from the payload functions - left out when empty
  "metadata": {
    "time": "1970-01-01T00:00:00Z",   // Time when the server received the message
    "frequency": 868.1,               // Frequency at which the message was sent
    "modulation": "LORA",             // Modulation that was used - LORA or FSK
    "data_rate": "SF7BW125",          // Data rate that was used - if LORA modulation
    "bit_rate": 50000,                // Bit rate that was used - if FSK modulation
    "coding_rate": "4/5",             // Coding rate that was used
    "gateways": [
      {
        "gtw_id": "ttn-herengracht-ams", // EUI of the gateway
        "timestamp": 12345,              // Timestamp when the gateway received the message
        "time": "1970-01-01T00:00:00Z",  // Time when the gateway received the message - left out when gateway does not have synchronized time
        "channel": 0,                    // Channel where the gateway received the message
        "rssi": -25,                     // Signal strength of the received message
        "snr": 5,                        // Signal to noise ratio of the received message
        "rf_chain": 0,                   // RF chain where the gateway received the message
      },
      //...more if received by more gateways...
    ]
  },
  "downlink_url": "https://integrations.thethingsnetwork.org/ttn/api/v2/down/my-app-id/my-process-id?key=ttn-account-v2.secret"
}
 * 
 * for this application to work there must be a payload function in TTN creating the following fields:
 * - distance: integer
 */
file_put_contents("/log.txt", file_get_contents('php://input'));

if(!($_GET['key'] == "2uyerebra5uret7bac5edAFe")) {
    file_put_contents("/log.txt", "Access denied: wrong key");
    $response = array(
        "success" => false,
        "error" => array(
          "code" => 1,
          "message" => "Access denied: wrong key"
        )
    );
    echo $response;
    die();
    
}
$json = json_decode(file_get_contents('php://input'), true);

if(!(isset($json['dev_id']) && isset($json["payload_fields"]) && 
        isset($json["payload_fields"]['distance']) && isset($json["payload_fields"]['IRdistance']))) {

    $response = array(
        "success" => false,
        "error" => array (
          "code" => 2,
          "message" => "Fields are missing - forgot payload function?"
         )
      );
    echo $json_encode($response);
    die();
}
$dustbinid = $json['dev_id'];
$distance = $json["payload_fields"]['distance'];
$IRdistance = $json["payload_fields"]['IRdistance'];

require "database.php";

$database = Database::getInstance();
$database->prepare("INSERT INTO measurepoint (deviceid, distance, IRdistance) VALUES (:dustbinid, :distance, :IRdistance)");
$database->bindParam(":dustbinid", $dustbinid);
$database->bindParam(":distance", $distance);
$database->bindParam(":IRdistance", $IRdistance);
$database->execute();

$response = array(
  "success" => true,  
);
echo json_encode($response);