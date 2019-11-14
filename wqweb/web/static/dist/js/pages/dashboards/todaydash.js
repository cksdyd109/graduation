$(document).ready(function() {
    
   setInterval(function() {
       $.getJSON('/todayDash',
                function(data) {
           var json = data['todayWater'];
           
           $('#phValue').html('');
           $('#turValue').html('');
           
           $('#phValue').append(json['ph']);
           $('#turValue').append(json['turbidity']);
       });
   }, 1000);
});