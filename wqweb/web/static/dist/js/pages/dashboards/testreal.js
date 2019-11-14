$(document).ready(function() {
    
    setInterval(function() {
        $.getJSON('/realTime',
                    function (data) {
                        var json = data['test'];
                        var chart = new Chartist.Line('.icanspeak', {
                            labels: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
                            series: [json]
                        }, {
                            low: 4,
                            high: 8,
                            showArea: true,
                            fullWidth: true,
                            plugins: [
                                Chartist.plugins.tooltip()
                            ],
                            axisY: {
                                onlyInteger: true,
                                scaleMinSpace: 15,
                                offset: 5,
                                labelInterpolationFnc: function(value) {
                                    return value;
                                }
                            },
                        });
//                        var tr;
//                        $('#icanspeak').html("");
//                        tr = $('<h1/>');
//                        tr.append(json);
//                        $('#icanspeak').append(tr);
            
//                        var tr;
//                        var test;
//                         $('#testtest').html("");
//            
//                        tr = $('<p/>');
//                        tr.append('<img src="data:Image/png;base64,' + json[0] + '">');
//            
//                        test = $('<h1/>');
//                        test.append(json.length);
//                        $('#testtest').append(test);
//                        $('#testtest').append(tr);
                    });
    }, 2000);
});