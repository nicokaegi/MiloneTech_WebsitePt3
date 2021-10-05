from flask import render_template, url_for, flash, redirect, Response, request

import datetime
import io
import base64
import json

import flask_website.emailer as email
from flask_website.forms import RegistrationForm, LoginForm, SettingsForm, AccountForm, SensorAccountForm, \
    RequestResetForm, ResetPasswordForm
from flask_website import app, bcrypt, db, login_manager
from itsdangerous import TimedJSONWebSignatureSerializer as Serializer
from flask_login import login_user, current_user, logout_user, login_required, UserMixin
import datetime

from pprint import pprint

from flask_website import socketio
from flask_socketio import SocketIO, emit, send

# define a dictionary to store active sessions,
# key is SocketIO client ID, value is account ID
sessions = {}


# Creates an abomination of a data item in it's writer's on words:
# defines current_user.user_data. I would recommend just experimenting to see
# what the structure is.
class User(UserMixin):
    def initialize_user_data(self):
        data = {}

        data["id"] = self.id

        data["name"] = db.accounts.get_name_by_id(self.id)[0]

        data["email"] = self.email
        data["payment_tier"] = db.accounts.get_status_by_id(self.id)

        data["sensor_data"] = dict()
        curr_user_sensors = db.sensors.get_all_sensors(self.id)

        curr_user_groups = {}

        # find all groups for current user
        for sensor in curr_user_sensors:

            curr_group = db.sensors.get_sensor_info(sensor)[0][6]

            if curr_group != None:
                if curr_group not in curr_user_groups:

                    temp_list = []
                    temp_list.append(sensor)
                    curr_user_groups[curr_group] = temp_list

                else:

                    curr_user_groups[curr_group].append(sensor)
            else:
                if "No Group" not in curr_user_groups:

                    temp_list = []
                    temp_list.append(sensor)
                    curr_user_groups["No Group"] = temp_list

                else:

                    curr_user_groups["No Group"].append(sensor)

        for group in curr_user_groups:

            '''[(535, '100', 10, 'norm', 'water_tower1', 60, None)]'''
            group_list = curr_user_groups[group]
            data["sensor_data"][group] = {}

            for sensor in group_list:

                curr_sensor = {}
                sensor_data = db.sensors.get_sensor_info(sensor)[0]

                print(sensor_data)

                curr_sensor["name"] = sensor_data[4]
                curr_sensor["x_vals"] = []
                curr_sensor["y_vals"] = []
                curr_sensor["type"] = sensor_data[3]
                battery_lev = db.sensor_readings.get_sensor_battery(sensor)
                if battery_lev:
                    curr_sensor["bat_level"] = battery_lev
                else:
                    curr_sensor["bat_level"] = 0

                sensor_values = db.sensor_readings.get_sensor_data_points(sensor)
                counter = 0

                for data_point in sensor_values:
                    dateSQL = data_point[5]
                    dateSQL = dateSQL - datetime.timedelta(hours=5)  # The sensors report 5 hours ahead of EST
                    date = str(dateSQL)

                    curr_sensor["x_vals"].append(date)
                    curr_sensor["y_vals"].append(data_point[3])
                    counter = counter + 1

                if len(curr_sensor['y_vals']) > 20:
                    num_readings = len(curr_sensor['y_vals'])
                    # Overwrites the x_vals and y_vals with the last 20 elements of themselves
                    # Replace 20 with something else for a different default number of displayed data points
                    curr_sensor['x_vals'] = curr_sensor['x_vals'][num_readings - 20:]
                    curr_sensor['y_vals'] = curr_sensor['y_vals'][num_readings - 20:]

                data["sensor_data"][group][sensor] = curr_sensor

        self.user_data = data

    def get_reset_token(self, expires_sec=1800):

        s = Serializer(app.config["SECRET_KEY"], expires_sec)
        return s.dumps({'user_id': self.id}).decode('utf-8')

    def verify_reset_token(token):

        s = Serializer(app.config["SECRET_KEY"])

        try:
            user_id = s.loads(token)['user_id']
        except:
            return None

        return user_id

    def __init__(self, userID):
        self.id = userID
        self.email = db.accounts.get_email_by_id(userID)
        self.user_data = None


@login_manager.user_loader
def load_user(user_id):
    return User(user_id)

