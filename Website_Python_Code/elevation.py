import requests
import json

'''
Interaction with elevation API
Current API Implementation comes from :
https://developers.airmap.com/docs/elevation-api
'''


#API Parameters
elevation_key = """eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJjcmVkZW50aWFsX2lkIjoiY3JlZGVudGlhbHxON1d
                6emtZZlhBUk1YUHVlYXZPWXFUSzlib0pYIiwiYXBwbGljYXRpb25faWQiOiJhcHBsaWNhdGlvbnxXNE1
                Md1lxSXptd2dBelNwR1dZeDRGcEVXQU5PIiwib3JnYW5pemF0aW9uX2lkIjoiZGV2ZWxvcGVyfEs3d25
                FT1dVeXB6bmJvc21wNzh2M3M4N0p4OEIiLCJpYXQiOjE2MzY0NjQ4MTB9.ohWcYRyqTGxZ1kqg2t3sCe
                N6H1gibd143ezKnuWTcxs"""

content_type = "Content-Type: application/json; charset=utf-8"

carpet_request_url = "https://api.airmap.com/elevation/v1/ele/carpet"

point_request_url =  "https://api.airmap.com/elevation/v1/ele"


'''
Param: a 2 element list that contains 2, 2 element lists. Each nested list is a latitude and longitude pair.
'''
def get_carpet_elevation(coordinate_point:list) -> list:
    building_url = carpet_request_url + "?"
    coordinate_list = get_surrounding_elevation(coordinate_point)
    points_data = convert_string_list(coordinate_list)
    building_url = add_param(building_url, "points", points_data)
    building_url = add_param(building_url, "X-API-Key", elevation_key)
    building_url = add_param(building_url, "Content_Type", content_type)
    response = requests.get(building_url)
    extracted_data = json.loads(response.content)
    return (extracted_data['data']['carpet'])
    
'''
Returns the NW and SE corners of the desired coordinate points
param: The coordinates and the bounds.
Bounds are defaulted to 10 meters
'''
def get_surrounding_elevation(coordinate:list, bounds:int=10) -> list:
    offset = bounds/111111
    lat = coordinate[0]
    log = coordinate[1]
    nw = [float(lat+offset),float(log-offset)]
    se = [lat-offset,log+offset]
    return [nw,se]




"""
Adds a parameter to the API request
takes in the url it is building off of, and applies the parameter name and data to it
"""
def add_param(building_url:str, param_name: str, param_data :object) -> list:
    new_url = building_url+param_name+"="+param_data
    return new_url


"""
Converts coordinate_lists into format acceptable to be put in a get request
"""
def convert_string_list(coordinate_list: list) -> str:
    tmp_format_list= ""
    for pair in coordinate_list:
        for x in pair:
            tmp_format_list += str(x) +","
    tmp_format_list = tmp_format_list[:-1]
    return tmp_format_list



'''
Takes in any amount of coordinate points (Lat,Long pairs) as a list of 2-element lists
and returns elevations for those points 
'''
def get_point_elevations(coordinate_list:list) -> list:
    coordinate_list = pairs(coordinate_list)
    building_url = point_request_url + "?"
    building_url = add_param(building_url, "points", convert_string_list(coordinate_list))
    building_url = add_param(building_url, "X-API-Key", elevation_key)
    building_url = add_param(building_url, "Content_Type", content_type)
    response = requests.get(building_url)
    extracted_data = json.loads(response.content)
    return(extracted_data["data"])

def pairs(coordinates):
    tmp = []
    values = []
    for i in coordinates:
        tmp.append(i)
        if len(tmp) == 2:
            values.append([tmp[0],tmp[1]])
            tmp.clear()
    return values



if __name__=="__main__":
    print(get_point_elevations([[39.70179236136183, -75.12333012077949]]))