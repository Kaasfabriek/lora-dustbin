$(document).ready(function() {
   // when the document is loaded wait 5 seconds   
   setTimeout(refreshData, 5000);
});
function refreshData() {
	// the for each dustbin update the data
        $(".dustbin").each(function() {
          var $dustbin = $(this);
          var $id = $(this).attr("id");
          
          var $distance = $($dustbin).find(".distance");
          var $IRdistance = $($dustbin).find(".IRdistance");
          var url = "get.php?id=" + $id; 
	  // we have no data so we as the php script of get.php
          $.ajax({
            url: url,
            dataType: "json",
            success:function(data, $dustbin) {
 		// on succes we put the data into the html page and change the image height
                console.log("Success");
                data.distance *= 10; // cm to mm
                $distance.html(data.distance);
                $IRdistance.html(data.IRdistance);
                $("#" + $id).find(".image .overlay").css("height", (data.IRdistance / 10));               
                console.log()
            },
            error: function(data) {
                
            }
        });
        
       });
       // after 5 seconds we update the values again
       setTimeout(refreshData, 5000);
   }