@app.route("/", methods=['GET', 'POST'])
@app.route("/login", methods=['GET', 'POST'])
def login():
    if current_user.is_authenticated:
        return redirect(url_for('home'))

    form = LoginForm()

    if form.validate_on_submit():

        userID = db.accounts.get_id_by_email(form.email.data)

        if userID is False:
            flash('Login Unsuccessful. Please check username and password', 'danger')
            return render_template('login.html', title='Login', form=form)

        user = User(userID)

        if user and bcrypt.check_password_hash(db.accounts.get_pass_by_id(userID), str(form.password.data)):
            login_user(user, remember=form.remember.data)
            return redirect(url_for('home'))

        else:
            flash('Login Unsuccessful. Please check username and password', 'danger')

    return render_template('login.html', title='Login', form=form)


@app.route("/home")
@login_required
def home():
    current_user.initialize_user_data()
    return render_template('home.html', account_info=current_user.user_data)


# Default sensors page
@app.route("/sensors")
@login_required
def sensors():
    current_user.initialize_user_data()
    return render_template('sensors.html', account_info=current_user.user_data)


# Page for filtering sensors by group
@app.route("/sensors/sensor-group/<sensor_group>")
@login_required
def sensor_group_route(sensor_group):
    current_user.initialize_user_data()
    return render_template('sensor-group.html', account_info=current_user.user_data, groupFilter=sensor_group)


# Returns the html page for a single sensor, where measurement can be configured
@app.route("/sensors/<sensor_id>")
@login_required
def single_sensor_page_route(sensor_id):
    try:
        auth_id = db.sensors.get_acc_id_by_sens_id(sensor_id)
        if not (str(auth_id) == str(current_user.id)):
            return "Sensor not found", 404
    except:
        return "Sensor not found", 404

    data = db.sensor_readings.get_n_sensor_data_points(sensor_id, 20)
    chart_data = {"x_vals": [], "y_vals": []}
    for datapoint in data:
        chart_data['x_vals'].append(str(datapoint[0] - datetime.timedelta(hours=5)))
        chart_data["y_vals"].append(datapoint[1])

    sensor_settings = db.settings.get_sensor_settings(sensor_id)

    return render_template('single-sensor.html', data=chart_data, sensorID=sensor_id, settings=sensor_settings)


# Send POST requests to here to receive data points within a certain timeframe
# See below for details on the contents of the POST
@app.route("/sensors/get-range", methods=["POST"])
@login_required
def get_sensor_data_range_route():
    # A JSON should have been passed via the post with items "start_date" and "end_date", whose
    # elements are the lower and upper time bounds of the sensor readings we wish to query, in
    # datetime format: 'YYYY-MM-DD HH:MM:SS'
    data = request.json
    start_date = datetime.datetime.now() - datetime.timedelta(days=(data["days"]))
    start_date.replace(hour=0, minute=0, second=0)
    sensor_id = data["sensor_id"]

    # Dynamically determine number of datapoints to display
    num_datapoints = 0
    if data["days"] >= 30:
        num_datapoints = 60
    elif data["days"] >= 7:
        num_datapoints = 24
    else:
        num_datapoints = 24

    # Ensure that only the owner of the sensor can view this data
    # Comment the following if-statement out if you need send test requests from an outside source like Reqbin
    # TODO: this 403 should be bypassed if current_user is an admin
    if str(db.sensors.get_acc_id_by_sens_id(sensor_id)) != str(current_user.id):
        return "Unauthorized", 403

    # Grab the data using the appropriate database function. Adjust the max_size argument to the number
    # of data points you think this function should return, or remove it for all of them (potentially thousands)
    data = db.sensor_readings.get_sensor_data_points_by_date(sensor_id, start_date, max_size=num_datapoints)

    # Then parse it into a new JSON, chart_data, for a more usable form in the chart on the clientside
    chart_data = {"x_vals": [], "y_vals": []}
    for datapoint in data:
        chart_data['x_vals'].append(str(datapoint[0] - datetime.timedelta(hours=5)))
        chart_data["y_vals"].append(datapoint[1])

    return chart_data

# POST sensor  settings to the db
@app.route("/sensors/sensor-settings/store", methods=["POST"])
@login_required
def store_settings_route():
    request_data = request.json

    logger.info("{}".format(request_data))

    if str(db.sensors.get_acc_id_by_sens_id(request_data["sensorID"])) != str(current_user.id):
        return "Unauthorized", 403

    data = [request_data["sensorID"],
            request_data["measurementType"],
            request_data["width"],
            request_data["length"],
            request_data["radius"],
            request_data["height"],
            request_data["sensorBottomHeight"],
            request_data["sensorTopHeight"]]

    result = db.settings.store_sensor_settings(data)
    return {"result": result}


