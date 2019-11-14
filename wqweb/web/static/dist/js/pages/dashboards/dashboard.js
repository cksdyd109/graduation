$(document).ready(function() {
    
   var syncTime = $('.icanspeak').attr('syncTime');
   syncTime = parseInt(syncTime) * 1000;
    
   setInterval(function() {
       $.getJSON('/todayDash',
                function(data) {
           var json = data['todayWater'];
           var graphlist = data['realgraph'];
          var chart = new Chartist.Line('.icanspeak', {
//              labels: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
              series: [
                  graphlist['phline'],
                  graphlist['turline'],
              ]
          }, {
        low: 0,
        high: 10,
        showArea: true,
        fullWidth: true,
        plugins: [
            Chartist.plugins.tooltip()
        ],
        axisY: {
            onlyInteger: true,
            scaleMinSpace: 15,
            offset: 3,
            labelInterpolationFnc: function(value) {
                return value;
            }
        },

    });
    // temperature
    var chart2 = new Chartist.Line('.temp', {
//              labels: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
              series: [
                  graphlist['temline']
              ]
          }, {
        low: 0,
        high: 40,
        showArea: true,
        fullWidth: true,
        plugins: [
            Chartist.plugins.tooltip()
        ],
        axisY: {
            onlyInteger: true,
            scaleMinSpace: 20,
            offset: 5,
            labelInterpolationFnc: function(value) {
                return value;
            }
        },

    });

    var chart3 = new Chartist.Line('.do', {
//              labels: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
              series: [
                  graphlist['doline']
              ]
          }, {
        low: 0,
        high: 20,
        showArea: true,
        fullWidth: true,
        plugins: [
            Chartist.plugins.tooltip()
        ],
        axisY: {
            onlyInteger: true,
            scaleMinSpace: 20,
            offset: 5,
            labelInterpolationFnc: function(value) {
                return value;
            }
        },

    });

    // weekly
    var sparklineLogin = function() {
        $('#earnings').sparkline(graphlist['weekph'], {
            type: 'bar',
            height: '30',
            barWidth: '10',
            width: '100%',
            resize: true,
            barSpacing: '8',
            barColor: '#137eff'
        });
    };
    var sparkResize;

    $(window).resize(function(e) {
        clearTimeout(sparkResize);
        sparkResize = setTimeout(sparklineLogin, 500);
    });
    sparklineLogin();

    var sparklineLogin2 = function() {
        $('#temings').sparkline(graphlist['weektem'], {
            type: 'bar',
            height: '30',
            barWidth: '10',
            width: '100%',
            resize: true,
            barSpacing: '8',
            barColor: '#137eff'
        });
    };
    var sparkResize2;

    $(window).resize(function(e) {
        clearTimeout(sparkResize);
        sparkResize2 = setTimeout(sparklineLogin2, 500);
    });
    sparklineLogin2();
          
           
           $('#phValue').html('');
           $('#turValue').html('');
           $('#temValue').html('');
           $('#doValue').html('');
           $('#waterstatus').html('');
           
           $('#phValue').append(json['ph']);
           $('#turValue').append(json['turbidity']);
           $('#temValue').append(json['temperature']);
           $('#doValue').append(json['do']);

           if (json['turbidity'] >= 4.3)
           {
                if (json['ph'] >= 6.5 && json['ph'] <= 8.5)
                {
                    if (json['do'] >= 7.5)
                    {
                        $('#waterstatus').append('<h1><i class="fas fa-laugh" style="color:#66ff66"></i></h1>');
                    }
                    else if (json['do'] >= 5.0)
                    {
                        $('#waterstatus').append('<h1><i class="fas fa-smile" style="color:#ffff1a"></i></h1>');
                    }
                    else if (json['do'] >= 2.0)
                    {
                        $('#waterstatus').append('<h1><i class="fas fa-meh" style="color:#ff9900"></i></h1>');
                    }
                    else if (json['do'] < 2.0)
                    {
                        $('#waterstatus').append('<h1><i class="fas fa-frown" style="color:#ff3300"></i></h1>');
                    }
                }
                else
                {
                    $('#waterstatus').append('<h1><i class="fas fa-angry" style="color:#990000"></i></h1>');
                }
           }
           else
           {
            $('#waterstatus').append('<h1><i class="fas fa-angry" style="color:#990000"></i></h1>');
           }
       });
   }, syncTime);
});

$(".customcus-select").each(function() {
  var classes = $(this).attr("class"),
      id      = $(this).attr("id"),
      name    = $(this).attr("name");
  var template =  '<div class="' + classes + '">';
      template += '<span class="customcus-select-trigger">' + $(this).attr("placeholder") + '</span>';
      template += '<div class="customcus-options">';
      $(this).find("option").each(function() {
        template += '<span class="customcus-option ' + $(this).attr("class") + '" data-value="' + $(this).attr("value") + '">' + $(this).html() + '</span>';
      });
  template += '</div></div>';

  $(this).wrap('<div class="customcus-select-wrapper"></div>');
  $(this).hide();
  $(this).after(template);
});
$(".customcus-option:first-of-type").hover(function() {
  $(this).parents(".customcus-options").addClass("option-hover");
}, function() {
  $(this).parents(".customcus-options").removeClass("option-hover");
});
$(".customcus-select-trigger").on("click", function() {
  $('html').one('click',function() {
    $(".customcus-select").removeClass("opened");
  });
  $(this).parents(".customcus-select").toggleClass("opened");
  event.stopPropagation();
});
$(".customcus-option").on("click", function() {
  $(this).parents(".customcus-select-wrapper").find("select").val($(this).data("value"));
  $(this).parents(".customcus-options").find(".customcus-option").removeClass("selection");
  $(this).addClass("selection");
  $(this).parents(".customcus-select").removeClass("opened");
  $(this).parents(".customcus-select").find(".customcus-select-trigger").text($(this).text());
});