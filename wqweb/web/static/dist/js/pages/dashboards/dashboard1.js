/*
Template Name: Admin Pro Admin
Author: Wrappixel
Email: niravjoshi87@gmail.com
File: js
*/
$(function() {
    "use strict";

    var temp = $('.sales').attr('a');
    var k = JSON.parse(temp);
    var temp2 = $('.sales').attr('b');
    var k2 = JSON.parse(temp2);
    
    // ============================================================== 
    // sales ratio
    // ============================================================== 
    var chart = new Chartist.Line('.sales', {
        labels: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
        series: [
            [9.54, 9.56, 9.55, 9.6, 9.53, 9.56, 9.55, 9.55, 9.6, 9.53],
            [8.54, 8.56, 8.55, 8.6, 8.53, 8.56, 8.55, 8.55, 8.6, 8.53],
            [4.27, 4.27, 4.27, 4.26, 4.26, 4.26, 4.26, 4.28, 4.27, 4.26],
        ]
    }, {
        low: 4,
        high: 10,
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

    var chart = [chart];

    // ============================================================== 
    // Our Visitor
    // ============================================================== 
    var sparklineLogin = function() {
        $('#earnings').sparkline([6, 10, 9, 11, 9, 10, 12, 10, 9, 11, 9, 10, 12, 10, 9, 11, 9], {
            type: 'bar',
            height: '40',
            barWidth: '4',
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
});