@app.route("/profile")
@login_required
def profile():
    return render_template('page-user.html')


@app.route("/notifications")
@login_required
def notifications():
    return render_template('notifications.html')


@app.route("/maps")
@login_required
def maps():
    return render_template('maps.html')


@app.route("/support")
def support():
    return render_template('support.html')


'''
register : this handles the intial user generation, by taking in info
from the form in the registration template, hashing the chosen password
and, then storing all relevant information into the database

the form on the front end confirms password choice, and well as if the email is valid
'''
@app.route("/register", methods=['GET', 'POST'])
def register():
    if current_user.is_authenticated:
        return redirect(url_for('home'))

    form = RegistrationForm()

    if form.validate_on_submit():
        hashed_pass = bcrypt.generate_password_hash(form.password.data).decode('utf-8')

        fullname = form.name.data.split()
        firstname = fullname[0]
        lastname = fullname[-1]

        db.accounts.create_account(form.email.data, firstname, lastname, hashed_pass)

        flash(f'Your account has been Created! You may now Login', 'success')
        return redirect(url_for('login'))

    return render_template('register.html', title='Register', form=form)


@app.route("/sensor", methods=['POST'])
def sensor():
    data_string = "Received post at: %s\n" % datetime.datetime.now()
    print(data_string)
    data = request.json
    data_string = data_string + str(data)


    #with open('./flask_website/records.txt', 'a') as f:
        #f.write(data_string)

    sensorID = data["Sensor ID"]

    '''all assuming that the sensor already exists in the DB'''
    curr_sensor_info = db.sensors.get_sensor_info(sensorID)

    if curr_sensor_info and curr_sensor_info[0][0] is not None:

        sensor_name = curr_sensor_info[0][4]

        # if the sensor_name is Null in DB, just make it equal to the sensorID
        if not sensor_name:
            sensor_name = sensorID

        owner_acc_info = db.accounts.get_all_by_id(curr_sensor_info[0][0])
        '''owner_acc_info structure FOR NOW
        (acc_id, 'acc_email', fname , lname, number, pass_hash, is_paid)'''

        curr_sensor_alerts = db.alerts.check_alerts(sensorID)
        '''curr_sensor_alerts structure FOR NOW
        [(rec num,acc_id, 'sens_id', trig_lev , email? (0/1/2), text? (0/1/2))]'''

        for poss_alert in curr_sensor_alerts:

            for entry in data["Sensor Data"]:

                email_alert_enc = poss_alert[4]
                text_alert_enc = poss_alert[5]
                hit = False

                if int(entry["Liquid %"]) >= poss_alert[3]:

                    '''(to_email, sensor, curr_user_name, alert_level, curr_level):'''
                    if email_alert_enc == 2:
                        email_alert_enc = 1
                        hit = True
                        pass

                    if text_alert_enc == 2:
                        text_alert_enc = 1
                        hit = True
                        pass

                    if hit:
                        db.alerts.set_alert_type(poss_alert[0], email_alert_enc, text_alert_enc)
                        break

        # Code for sending out an email if the level is un-triggered
        for poss_alert in curr_sensor_alerts:

            for entry in data["Sensor Data"]:

                email_alert_enc = poss_alert[4]
                text_alert_enc = poss_alert[5]
                hit = False

                if int(entry["Liquid %"]) < poss_alert[3]:

                    '''(to_email, sensor, curr_user_name, alert_level, curr_level):'''
                    if email_alert_enc == 1:
                        full_name = owner_acc_info[2] + " " + owner_acc_info[3]
                        email.send_email_notification(owner_acc_info[1], sensor_name, full_name, poss_alert[3],
                                                      entry["Liquid %"])
                        email_alert_enc += 1
                        hit = True
                        pass

                    if text_alert_enc == 1:
                        if owner_acc_info[4]:
                            email.send_text_notification(owner_acc_info[4], sensor_name, full_name, poss_alert[3],
                                                         entry["Liquid %"])
                            text_alert_enc += 1
                            hit = True
                        pass

                    if hit:
                        db.alerts.set_alert_type(poss_alert[0], email_alert_enc, text_alert_enc)
                        break

    else:
        # the sensor doesn't exist in the db... create it
        # (TODO) FOR NOW with default values
        db.sensors.add_sensor(sensorID, data["Sensor Leng"], data["Sensor Type"])

    # Capture a list of the user's current sessions
    user_sessions = []
    accountID = db.sensors.get_acc_id_by_sens_id(sensorID)
    for session in sessions:
        if sessions[session] == str(accountID):
            user_sessions.append(session)

    # For each datapoint in this POST
    for entry in data["Sensor Data"]:

        # Convert timestamp to datetime
        timestamp_datetime = datetime.datetime.strptime(entry["Time Stamp"], "%a %b %d %H:%M:%S %Y\n")
        entry["Time Stamp"] = str(timestamp_datetime)

        # Add to database
        db.sensor_readings.add_reading_yes_time(sensorID, entry["Liquid %"], entry["Battery %"],
                                                entry["Time Stamp"], entry["RSSI"])

        timestamp_datetime = timestamp_datetime - datetime.timedelta(hours=5)

        entry["Time Stamp"] = str(timestamp_datetime)
        # Then send the entry to the charts in all of the user's active sessions
        for session in user_sessions:
            socketio.emit('POST', [sensorID, entry], to=session)

    time_response = str(db.sensors.get_sensor_time_between(sensorID)[0])
    print(time_response)

    return time_response


