import json

targetFile = input("Please input the file you'd like to change...")

with open(targetFile,"r") as f:
    target = json.load(f)

print(target)

targetFileSettings = input("a -> append, w -> clobber")
numberEntries = input("please enter the number of new entries")
numberKeys = input("please enter the number of new keys (or 0 to follow old)")

#old_file_forbackup = targetFile + "_old"

#with open(old_file_forbackup,"x") as f:
#    json.dump(target, f)

if int(numberKeys) == 0:
    
    for i in range(0,int(numberEntries)):
        
        newEntry = {}
        
        for key in target[0]:
                
            newEntry[key] = input("enter new %s\n" % key)
            
        target.append(newEntry)
    
else:
    
    keyList = []
    
    for i in range(0,int(numberKeys)):
        
        keyList.append(input("Please enter key #%s" % str(i)))
        
    for i in range(0,int(numberEntries)):
        
        newEntry = {}
        
        for key in keyList:
                
            newEntry[key] = input("enter new %s\n" % key)
            
        target.append(newEntry)
        

with open(targetFile,targetFileSettings) as f:
    json.dump(target, f,indent="\t")
    