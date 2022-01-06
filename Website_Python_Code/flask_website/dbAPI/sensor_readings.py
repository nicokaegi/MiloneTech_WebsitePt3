from sqlalchemy import exc
from . import db
from . import sensors
import traceback


class SensorReadings(db.Base):
    __table__ = db.Base.metadata.tables['sensor_readings']


def add_reading_no_time(sens_id, liquid, battery, rssi):
    try:
        with db.engine.connect() as connection:
            connection.execute("insert into sensor_readings "
                               "values (default, {}, '{}', {}, {}, default, {})"
                               .format(sensors.get_acc_id_by_sens_id(sens_id), sens_id, liquid, battery, rssi))
            return True
    except exc.SQLAlchemyError:
        return False


def add_reading_yes_time(sens_id, liquid, battery, time_stamp, rssi):
    try:
        with db.engine.connect() as connection:
            connection.execute("insert into sensor_readings "
                               "values (default, {}, '{}', {}, {}, {}, {})"
                               .format(sensors.get_acc_id_by_sens_id(sens_id), sens_id, liquid, battery, time_stamp, rssi))
            return True
    except exc.SQLAlchemyError:
        return False


def add_reading_no_time(sens_id, liquid, battery, rssi):
    try:
        acc_id = sensors.get_acc_id_by_sens_id(sens_id)
        if acc_id is None:
            acc_id = 'null'
        with db.engine.connect() as connection:
            connection.execute("insert into sensor_readings "
                               "values (default, {}, '{}', {}, {}, default, {})"
                               .format(acc_id, sens_id, liquid, battery, rssi))
            return True
    except exc.SQLAlchemyError:
        return False


def add_reading_yes_time(sens_id, liquid, battery, time_stamp, rssi):
    try:
        acc_id = sensors.get_acc_id_by_sens_id(sens_id)
        if acc_id is None:
            acc_id = 'null'

        with db.engine.connect() as connection:
            connection.execute("insert into sensor_readings "
                               "values (default, {}, '{}', {}, {}, '{}', {})"
                               .format(acc_id, sens_id, liquid, battery, time_stamp, rssi))
            return True
    except exc.SQLAlchemyError:
        return False


def get_sensor_data_points(sens_id):
    try:
        with db.engine.connect() as connection:
            data = []
            result = connection.execute("select * "
                                        "from sensor_readings "
                                        "where sensorID = '{}'"
                                        "order by recordTime asc"
                                        .format(sens_id))
            for row in result:
                data.append(row)
            return data
    except exc.SQLAlchemyError:
        return False


# get the last n datapoints, only recordTime and liquidLevel
def get_n_sensor_data_points(sens_id, n):
    try:
        with db.engine.connect() as connection:
            data = []
            result = connection.execute("select recordTime, liquidLevel "
                                        "from sensor_readings "
                                        "where sensorID = '{}' "
                                        "order by recordTime desc "
                                        "limit {}"
                                        .format(sens_id, n))
            for row in result:
                data.append(row)
            data.reverse()  # because it's current in descending order, no good for charts
            return data
    except exc.SQLAlchemyError:
        return False


def get_sensor_data_points_by_date(sens_id, start_date, end_date=None, max_size=-1):
    try:
        with db.engine.connect() as connection:
            data = []
            # Query for the sensor_readings in our date interval
            if end_date is None:
                result = connection.execute("select recordTime, liquidLevel "
                                            "from sensor_readings "
                                            "where sensorID = '{}' and recordTime > '{}' "
                                            "order by recordTime desc "
                                            .format(sens_id, start_date))
            else:
                result = connection.execute("select recordTime, liquidLevel "
                                            "from sensor_readings "
                                            "where sensorID = '{}' and recordTime > '{}' and recordTime < '{}' "
                                            "order by recordTime desc "
                                            .format(sens_id, start_date, end_date))

            # Turn our SQL result into a list that we can actually use
            for row in result:
                data.append(row)

            # if max size was passed and is less than the length of the data,
            if 0 < max_size < len(data):
                # cut it down to roughly max size
                # TODO: change the way we exclude values. If rate of change is high, this may not show critical values
                step = int(len(data) / max_size)
                new_data = data[::step]
                data = new_data

                # then make it exactly max size
                data = data[:max_size]

            # Reverse the list order because it's currently in descending order, and this is designed for chart use
            data.reverse()

            return data

    except exc.SQLAlchemyError as e:
        traceback.print_exception()
        return False


def get_sensor_battery(sens_id):
    try:
        with db.engine.connect() as connection:
            battery = ""
            result = connection.execute("select batteryLevel "
                                        "from sensor_readings "
                                        "where sensorID = '{}' "
                                        "order by recordNumber "
                                        "desc limit 1"
                                        .format(sens_id))
            for row in result:
                battery = row['batteryLevel']
            return battery
    except exc.SQLAlchemyError:
        return False


def get_liquid_level(sens_id):
    try:
        with db.engine.connect() as connection:
            liquid = 0
            result = connection.execute("select liquidLevel "
                                        "from sensor_readings "
                                        "where sensorID = '{}' "
                                        "order by recordNumber "
                                        "desc limit 1".format(sens_id))
            for row in result:
                liquid = row['liquidLevel']
            return liquid
    except exc.SQLAlchemyError:
        return False
