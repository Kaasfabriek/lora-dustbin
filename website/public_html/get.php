<?php
// return json
header('Content-type: application/json');

// check for id of dustbin
if(!(isset($_GET['id']))) {
    $response = array('success' => false,
    "error" => array ("code"=> 4,
    "message"=> "No devid")
        
  );    
  echo json_encode($response);
}

// get database access
require "database.php";

// Get variables form the database
$database = Database::getInstance();
$database->prepare("SELECT distance, IRdistance FROM measurepoint WHERE deviceid=:deviceid ORDER BY id DESC LIMIT 1;");
$database->bindParam(":deviceid", $_GET['id']);
$database->execute();
$data = $database->getAll();
// if there are rows return the variables in json format for the jQuery ajax script that live updates the data on the website
if(count($data) > 0) {
    $distance = intval($data[0]['distance']);
    $IRdistance = intval($data[0]['IRdistance']);
    $response = array('success' => true, 'distance' => $distance, 'IRdistance' => $IRdistance);
    echo json_encode($response);
}

