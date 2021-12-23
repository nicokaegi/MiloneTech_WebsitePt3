from sqlalchemy import exc
from . import db


class Accounts(db.Base):
    __table__ = db.Base.metadata.tables['accounts']

def get_id_by_email(acc_email):
    try:
        with db.engine.connect() as connection:
            acc_id = []
            result = connection.execute("select accountID "
                                        "from accounts "
                                        "where accountEmail = '{}'"
                                        .format(acc_email))
            for row in result:
                # print(row)
                acc_id.append(row[0])
            if len(acc_id) > 0:
                return acc_id[0]
            else:
                return False
    except exc.SQLAlchemyError:
        return False
        
def get_email_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            email = []
            result = connection.execute("select accountEmail "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            for row in result:
                email.append(row['accountEmail'])
            return email[0]
    except exc.SQLAlchemyError:
        return False

def get_id_by_email(acc_email):
    try:
        with db.engine.connect() as connection:
            acc_id = []
            result = connection.execute("select accountID "
                                        "from accounts "
                                        "where accountEmail = '{}'"
                                        .format(acc_email))
            for row in result:
                # print(row)
                acc_id.append(row[0])
            if len(acc_id) > 0:
                return acc_id[0]
            else:
                return False
    except exc.SQLAlchemyError:
        return False


def get_status_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            status = []
            result = connection.execute("select accountStatus "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            for row in result:
                email.append(row['accountStatus'])
            return email[0]
    except exc.SQLAlchemyError:
        return False

def get_all_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            info = []
            result = connection.execute("select * "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            for row in result:
                info.append(row)
            return info[0]
    except exc.SQLAlchemyError:
        return False

def get_phone_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            phone = []
            result = connection.execute("select phoneNumber "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            for row in result:
                phone.append(row['phoneNumber'])
            return phone[0]
    except exc.SQLAlchemyError:
        return False


def get_name_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            name = []
            result = connection.execute("select fname, lname "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            for row in result:
                name.append((row['fname'], row['lname']))
            return name[0]
    except exc.SQLAlchemyError:
        return False


def get_pass_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            pass_hash = []
            result = connection.execute("select passwordHash "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            print(result)
            for row in result:
                print(row)
                print(type(row))
                print(type(row[0]))
                pass_hash.append(row[0])
            return pass_hash[0]
    except exc.SQLAlchemyError:
        return False


def get_status_by_id(account_id):
    try:
        with db.engine.connect() as connection:
            account_status = []
            result = connection.execute("select accountStatus "
                                        "from accounts "
                                        "where accountID = {}"
                                        .format(account_id))
            for row in result:
                account_status.append(row['accountStatus'])
            return account_status[0]
    except exc.SQLAlchemyError:
        return False


def create_account(email, first_name, last_name, pass_hash):
    try:
        with db.engine.connect() as connection:
            connection.execute("insert into accounts "
                               "values (default, '{}', '{}', '{}', null, '{}', 0)"
                               .format(email, first_name, last_name, pass_hash))
            return True
    except exc.SQLAlchemyError:
        return False

def create_account(email, first_name, last_name, pass_hash, phone_number):
    try:
        with db.engine.connect() as connection:
            connection.execute("insert into accounts "
                               "values (default, '{}', '{}', '{}', '{}', '{}', 0)"
                               .format(email, first_name, last_name, phone_number, pass_hash))
            return True
    except exc.SQLAlchemyError:
        return False

def delete_account_by_id(user_id):
    try:
        with db.engine.connect() as connection:
            connection.execute("DELETE FROM accounts "
                               "WHERE accountID = {}"
                               .format(user_id))
            return True
    except exc.SQLAlchemyError:
        return False

def set_account_email(old_email, new_email):
    try:
        with db.engine.connect() as connection:
            connection.execute("update accounts "
                               "set accountEmail = '{}' "
                               "where accountEmail = '{}'"
                               .format(new_email, old_email))
            return True
    except exc.SQLAlchemyError:
        return False


def set_account_password(pass_hash, email):
    try:
        with db.engine.connect() as connection:
            connection.execute("update accounts "
                               "set passwordHash = '{}' "
                               "where accountEmail = '{}'"
                               .format(pass_hash, email))
            return True
    except exc.SQLAlchemyError:
        return False


def set_account_payment_tier(status, email):
    try:
        with db.engine.connect() as connection:
            connection.execute("update accounts "
                               "set accountStatus = {} "
                               "where accountEmail = '{}'"
                               .format(status, email))
            return True
    except exc.SQLAlchemyError:
        return False


def set_account_phone(phone, email):
    try:
        with db.engine.connect() as connection:
            connection.execute("update accounts "
                               "set phoneNumber = '{}' "
                               "where accountEmail = '{}'"
                               .format(email,phone))
            return True
    except exc.SQLAlchemyError:
        return False