@app.route("/account", methods=['GET', 'POST'])
@login_required
def account():
    form = AccountForm()
    sensorAccountForm = SensorAccountForm()
    if form.validate_on_submit():
        if form.newEmail.data != '':
            db.accounts.set_account_email(current_user.email, form.newEmail.data)
            flash('email updated. New Email: ' + current_user.email, 'success')
        if form.phoneNumber.data != '':
            db.accounts.set_account_phone(current_user.email, form.phoneNumber.data)
            flash('phone number updated. New Phone Number:' + form.phoneNumber.data, 'success')
        if form.password.data != '':
            hashed_pass = bcrypt.generate_password_hash(form.password.data).decode('utf-8')
            if form.password.data == form.confirmPassword.data:
                db.accounts.set_account_password(hashed_pass, current_user.email)
                flash('password updated', 'success')
    if sensorAccountForm.validate_on_submit():
        if sensorAccountForm.sensorID.data != '':
            flash('sensor ID: ' + sensorAccountForm.sensorID.data + ' has been added to your account', 'success')
            db.sensors.add_sensor_to_account(sensorAccountForm.sensorID.data, current_user.email)
            # db.accounts.set_account_payment_tier(0, current_user.email) Proof that it does Work. TODO: Make it work.

    return render_template('account.html', title='Account', form=form, sensorAccountForm=sensorAccountForm,
                           account_info=current_user.user_data, currentUser=current_user)




@app.route("/logout")
@login_required
def logout():
    logout_user()
    return redirect(url_for('login'))


@app.route("/reset_password", methods=['GET', 'POST'])
def reset_request():
    form = RequestResetForm()

    if form.validate_on_submit():
        user = db.accounts.get_id_by_email(form.email.data)
        flash('An email has been sent with instructions on how to reset your password')
        user_obj = User(user)
        token = User.get_reset_token(user_obj)
        email.send_password_request(form.email.data, url_for('reset_token', token=token, _external=True))

        return redirect(url_for('login'))

    return render_template('reset_request.html', title="Reset Password", form=form)


@app.route("/reset_password/<token>", methods=['GET', 'POST'])
def reset_token(token):
    user = load_user(User.verify_reset_token(token))

    if user is None:
        flash('That is an Invalid/Expired Token', 'warning')
        return redirect(url_for('reset_request'))

    form = ResetPasswordForm()

    if form.validate_on_submit():
        hashed_pass = bcrypt.generate_password_hash(form.password.data).decode('utf-8')
        db.accounts.set_account_password(hashed_pass, user.email)

        flash('Your password has been updated! You may now Login', 'success')
        return redirect(url_for('login'))

    return render_template('reset_token.html', title="Reset Password", form=form)


# Catch users connecting, store the (session id):(user id) pair in the sessions dictionary
@socketio.on("connect")
def connect():
    sessions[request.sid] = current_user.id
    print("connected user ID " + current_user.id + " to session " + request.sid)
    print("Active Sessions:")
    pprint(sessions)


# Catch users disconnecting, remove that session id's entry from the sessions dictionary
@socketio.on("disconnect")
def disconnected():
    del sessions[request.sid]
    print("disconnected user ID " + current_user.id + " from session " + request.sid)
    print("Active Sessions:")
    pprint(sessions)
