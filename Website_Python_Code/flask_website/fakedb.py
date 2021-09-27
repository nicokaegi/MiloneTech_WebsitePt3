'''
    Represents all the functionality we will eventually get out
    of a real data base and dbAPI.py once the AWS team gets it up
    and running.
    
    For testing purposes with flask app.
'''
import json

usersData, sensorsData, sensorReadingsData = None, None, None

def fakedb_init():
    
    global usersData, sensorsData, sensorReadingsData
    
if not usersData:
    with open("flask_website/json/fake_users.json","r") as f:
        usersData = json.load(f)
    
    with open("flask_website/json/fake_sensors.json","r") as f:
        sensorsData = json.load(f)
        
    with open("flask_website/json/fake_sensor_readings.json","r") as f:
        sensorReadingsData = json.load(f)

def getUserPassword(targetEmail):
    
    for user in usersData:
        if user["email"] == targetEmail:
            return user["password"]
    
    return "$2b$12$mE3X.vIwpjlryiuGTBX17OqdautRHtG3cpPSF/zrgJWLmGXoANZIO"

def createNewAccount(accountEmail,password,name):
    
    for user in usersData:
        if user["email"] == accountEmail:
            return False
    
    new_user = {}
    new_user["email"] = str(accountEmail)
    new_user["password"] = str(password)
    new_user["name"] = str(name)
    new_user["is_paid"] = False
    new_user["owns"] = []

    usersData.append(new_user)
    
    with open("flask_website/json/fake_users.json","w") as f:
        json.dump(usersData, f, indent="\t")

def validateEmail(targetEmail):
    
    for user in usersData:
        if user["email"] == targetEmail:
            return True
        
    return False

def getPaymentTier(targetEmail):
    
    for user in usersData:
        if user["email"] == targetEmail:
            return user["is_paid"]
    
    return False

def setPaymentTier(targetEmail, newTier):
    
    for user in usersData:
        if user["email"] == targetEmail:
            user["is_paid"] = newTier
            
            with open("flask_website/json/fake_users.json","w") as f:
                json.dump(usersData, f, indent="\t")
                
            return True
    
    return False

def getAllSensors(targetEmail):
    
    for user in usersData:
        if user["email"] == targetEmail:
            return user["owns"]
    
    return False

def setSensorInfo(sensorID, newReading, newBattery, timeStamp):
    
    for sensor in sensorReadingsData:
        if sensor["sensor"] == sensorID:
            
            sensor["x_vals"].append(timeStamp)
            sensor["y_vals"].append(newReading)
            sensor["battery_level"] = newBattery
            
            print("finished\nadded %s from %s to %s bat %s" % (newReading,timeStamp,sensorID,newBattery))
            print(sensor)
            
            with open("flask_website/json/fake_sensor_readings.json","w") as f:
                json.dump(sensorReadingsData, f, indent="\t")
            

def getSensorInfo(targetSensor):
    
    returned = {}
    
    for sensor in sensorsData:
        if sensor["sensorID"] == targetSensor:
            returned["name"] = sensor["name"]
            returned["time_between_readings"] = sensor["time_between_readings"]
            returned["group"] = sensor["group"]
            
            return returned
        
    return False

def getSensorDataPoints(targetSensor):
    
    returned = {}
    
    for sensor in sensorReadingsData:
        if sensor["sensor"] == targetSensor:
            
            returned["sensorID"] = sensor["sensor"]
            returned["x_vals"] = sensor["x_vals"]
            returned["y_vals"] = sensor["y_vals"]
            
            return returned
        
    return False

def getSensorBattery(targetSensor):
    
    for sensor in sensorReadingsData:
        if sensor["sensorID"] == targetSensor:
            
            return sensor["battery_level"]
        
    return False

def setUserPassword(targetEmail,newPassword):
    
    for user in usersData:
        if user["email"] == targetEmail:
            
            user["password"]=newPassword
            
            with open("flask_website/json/fake_users.json","w") as f:
                json.dump(usersData, f, indent="\t")
            return True
        
    return False

def addSensorToAccount(targetEmail,targetSensor):
    
    for user in usersData:
        if user["email"] == targetEmail:
            
            user["owns"].append(targetSensor)
            
            with open("flask_website/json/fake_users.json","w") as f:
                json.dump(usersData, f, indent="\t")
                
            return True
        
    return False 

def setSensorName(targetSensor,newName):
    
    for sensor in sensorReadingsData:
        if sensor["sensorID"] == targetSensor:
            
            sensor["name"]=newName
            
            with open("flask_website/json/fake_sensors.json","w") as f:
                json.dump(sensorsData, f, indent="\t")
            return True
        
    return False

def setSensorGroup(targetSensor,newGroup):
    
    for sensor in sensorReadingsData:
        if sensor["sensorID"] == targetSensor:
            
            sensor["group"]=newGroup
            
            with open("flask_website/json/fake_sensors.json","w") as f:
                json.dump(sensorsData, f, indent="\t")
            return True
        
    return False
    
if __name__ == "__main__":
    from pprint import pprint 
    pprint(usersData)
    
    