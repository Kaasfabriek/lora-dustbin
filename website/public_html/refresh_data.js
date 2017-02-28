$(document).ready(function() {
   
   setTimeout(refreshData, 5000);
});
function refreshData() {
        $(".dustbin").each(function() {
          var $dustbin = $(this);
          var $id = $(this).attr("id");
          
          var $distance = $($dustbin).find(".distance");
          var $IRdistance = $($dustbin).find(".IRdistance");
          var url = "get.php?id=" + $id; 
          $.ajax({
            url: url,
            dataType: "json",
            success:function(data, $dustbin) {
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
       setTimeout(refreshData, 5000);
   }