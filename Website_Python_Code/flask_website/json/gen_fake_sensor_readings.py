import json
import datetime
import random

'''
    Uses the correct time_between_readings field in the
    fake_sensors.json file to create life like readings
    for fake sensors. 
    
    Currently will overwrite the old file of fake_sensors_readings.json
    
    Also will generate a random battery level for the fake sensor readings
    
    Created by: Isaac
    date: 10/30/2020
'''

with open("fake_sensors.json","r") as f:
    sensor_info = json.load(f)

target = []

for i in range(1,4):
    
    dict = {}
    dict["sensor"] = "00%s" % (str(i))
    
    x_vals = []
    y_vals = []
    
    diff_readings = datetime.timedelta(minutes=( int( sensor_info[i-1]["time_between_readings"] ) ) )
    
    slope = input("enter slope")
    starting = input("enter y intercept")
    
    for i in range (0,10):

        y_curr = min(1,max(0,( float(starting) + ( float(slope) * i))))
        x_curr = datetime.datetime.now() + (diff_readings * i)
        x_vals.append(str(x_curr))
        y_vals.append(round(y_curr,4))
        
    dict["x_vals"]=x_vals
    dict["y_vals"]=y_vals
    dict["battery_level"] = round(random.random(),4)
    
    target.append(dict)
    
print(target)

with open("fake_sensor_readings.json","w") as f:
    json.dump(target, f, indent="\t")