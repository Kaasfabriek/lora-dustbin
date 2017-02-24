$(document).ready(function() {
   
   setTimeout(refreshData, 5000);
});
function refreshData() {
        $(".dustbin").each(function() {
          var $dustbin = $(this);
          var $id = $(this).attr("id");
          
          var $distance = $($dustbin).find(".distance");
          var url = "get.php?id=" + $id; 
          $.ajax({
            url: url,
            dataType: "json",
            success:function(data, $dustbin) {
                console.log("Success");
                $distance.html(data.distance);
                $("#" + $id).find(".image .overlay").css("height", (data.distance / 10));               
                console.log()
            },
            error: function(data) {
                
            }
        });
        
       });
       setTimeout(refreshData, 5000);
   }