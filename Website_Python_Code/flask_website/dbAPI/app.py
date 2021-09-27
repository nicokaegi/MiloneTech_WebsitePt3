from sqlalchemy.orm import scoped_session, sessionmaker
from . import accounts
from . import sensors
from . import sensor_readings
from . import alerts
from . import settings
from . import db


if __name__ == '__main__':
    db_session = scoped_session(sessionmaker(bind=db.engine))
    # sensors.add_sensor(533, '104324873892', 12, 'chem', 'dink')
    # alerts.add_sensor_alert(1, '100.101', 99, 1, 1)
    # alerts.add_sensor_alert(1, '100.102', 100, 0, 0)
    # alerts.add_sensor_alert(1, '100.103', 100, 0, 1)
    # alerts.add_sensor_alert(1, '100.104', 10, 1, 0)
    # alerts.add_sensor_alert(2, '100.100', 15, 1, 1)
    # print(accounts.get_phone_by_id(1))
