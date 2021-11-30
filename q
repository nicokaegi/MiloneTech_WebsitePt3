[1mdiff --cc Website_Python_Code/flask_website/__init__.py[m
[1mindex 1ec302e,cde8bbb..0000000[m
[1m--- a/Website_Python_Code/flask_website/__init__.py[m
[1m+++ b/Website_Python_Code/flask_website/__init__.py[m
[36m@@@ -2,6 -2,7 +2,10 @@@[m [mfrom flask import Flask, url_fo[m
  from flask_bcrypt import Bcrypt[m
  import flask_website.dbAPI.app as db[m
  from flask_login import LoginManager[m
[32m++<<<<<<< HEAD[m
[32m++=======[m
[32m+ from flask_admin import Admin[m
[32m++>>>>>>> 5719e991dc9a3214a63a84d20c2f02064841cf48[m
  import sys[m
  [m
  from flask_socketio import SocketIO[m
[36m@@@ -11,8 -13,10 +16,14 @@@[m [mapp = Flask(__name__[m
  app.config['SECRET_KEY'] = '5791628bb0b13ce0c676dfde280ba245'[m
  bcrypt = Bcrypt(app)[m
  login_manager = LoginManager(app)[m
[32m+ admin = Admin(app, name='MiloneTech Admin Page', template_mode='bootstrap4')[m
[32m+ [m
[32m+ [m
  [m
  socketio = SocketIO(app, logger=False)[m
[31m -[m
  from flask_website import routes[m
[32m++<<<<<<< HEAD[m
[32m +[m
[32m +[m
[32m++=======[m
[32m++>>>>>>> 5719e991dc9a3214a63a84d20c2f02064841cf48[m
[1mdiff --cc Website_Python_Code/flask_website/templates/single-sensor.html[m
[1mindex 583da3c,b931768..0000000[m
[1m--- a/Website_Python_Code/flask_website/templates/single-sensor.html[m
[1m+++ b/Website_Python_Code/flask_website/templates/single-sensor.html[m
[36m@@@ -90,8 -90,8 +90,13 @@@[m
          } else if(si == 3) {[m
              changeDate(30);[m
          } else if(si == 4){[m
[32m++<<<<<<< HEAD[m
[32m +            // var UserInput = prompt('Select an interval of dates');[m
[32m +            customDate(); //Work here for user input[m
[32m++=======[m
[32m+             var UserInput = prompt('Select number of days range');[m
[32m+             changeDate(parseInt(UserInput));[m
[32m++>>>>>>> 5719e991dc9a3214a63a84d20c2f02064841cf48[m
  [m
          }[m
      }[m
[36m@@@ -119,41 -118,13 +124,47 @@@[m
          });[m
      }[m
  [m
[32m +    // Nico's work. Use this function[m
[32m +    function gate_date_range(first_date, second_date) {[m
[32m +        chart_mode = first_date[m
[32m +[m
[32m +        var arr = {first_date: first_date, second_date: second_date, sensor_id: "{{sensorID}}"}[m
[32m +[m
[32m +        $.ajax({[m
[32m +            url: '/sensors/get-date-range',[m
[32m +            type: 'POST',[m
[32m +            data: JSON.stringify(arr),[m
[32m +            contentType: 'application/json; charset=utf-8',[m
[32m +            dataType: 'json',[m
[32m +            async: false,[m
[32m +            success: function(response) {[m
[32m +                console.log(response);[m
[32m +                barchart.data.labels = response["x_vals"];[m
[32m +                barchart.data.datasets.forEach((dataset) => {[m
[32m +                    dataset.data = response["y_vals"];[m
[32m +                });[m
[32m +                barchart.update()[m
[32m +[m
[32m +            }[m
[32m +        });[m
[32m +    }[m
[32m +[m
[32m +    //gate_date_range('2021/10/31', '2021/10/27');[m
[32m +[m
      function customDate(){[m
[31m -        var date1 = prompt('Starting Date eg. MM/DD/YYYY');[m
[31m -        var date2 = prompt('Ending Date eg. MM/DD/YYYY');[m
[32m +        var date1 = prompt('Ending Date (Eg. Oct 1 2021 1:00PM)');[m
[32m +        var date2 = prompt('Starting Date (Eg. Nov 1 2021 10:00AM)');[m
[32m +        /* switching dates, going out of scope, when user puts in invalid date */[m
          var curDate = new Date();[m
[32m++<<<<<<< HEAD[m
[32m +        //var daysFromDate1 = curDate.getTime() - date1.getTime();[m
[32m +        //var daysFromDate2 = curDate.getTime() - date2.getTime();[m
[32m +        gate_date_range(date1, date2);[m
[32m++=======[m
[32m+         var daysFromDate1 = curDate.getTime() - date1.getTime();[m
[32m+         var daysFromDate2 = curDate.getTime() - date2.getTime();[m
[32m+         changeDate(daysFromDate1);[m
[32m++>>>>>>> 5719e991dc9a3214a63a84d20c2f02064841cf48[m
  [m
      }[m
  [m
