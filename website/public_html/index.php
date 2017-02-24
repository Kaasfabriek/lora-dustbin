<?php

/* 
 * show the dustbin data 
 */
require "database.php";
$database = Database::getInstance();
$database->prepare("SELECT DISTINCT (deviceid) FROM measurepoint");
$database->execute();
$dustbins = $database->getAll();

?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Prullenbakken</title>
  <link rel="stylesheet" href="style.css" type="text/css">
  
  </head>
  <body>
  <?php
  foreach($dustbins as $dustbin) {
      ?>
      <div class="dustbin" id="<?=$dustbin['deviceid']?>">
          <div class="title"><b>Dustbin: </b><?=$dustbin['deviceid']?></div>
          <div class="data">
              <?php
              $database->prepare("SELECT distance, IRdistance FROM measurepoint WHERE deviceid=:deviceid ORDER BY id DESC LIMIT 1;");
              $database->bindParam(":deviceid", $dustbin['deviceid']);
              $database->execute();
              $all = $database->getAll()[0];
              $distance = intval($all['distance']);
              $distance *= 10;
              $IRdistance = $all['IRdistance'];
              ?>
              <div class="full"><b>Echo says dustbin still has: </b><span class="distance"><?=$distance?></span> mm of free space<br/>
                  <b>IR says dustbin still has: </b><span class="IRdistance"><?=$IRdistance?></span> mm of free space
              </div>
              <div class="image"><img src="dustbin2.png" />
                  <div class="overlay"><img src="dustbin1.png" /></div>
              </div>
          </div>
      </div>
      <?php
  }
  ?>
  <script src="jquery-3.1.1.min.js" type="text/javascript"></script>
  <script src="refresh_data.js"></script>
  </body>
</html>
