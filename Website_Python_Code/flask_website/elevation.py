import requests
import json
'''

Interaction with elevation API
Current API Implementation comes from :
https://developers.airmap.com/docs/elevation-api

'''

elevation_key = """eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJjcmVkZW50aWFsX2lkIjoiY3JlZGVudGlhbHxON1d
                6emtZZlhBUk1YUHVlYXZPWXFUSzlib0pYIiwiYXBwbGljYXRpb25faWQiOiJhcHBsaWNhdGlvbnxXNE1
                Md1lxSXptd2dBelNwR1dZeDRGcEVXQU5PIiwib3JnYW5pemF0aW9uX2lkIjoiZGV2ZWxvcGVyfEs3d25
                FT1dVeXB6bmJvc21wNzh2M3M4N0p4OEIiLCJpYXQiOjE2MzY0NjQ4MTB9.ohWcYRyqTGxZ1kqg2t3sCe
                N6H1gibd143ezKnuWTcxs"""


content_type = "Content-Type: application/json; charset=utf-8"


request_url = "https://api.airmap.com/elevation/v1/ele/carpet"


'''
Param: a 2 element list that contains 2, 2 element lists. Each nested list is a latitude and longitude pair.
'''
def get_carpet_elevation(coordinate_list):
    building_url = request_url + "?"
    points_data = convert_string_list(coordinate_list)
    building_url = add_param(building_url, "points", points_data)
    building_url = add_param(building_url, "X-API-Key", elevation_key)
    building_url = add_param(building_url, "Content_Type", content_type)
    response = requests.get(building_url)
    extracted_data = json.loads(response.content)
    return (extracted_data['data']['carpet'])
    
'''
Returns the NW and SE corners of the desired coordinate points
'''
def get_surrounding_elevation(coordinate):
    pass


def add_param(building_url, param_name, param_data):
    new_url = building_url+param_name+"="+param_data
    return new_url

def convert_string_list(coordinate_list):
    tmp_format_list= ""
    for pair in coordinate_list:
        for x in pair:
            tmp_format_list += x 
    return q
    

def get_single_point_elevation():
    pass

if __name__=="__main__":
    get_carpet_elevation([[],[]])