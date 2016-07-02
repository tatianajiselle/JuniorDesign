 $(document).ready(function() {


var display = function(){

$.get( "/getlog", function(data) {
  $( "#log" ).html(data);

});

}
    
window.setInterval(function(){

     display();
  
}, 2000);




});


