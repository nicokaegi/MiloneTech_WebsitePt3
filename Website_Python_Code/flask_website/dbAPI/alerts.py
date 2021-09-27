from sqlalchemy import exc
from . import db


class Alerts(db.Base):
    __table__ = db.Base.metadata.tables['alerts']


def check_alerts(sens_id):
    try:
        with db.engine.connect() as connection:
            existing_alerts = []
            result = connection.execute("select * "
                                        "from alerts "
                                        "where sensorID = '{}'"
                                        .format(sens_id))
            for row in result:
                existing_alerts.append(row)
            return existing_alerts

    except exc.SQLAlchemyError:
        return False


def add_sensor_alert(acc_id, sens_id, trigger, email_alert, phone_alert):
    try:
        with db.engine.connect() as connection:
            checker = check_alerts(sens_id)
            #if len(checker) == 0 or not checker:
            connection.execute(
                "insert into alerts (accountID, sensorID, triggerLevel, alertPhone, alertEmail)"
                "values ({}, '{}', {}, {}, {})"
                .format(acc_id, sens_id, trigger, email_alert, phone_alert))
            return True
            '''
            else:
                connection.execute(
                    "update alerts "
                    "set alerts.triggerLevel = {}, alerts.alertEmail = {}, alerts.alertPhone = {} "
                    "where alerts.accountID = {} and alerts.sensorID = '{}'"
                    .format(trigger, email_alert, phone_alert, acc_id, sens_id))
                return True
                '''
    except exc.SQLAlchemyError as e:
        print(">>>>>>>SQLAlchemy Error : " + str(e))
        return False


def remove_alert(alert_num):
    try:
        with db.engine.connect() as connection:
            connection.execute("delete from alerts "
                               "where alertsNum = %s;", (alert_num,))
            return True
    except exc.SQLAlchemyError:
        return False


def set_alert_type(alert_num, new_email, new_text):
    try:
        with db.engine.connect() as connection:

            connection.execute(
                "update alerts "
                "set alertPhone = {}, alertEmail = {} "
                "where alertsNum = {}"
                .format(new_email, new_text, alert_num))

            return True

    except exc.SQLAlchemyError as e:
        print(e)

        return False
