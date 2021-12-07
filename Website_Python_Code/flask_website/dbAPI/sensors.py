from sqlalchemy import exc
from . import db
import traceback


def get_acc_id_by_sens_id(sens_id):
    try:
        with db.engine.connect() as connection:
            acc = []
            result = connection.execute("select accountID "
                                        "from sensors "
                                        "where sensorID = {}"
                                        .format(sens_id))
            for row in result:
                acc.append(row['accountID'])

            return acc[0]
    except exc.SQLAlchemyError:
        return False


class Sensors(db.Base):
    __table__ = db.Base.metadata.tables['sensors']


def get_acc_id_by_sens_id(sens_id):
    try:
        with db.engine.connect() as connection:
            acc = []
            result = connection.execute("select accountID "
                                        "from sensors "
                                        "where sensorID = '{}' "
                                        .format(sens_id))
            for row in result:
                acc.append(row['accountID'])

            return acc[0]
    except exc.SQLAlchemyError as e:
        print(e)
        return False


def add_sensor(sens_id, sens_size = 10, sens_type = 'norm', sens_time = 60):
    try:
        with db.engine.connect() as connection:
            connection.execute("insert into sensors "
                               "values ( null , '{}', {}, '{}', null ,'{}',null)"
                               .format(sens_id, sens_size, sens_type, sens_time))
            return True
    except exc.SQLAlchemyError:
        return False


def add_sensor_to_account(sens_id, email):
    try:
        with db.engine.connect() as connection:
            connection.execute("update sensors "
                               "set sensors.accountID = (select accountID "
                                                        "from accounts "
                                                        "where accountEmail = '{}') "
                                                        "where sensorID = '{}'"
                                                        .format(email, sens_id))
            return True
    except exc.SQLAlchemyError:
        return False


def get_all_sensors(acc_id):
    try:
        with db.engine.connect() as connection:
            sens = []
            result = connection.execute("select sensorID "
                                        "from sensors "
                                        "where accountID = {}"
                                        .format(acc_id))
            print(result)
            for row in result:
                print(row)
                sens.append(row['sensorID'])
            return sens
    except exc.SQLAlchemyError:
        return False


def get_sensor_info(sens_id):
    try:
        with db.engine.connect() as connection:
            sens = []
            result = connection.execute("select * "
                                        "from sensors "
                                        "where sensorID = '{}'"
                                        .format(sens_id))
            for row in result:
                sens.append(row)
            return sens
    except exc.SQLAlchemyError:
        return False


def set_sensor_name(sens_id, sens_name):
    try:
        with db.engine.connect() as connection:
            connection.execute("update sensors "
                               "set sensorName = '{}' "
                               "where sensorID = '{}'"
                               .format(sens_name, sens_id))
            return True
    except exc.SQLAlchemyError:
        return False


def get_sensor_time_between(sens_id):
    try:
        with db.engine.connect() as connection:

            time = []
            result = connection.execute("select timeBetweenReadings "
                                      "from sensors "
                                      "where sensorID = '{}'"
                                      .format(sens_id))
            for row in result:
                time.append(row)
            return time[0]

    except exc.SQLAlchemyError:
        return False


def set_sensor_group(sens_id, sens_group):
    try:
        with db.engine.connect() as connection:
            connection.execute("update sensors "
                               "set sensorGroup = '{}' "
                               "where sensorID = '{}'"
                               .format(sens_group, sens_id))
            return True
    except exc.SQLAlchemyError:
        return False


def get_coordinates(sens_id):
    try:
        with db.engine.connect() as connection:
            sens = []
            result = connection.execute("select latitude, longitude "
                                        "from sensor_location " #not sure if this needs to be changed
                                        "where sensorID = '{}'" # This is the code from sens_id
                                        .format(sens_id))
            for row in result:
                sens.append(row)
            return sens
    except  exc.SQLAlchemyError: 
        traceback.print_exc()
        return False