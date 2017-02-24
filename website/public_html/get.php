<?php
header('Content-type: application/json');
if(!(isset($_GET['id']))) {
    $response = array('success' => false,
    "error" => array ("code"=> 4,
    "message"=> "No devid")
        
  );    
  echo json_encode($response);
}

require "database.php";

$database = Database::getInstance();
$database->prepare("SELECT distance FROM measurepoint WHERE deviceid=:deviceid ORDER BY id DESC LIMIT 1;");
$database->bindParam(":deviceid", $_GET['id']);
$database->execute();
$data = $database->getAll();
if(count($data) > 0) {
    $distance = intval($data[0]['distance']);
    $response = array('success' => true, 'distance' => $distance);
    echo json_encode($response);
}
