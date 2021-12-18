from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField, BooleanField, SelectField, FloatField
from wtforms.validators import DataRequired, Length, Email, EqualTo, ValidationError
import flask_website.dbAPI.app as db

#Registration form
class RegistrationForm(FlaskForm):

    email = StringField('Email',
                        validators=[DataRequired(), Email()])
    name = StringField('Full Name',
                        validators=[DataRequired()])
    password = PasswordField('Password', validators=[DataRequired()])
    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), EqualTo('password')])
    submit = SubmitField('Sign Up')

#Login form
class LoginForm(FlaskForm):
    email = StringField('Email',
                        validators=[DataRequired(), Email()])
    password = PasswordField('Password', validators=[DataRequired()])
    remember = BooleanField('Remember Me')
    submit = SubmitField('Login')

class AccountForm(FlaskForm):
    newEmail = StringField('Update Email Info')
    password = PasswordField('New Password')
    confirmPassword = PasswordField('Confirm New Password')
    phoneNumber = StringField('New Phone Number')
    submitAccountInfo = SubmitField('Submit Account Info')

class SensorAccountForm(FlaskForm):
    sensorID = StringField('Sensor ID')
    submitSensor = SubmitField('Submit Sensor')


class SettingsForm(FlaskForm):
    sensorID = SelectField('Select to add trigger to Sensor', choices = [('', 'Choose your Sensor')])
    textOrEmail = SelectField('Text or Email', choices = [(0, 'text'), (1, 'email')])
    level = StringField('Level')
    allSensorNames = SelectField('TODO: change in routes', choices = [('yes', 'no'), ('no', 'yes')])
    newSensorName = StringField('Enter Sensor\'s Name')
    sensorGroup = SelectField('ll', choices = [('', 'Choose Sensor Group')])
    sensorGroup_2 = SelectField('ll', choices = [('', 'Choose Sensor Group')])
    newSensorGroup = StringField('Enter New Sensor Group')
    submit = SubmitField('Add Trigger')
    alerts = SelectField('', choices =[('', 'Choose an alert to remove')])
    removeAlert = SubmitField('Remove Alert')
    changeName = SubmitField('Change Name')
    changeGroup = SubmitField('Change Group')
    modifyArea = SubmitField('Modify Area')
    newBottomLat = FloatField('BottomLat')
    newBottomLong = FloatField('BottomLong')
    newTopLat = FloatField('TopLat')
    newTopLong = FloatField('TopLong')
    add_sensor= SubmitField('Add Sensor')
    sensor_add_value = StringField(label='serial')
    sensor_add_latitude = FloatField(label='add_latitude')
    sensor_add_longitude = FloatField(label='add_longitude')
    sensor_elevation = FloatField(label='elevation')


class RequestResetForm(FlaskForm):
    email = StringField('Username',
                        validators=[DataRequired(), Email()])
    submit = SubmitField('Request Password Reset')

    def validate_email(self,email):

        user_email = db.accounts.get_id_by_email(email.data)
        print(user_email)
        if not user_email:
            raise ValidationError('That Email doesnt Exist. You must Regsiter First')

class ResetPasswordForm(FlaskForm):

    password = PasswordField('Password', validators=[DataRequired()])
    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), EqualTo('password')])
    submit = SubmitField('Reset Password')
